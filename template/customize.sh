# check disable=SC2034
SKIPUNZIP=1

DEBUG=@DEBUG@
SONAME=@SONAME@

# 지원 ABI
SUPPORTED_ABIS="arm64 arm64-v8a arm armeabi-v7a x64 x86_64 x86"

if [ "$BOOTMODE" ] && [ "$KSU" ]; then
    ui_print "- Installing from KernelSU app"

    if [ "$(which magisk)" ]; then
        ui_print "************************************************"
        ui_print "! Multiple root implementation is NOT supported!"
        ui_print "! Please uninstall Magisk before installing"
        ui_print "************************************************"
        abort
    fi

elif [ "$BOOTMODE" ] && [ "$MAGISK_VER_CODE" ]; then
    ui_print "- Installing from Magisk app"

else
    ui_print "************************************************"
    ui_print "! Install from recovery is not supported"
    ui_print "! Please install from KernelSU or Magisk app"
    ui_print "************************************************"
    abort
fi

VERSION=$(grep_prop version "${TMPDIR}/module.prop")
ui_print "- Installing $SONAME $VERSION"

########################################################
# ABI CHECK
########################################################

support=false

ui_print "- Device platform: $ARCH"

for abi in $SUPPORTED_ABIS
do
    case "$ARCH" in
        *"$abi"*)
            support=true
            ;;
    esac
done

if [ "$support" = "false" ]; then
    ui_print "! Unsupported platform: $ARCH"
    ui_print "! Trying force install..."
fi

########################################################
# EXTRACT verify.sh
########################################################

ui_print "- Extracting verify.sh"

unzip -o "$ZIPFILE" 'verify.sh' -d "$TMPDIR" >&2

if [ ! -f "$TMPDIR/verify.sh" ]; then
    ui_print "************************************************"
    ui_print "! Unable to extract verify.sh"
    ui_print "! Zip may be corrupted"
    ui_print "************************************************"
    abort
fi

. "$TMPDIR/verify.sh"

########################################################
# EXTRACT FILES
########################################################

extract "$ZIPFILE" 'customize.sh' "$TMPDIR/.unzip"
extract "$ZIPFILE" 'verify.sh' "$TMPDIR/.unzip"
extract "$ZIPFILE" 'sepolicy.rule' "$TMPDIR"

ui_print "- Extracting module files"

extract "$ZIPFILE" 'module.prop' "$MODPATH"
extract "$ZIPFILE" 'post-fs-data.sh' "$MODPATH"
extract "$ZIPFILE" 'service.sh' "$MODPATH"

mv "$TMPDIR/sepolicy.rule" "$MODPATH"

# 💡 버추얼 마스터 호환을 위해 구형 lib 구조 폴더를 생성합니다.
mkdir -p "$MODPATH/lib/armeabi-v7a"
mkdir -p "$MODPATH/lib/arm64-v8a"
mkdir -p "$MODPATH/lib/x86"
mkdir -p "$MODPATH/lib/x86_64"

########################################################
# EXTRACT LIBRARIES (버추얼 마스터 v25.2용 주입식 교정)
########################################################

# x86
if [ -f "$ZIPFILE/lib/x86/lib$SONAME.so" ]; then
    ui_print "- Extracting x86 libraries"
    extract "$ZIPFILE" "lib/x86/lib$SONAME.so" "$MODPATH/lib/x86" true
    mv "$MODPATH/lib/x86/lib$SONAME.so" "$MODPATH/lib/x86/libx86.so"
fi

# x86_64
if [ -f "$ZIPFILE/lib/x86_64/lib$SONAME.so" ]; then
    ui_print "- Extracting x64 libraries"
    extract "$ZIPFILE" "lib/x86_64/lib$SONAME.so" "$MODPATH/lib/x86_64" true
    mv "$MODPATH/lib/x86_64/lib$SONAME.so" "$MODPATH/lib/x86_64/libx86_64.so"
fi

# armeabi-v7a (32비트)
if [ -f "$ZIPFILE/lib/armeabi-v7a/lib$SONAME.so" ]; then
    ui_print "- Extracting arm libraries"
    extract "$ZIPFILE" "lib/armeabi-v7a/lib$SONAME.so" "$MODPATH/lib/armeabi-v7a" true
    mv "$MODPATH/lib/armeabi-v7a/lib$SONAME.so" "$MODPATH/lib/armeabi-v7a/libarmeabi-v7a.so"
fi

# arm64-v8a (64비트 게임 전용 핵심 주입)
if [ -f "$ZIPFILE/lib/arm64-v8a/lib$SONAME.so" ]; then
    ui_print "- Extracting arm64 libraries for Virtual Master"
    extract "$ZIPFILE" "lib/arm64-v8a/lib$SONAME.so" "$MODPATH/lib/arm64-v8a" true
    
    # 💡 최신 zygisk/ 형식을 버추얼 마스터가 읽을 수 있도록 원본 네이티브 이름으로 추출 유지합니다.
    mv "$MODPATH/lib/arm64-v8a/lib$SONAME.so" "$MODPATH/lib/arm64-v8a/libarm64-v8a.so"
fi

########################################################
# PERMISSION
########################################################

ui_print "- Setting permissions"

set_perm_recursive "$MODPATH" 0 0 0755 0644

# 생성된 핵심 바이너리에 실행 권한 부여
if [ -f "$MODPATH/lib/arm64-v8a/libarm64-v8a.so" ]; then
    set_perm "$MODPATH/lib/arm64-v8a/libarm64-v8a.so" 0 0 0755
fi

ui_print "*******************************"
ui_print " Installed successfully"
ui_print "*******************************"
