LUA_VER=5.2.3
IOS_SDK=7.0

XCODE_PATH=/Applications/Xcode.app/Contents/Developer

IOS_SDK_ROOT=$(XCODE_PATH)/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS$(IOS_SDK).sdk
IOS_CC=$(shell xcrun -sdk iphoneos$(IOS_SDK) -find clang)

IOS_SIM_SDK_ROOT=$(XCODE_PATH)/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator$(IOS_SDK).sdk
IOS_SIM_CC=$(shell xcrun -sdk iphonesimulator$(IOS_SDK) -find clang)

IOS_LIPO=$(shell xcrun -sdk iphoneos$(IOS_SDK) -find lipo)
IOS_AR=$(shell xcrun -sdk iphoneos$(IOS_SDK) -find ar)

_LUA_H=lua.h luaconf.h lualib.h lauxlib.h lua.hpp
LUA_H=$(addprefix lua-$(LUA_VER)/src/,$(_LUA_H))

# LUA_USE_MACOSX -> turn on LUA_USE_POSIX et al.
_CFLAGS=-Os -Wall -DLUA_COMPAT_ALL -DLUA_USE_MACOSX $(_MYCFLAGS)

all: lua-$(LUA_VER) build/liblua.a

# =================================================
# liblua -> fat static library for iOS
# =================================================
lua-$(LUA_VER):
	@echo "================================================= "
	@echo " DOWNLOADING Lua $(LUA_VER)"
	@echo "================================================= "
	@bash -c " \
		curl -O --progress-bar http://www.lua.org/ftp/lua-$(LUA_VER).tar.gz ;\
		tar xf lua-$(LUA_VER).tar.gz ;\
		rm -f lua-$(LUA_VER).tar.gz ;\
	"

build/liblua.a: build/liblua-armv7.a build/liblua-armv7s.a build/liblua-arm64.a\
 build/liblua-i386.a build/liblua-x86_64.a
	install -d $(shell dirname $@)
	rm -f $@
	$(IOS_LIPO) -output $@ -create $^
	cp $(LUA_H) $(shell dirname $@)

# =================================================
# liblua -> ARMv7
# =================================================
build/liblua-armv7.a: _MYCFLAGS=-isysroot $(IOS_SDK_ROOT) -arch armv7 -mthumb \
	-miphoneos-version-min=4.0
build/liblua-armv7.a:
	install -d $(shell dirname $@)
	$(MAKE) -C lua-$(LUA_VER)/src clean
	$(MAKE) -C lua-$(LUA_VER)/src a CC=$(IOS_CC) AR="$(IOS_AR) rcu" CFLAGS="$(_CFLAGS)"
	cp lua-$(LUA_VER)/src/liblua.a $@

# =================================================
# liblua -> ARMv7s
# =================================================
build/liblua-armv7s.a: _MYCFLAGS=-isysroot $(IOS_SDK_ROOT) -arch armv7s -mthumb \
	-miphoneos-version-min=4.0
build/liblua-armv7s.a:
	install -d $(shell dirname $@)
	$(MAKE) -C lua-$(LUA_VER)/src clean
	$(MAKE) -C lua-$(LUA_VER)/src a CC=$(IOS_CC) AR="$(IOS_AR) rcu" CFLAGS="$(_CFLAGS)"
	cp lua-$(LUA_VER)/src/liblua.a $@

# =================================================
# liblua -> ARM64
# =================================================
build/liblua-arm64.a: _MYCFLAGS=-isysroot $(IOS_SDK_ROOT) -arch arm64 \
	-miphoneos-version-min=7.0
build/liblua-arm64.a:
	install -d $(shell dirname $@)
	$(MAKE) -C lua-$(LUA_VER)/src clean
	$(MAKE) -C lua-$(LUA_VER)/src a CC=$(IOS_CC) AR="$(IOS_AR) rcu" CFLAGS="$(_CFLAGS)"
	cp lua-$(LUA_VER)/src/liblua.a $@

# =================================================
# liblua -> i386
# =================================================
build/liblua-i386.a: _MYCFLAGS=-isysroot $(IOS_SIM_SDK_ROOT) -arch i386 \
	-miphoneos-version-min=4.0
build/liblua-i386.a:
	install -d $(shell dirname $@)
	$(MAKE) -C lua-$(LUA_VER)/src clean
	$(MAKE) -C lua-$(LUA_VER)/src a CC=$(IOS_SIM_CC) AR="$(IOS_AR) rcu" CFLAGS="$(_CFLAGS)"
	cp lua-$(LUA_VER)/src/liblua.a $@

# =================================================
# liblua -> x86_64
# =================================================
build/liblua-x86_64.a: _MYCFLAGS=-isysroot $(IOS_SIM_SDK_ROOT) -arch x86_64 \
	-miphoneos-version-min=4.0
build/liblua-x86_64.a:
	install -d $(shell dirname $@)
	$(MAKE) -C lua-$(LUA_VER)/src clean
	$(MAKE) -C lua-$(LUA_VER)/src a CC=$(IOS_SIM_CC) AR="$(IOS_AR) rcu" CFLAGS="$(_CFLAGS)"
	cp lua-$(LUA_VER)/src/liblua.a $@

clean:
	rm -f build/liblua*.a
	rm -f $(addprefix build/,$(_LUA_H))

distclean:
	rm -rf build
	rm -rf lua-$(LUA_VER)

.PHONY: clean distclean
