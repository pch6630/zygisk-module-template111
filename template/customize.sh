# shellcheck disable=SC2034
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

mkdir -p "$MODPATH/zygisk"

########################################################
# EXTRACT LIBRARIES
########################################################

# x86
if [ -f "$ZIPFILE/lib/x86/lib$SONAME.so" ]; then
    ui_print "- Extracting x86 libraries"

    extract "$ZIPFILE" "lib/x86/lib$SONAME.so" \
        "$MODPATH/zygisk" true

    mv "$MODPATH/zygisk/lib$SONAME.so" \
       "$MODPATH/zygisk/x86.so"
fi

# x86_64
if [ -f "$ZIPFILE/lib/x86_64/lib$SONAME.so" ]; then
    ui_print "- Extracting x64 libraries"

    extract "$ZIPFILE" "lib/x86_64/lib$SONAME.so" \
        "$MODPATH/zygisk" true

    mv "$MODPATH/zygisk/lib$SONAME.so" \
       "$MODPATH/zygisk/x86_64.so"
fi

# armeabi-v7a
if [ -f "$ZIPFILE/lib/armeabi-v7a/lib$SONAME.so" ]; then
    ui_print "- Extracting arm libraries"

    extract "$ZIPFILE" "lib/armeabi-v7a/lib$SONAME.so" \
        "$MODPATH/zygisk" true

    mv "$MODPATH/zygisk/lib$SONAME.so" \
       "$MODPATH/zygisk/armeabi-v7a.so"
fi

# arm64-v8a
if [ -f "$ZIPFILE/lib/arm64-v8a/lib$SONAME.so" ]; then
    ui_print "- Extracting arm64 libraries"

    extract "$ZIPFILE" "lib/arm64-v8a/lib$SONAME.so" \
        "$MODPATH/zygisk" true

    mv "$MODPATH/zygisk/lib$SONAME.so" \
       "$MODPATH/zygisk/arm64-v8a.so"
fi

########################################################
# PERMISSION
########################################################

ui_print "- Setting permissions"

set_perm_recursive "$MODPATH" 0 0 0755 0644

ui_print "*******************************"
ui_print " Installed successfully"
ui_print "*******************************"
