APP_OPTIM      := release
APP_STL        := c++_static
APP_CPPFLAGS   += -std=c++11 -Werror -fexceptions -frtti
#APP_CPPFLAGS   += -Wall
APP_ABI        := x86 x86_64 armeabi-v7a arm64-v8a

ifeq ($(TARGET_ARCH_ABI), x86)
LOCAL_CFLAGS += -m32
endif

ifeq ($(TARGET_ARCH_ABI), x86_64)
LOCAL_CFLAGS += -march=x86-64 -msse4.2 -mpopcnt -m64 -mtune=intel
endif
