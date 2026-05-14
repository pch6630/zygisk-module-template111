```python
SUPPORTED_ABIS = [
    'arm64-v8a'
]

ABI_NAME_ALIAS = {
    'arm64-v8a': ['arm64', 'a64', 'aarch64', 'arm64_v8a'],
}

ABI_CHOICES = list(ABI_NAME_ALIAS.keys()) + sum(ABI_NAME_ALIAS.values(), [])

ABI_MAP = {
    None: None
}

def initialize_abi_alias():
    for k in ABI_NAME_ALIAS:
        ABI_MAP[k] = k
        for v in ABI_NAME_ALIAS[k]:
            ABI_MAP[v] = k

initialize_abi_alias()

DEFAULT_ABI = "arm64-v8a"

ABI_TO_ARCH = {
    'arm64-v8a': 'aarch64',
}

ABI_TO_MAGISK_ARCH = {
    'arm64-v8a': 'arm64',
}

BUILD_TYPE_CHOICES = [
    "debug",
    "release"
]

BUILD_TYPE_CHOICES_MAP = {
    "debug": "Debug",
    "release": "Release"
}


def config(abi, plat, build_type):
    bin_build_type = BUILD_TYPE_CHOICES_MAP[build_type]

    build_dir = BUILD_DIR / BUILD_DIR_NAME / abi
    lib_output_dir = LIB_OUTPUT_DIR / abi
    bin_output_dir = BIN_OUTPUT_DIR / abi
    unstripped_output_dir = UNSTRIPPED_OUTPUT_DIR / abi

    exec_cmd(
        [
            'cmake',

            f'-H{SOURCE_DIR}',
            f'-B{build_dir}',

            f'-DANDROID_ABI={abi}',
            f'-DANDROID_PLATFORM={plat}',
            f'-DANDROID_NDK={ANDROID_NDK_HOME}',

            '-DANDROID_STL=c++_static',
            '-DANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES=ON',
            '-DANDROID_ARM_NEON=TRUE',

            f'-DCMAKE_TOOLCHAIN_FILE={CMAKE_TOOLCHAIN_FILE}',

            f'-DCMAKE_RUNTIME_OUTPUT_DIRECTORY={bin_output_dir}',
            f'-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={lib_output_dir}',

            f'-DDEBUG_SYMBOLS_PATH={unstripped_output_dir}',

            f'-DCMAKE_BUILD_TYPE={bin_build_type}',
            f'-DMODULE_NAME={MODULE_ID}',

            '-DCMAKE_EXPORT_COMPILE_COMMANDS=ON',

            '-G',
            'Ninja'
        ]
    )


def build_zip(args):
    build_type = BUILD_TYPE

    for abi in SUPPORTED_ABIS:
        build_all(
            abi=abi,
            plat=PLATFORM,
            build_type=build_type,
            force=args.force
        )

    module_path = OUTPUT_DIR / "module" / BUILD_DIR_NAME
    module_template = ROOT_DIR / 'template'

    shutil.rmtree(module_path, ignore_errors=True)

    os.makedirs(module_path, exist_ok=True)

    def fix_crlf(p: Path):
        with open(p, 'r', encoding='utf-8') as f:
            text = f.read()
            text = text.replace('\r', '').encode('utf-8')

        with open(p, 'wb') as f:
            f.write(text)

    def expand_text_file(p: Path, expand=None):
        if not p.exists():
            return

        with open(p, 'r', encoding='utf-8') as f:
            text = f.read()

            if expand:
                for k in expand:
                    v = expand[k]

                    text = text.replace(f'@{k}@', v)
                    text = text.replace(f'${{{k}}}', v)

        with open(p, 'wb') as f:
            f.write(text.encode('utf-8'))

    shutil.copytree(module_template, module_path, dirs_exist_ok=True)

    shutil.copy(ROOT_DIR / 'README.md', module_path / 'README.md')

    for p, _, fns in module_path.walk():
        for fn in fns:
            if fn == 'mazoku':
                continue

            fix_crlf(p / fn)

    expand_text_file(module_path / 'module.prop', {
        'moduleId': MODULE_ID,
        'moduleName': MODULE_NAME,
        'versionName': f"{RELEASE_NAME} ({GIT_COMMIT_COUNT}-{GIT_COMMIT_HASH}-{build_type})",
        'versionCode': str(GIT_COMMIT_COUNT)
    })

    script_vars = {
        'DEBUG': str(build_type == 'debug'),
        'SONAME': MODULE_ID,
        'SUPPORTED_ABIS': ' '.join(
            map(lambda x: ABI_TO_MAGISK_ARCH[x], SUPPORTED_ABIS)
        )
    }

    expand_text_file(module_path / 'customize.sh', script_vars)
    expand_text_file(module_path / 'post-fs-data.sh', script_vars)
    expand_text_file(module_path / 'service.sh', script_vars)
    expand_text_file(module_path / 'uninstall.sh', script_vars)
    expand_text_file(module_path / 'cleanup.sh', script_vars)

    with open(module_path / 'sepolicy.rule', 'r', encoding='utf-8') as f:
        content = f.read()

    with open(module_path / 'sepolicy.rule', 'w', encoding='utf-8') as f:
        f.write(
            '\n'.join(
                filter(
                    lambda x: not (
                        x.strip().startswith('#') or
                        x.strip() == ''
                    ),
                    content.split('\n')
                )
            ) + '\n'
        )

    shutil.copytree(
        NATIVE_OUTPUT_DIR,
        module_path,
        dirs_exist_ok=True
    )

    build_name = (
        f'{MODULE_NAME}-{RELEASE_NAME}-'
        f'{GIT_COMMIT_COUNT}-{GIT_COMMIT_HASH}-{build_type}'
    )

    zip_file_name = f"{build_name}.zip".replace(' ', '-')

    output_path = RELEASE_DIR / zip_file_name

    os.makedirs(output_path.parent, exist_ok=True)

    try:
        os.remove(output_path)
    except FileNotFoundError:
        pass

    with zipfile.ZipFile(
        output_path,
        'w',
        compression=zipfile.ZIP_DEFLATED
    ) as out_zip:

        for p, dns, fns in module_path.walk():

            for dn in dns:
                d = p / dn

                rp = str(
                    d.relative_to(module_path)
                ).replace('\\', '/')

                out_zip.mkdir(rp)

            for fn in fns:
                f = p / fn

                rp = str(
                    f.relative_to(module_path)
                ).replace('\\', '/')

                s = sha256()

                with open(f, 'rb') as fp:
                    with out_zip.open(rp, 'w') as wf:
                        while data := fp.read(4096):
                            s.update(data)
                            wf.write(data)

                s = s.digest()

                with out_zip.open(rp + '.sha256', 'w') as wf:
                    wf.write(s.hex().encode('utf-8'))

    print(f"* output {output_path}")

    return output_path
```
