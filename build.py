import os
import subprocess as sp
import json
import sys
import shutil
import zipfile

from hashlib import sha256
from pathlib import Path
from argparse import ArgumentParser

MODULE_ID = "zygisk-test"
MODULE_NAME = "Zygisk Test"
VER_NAME = "1"

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


def exec_out(cmd):
    p = sp.Popen(cmd, stdout=sp.PIPE)

    content = p.stdout.read().decode('utf-8').strip()

    p.wait()

    return content


def exec_cmd(cmd, ignore_error=False, *args, **kwargs):
    p = sp.Popen(cmd, *args, **kwargs)

    v = p.wait()

    if not ignore_error and v != 0:
        raise RuntimeError(f'exec return non-zero: {v} {cmd}')


GIT_COMMIT_COUNT = int(
    exec_out('git rev-list HEAD --count'.split(' '))
)

GIT_COMMIT_HASH = exec_out(
    'git rev-parse --verify --short HEAD'.split(' ')
)


def initialize(args):
    global ANDROID_HOME
    global ANDROID_NDK_HOME
    global PLATFORM
    global CMAKE_TOOLCHAIN_FILE
    global ROOT_DIR
    global BUILD_DIR
    global OUTPUT_DIR
    global SOURCE_DIR
    global RELEASE_NAME
    global NATIVE_OUTPUT_DIR
    global BIN_OUTPUT_DIR
    global LIB_OUTPUT_DIR
    global BUILD_TYPE
    global UNSTRIPPED_OUTPUT_DIR
    global RELEASE_DIR
    global BUILD_DIR_NAME

    ANDROID_HOME = os.getenv('ANDROID_HOME')

    if ANDROID_HOME is None:
        ANDROID_HOME = os.getenv('ANDROID_SDK_ROOT')

    if ANDROID_HOME is not None:
        ANDROID_HOME = Path(ANDROID_HOME)

    with open('project-config.json', 'r', encoding='utf-8') as f:
        project_config = json.load(f)

        if args.ndk:
            ndk_ver = args.ndk
        else:
            ndk_ver = project_config['ndkVer']

        ANDROID_NDK_HOME = os.getenv('ANDROID_NDK_HOME')

        if ANDROID_NDK_HOME is None:
            if ANDROID_HOME is None:
                raise ValueError(
                    'ANDROID_HOME or ANDROID_NDK_HOME required!'
                )

            ANDROID_NDK_HOME = ANDROID_HOME / 'ndk' / ndk_ver
        else:
            ANDROID_NDK_HOME = Path(ANDROID_NDK_HOME)

        PLATFORM = project_config['platform']

    CMAKE_TOOLCHAIN_FILE = (
        ANDROID_NDK_HOME /
        'build/cmake/android.toolchain.cmake'
    )

    BUILD_TYPE = args.build_type

    ROOT_DIR = Path(__file__).parent.resolve()

    BUILD_DIR = (ROOT_DIR / "my_build").absolute()

    OUTPUT_DIR = (ROOT_DIR / "output").absolute()

    BUILD_DIR_NAME = BUILD_TYPE

    NATIVE_OUTPUT_DIR = (
        OUTPUT_DIR /
        "native" /
        BUILD_DIR_NAME
    )

    BIN_OUTPUT_DIR = NATIVE_OUTPUT_DIR / "bin"

    LIB_OUTPUT_DIR = NATIVE_OUTPUT_DIR / "lib"

    UNSTRIPPED_OUTPUT_DIR = (
        OUTPUT_DIR /
        "unstripped" /
        BUILD_DIR_NAME
    )

    RELEASE_DIR = ROOT_DIR / "release"

    SOURCE_DIR = ROOT_DIR / "native"

    RELEASE_NAME = VER_NAME


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


def build_all(abi, plat, build_type, force):
    lib_output_dir = LIB_OUTPUT_DIR / abi

    bin_output_dir = BIN_OUTPUT_DIR / abi

    if force:
        shutil.rmtree(lib_output_dir, ignore_errors=True)

        shutil.rmtree(bin_output_dir, ignore_errors=True)

        shutil.rmtree(
            UNSTRIPPED_OUTPUT_DIR / abi,
            ignore_errors=True
        )

    build_dir = BUILD_DIR / BUILD_DIR_NAME / abi

    config(
        abi,
        plat,
        build_type=build_type
    )

    exec_cmd(
        [
            'cmake',
            '--build',
            build_dir,
            '--',
            f'-j{os.cpu_count()}'
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

    shutil.copytree(
        module_template,
        module_path,
        dirs_exist_ok=True
    )

    shutil.copy(
        ROOT_DIR / 'README.md',
        module_path / 'README.md'
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


def zip_cmd(args):
    build_zip(args)


def main():
    ap = ArgumentParser()

    ap.add_argument(
        '--ndk',
        dest='ndk',
        required=False
    )

    ap.add_argument(
        '--force',
        dest='force',
        action='store_true'
    )

    ap.add_argument(
        '-t',
        dest='build_type',
        choices=BUILD_TYPE_CHOICES,
        default='release'
    )

    subps = ap.add_subparsers(required=True)

    zip_args = subps.add_parser('zip')

    zip_args.set_defaults(func=zip_cmd)

    args = ap.parse_args(sys.argv[1:])

    initialize(args)

    args.func(args)


main()
