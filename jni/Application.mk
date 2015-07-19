APP_OPTIM := release
APP_STL := gnustl_static
APP_CPPFLAGS += -fexceptions -frtti
APP_ABI := armeabi armeabi-v7a x86 mips

ifeq ($(TARGET_ARCH_ABI), x86)
LOCAL_CFLAGS += -m32
endif

ifeq ($(TARGET_ARCH_ABI), x86_64)
LOCAL_CFLAGS += -march=x86-64 -msse4.2 -mpopcnt -m64 -mtune=intel
endif
