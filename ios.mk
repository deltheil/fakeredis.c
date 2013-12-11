IOS_SDK=7.0

XCODE_PATH=/Applications/Xcode.app/Contents/Developer

IOS_SDK_ROOT=$(XCODE_PATH)/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS$(IOS_SDK).sdk
IOS_CC=$(shell xcrun -sdk iphoneos$(IOS_SDK) -find clang)

IOS_SIM_SDK_ROOT=$(XCODE_PATH)/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator$(IOS_SDK).sdk
IOS_SIM_CC=$(shell xcrun -sdk iphonesimulator$(IOS_SDK) -find clang)

IOS_LIPO=$(shell xcrun -sdk iphoneos$(IOS_SDK) -find lipo)
IOS_AR=$(shell xcrun -sdk iphoneos$(IOS_SDK) -find ar)

all: lib

lib: build/liblua.a build/libfakeredis.a

build/liblua.a:
	@$(MAKE) -f lua-ios.mk

build/libfakeredis.a: build/fkredis-armv7.o build/fkredis-armv7s.o \
	build/fkredis-arm64.o build/fkredis-i386.o build/fkredis-x86_64.o
	install -d $(shell dirname $@)
	$(IOS_LIPO) -output $@ -create $^
	cp fkredis.h $(shell dirname $@)

build/fkredis-armv7.o: CC=$(IOS_CC)
build/fkredis-armv7.o: CFLAGS=-isysroot $(IOS_SDK_ROOT) -arch armv7 -mthumb \
	-miphoneos-version-min=4.0 -I. -Ibuild
build/fkredis-armv7.o: fkredis.c fkredis.h fklua.h
	$(CC) -c $(CFLAGS) $< -o $@

build/fkredis-armv7s.o: CC=$(IOS_CC)
build/fkredis-armv7s.o: CFLAGS=-isysroot $(IOS_SDK_ROOT) -arch armv7s -mthumb \
	-miphoneos-version-min=4.0 -I. -Ibuild
build/fkredis-armv7s.o: fkredis.c fkredis.h fklua.h
	$(CC) -c $(CFLAGS) $< -o $@

build/fkredis-arm64.o: CC=$(IOS_CC)
build/fkredis-arm64.o: CFLAGS=-isysroot $(IOS_SDK_ROOT) -arch arm64 \
	-miphoneos-version-min=7.0 -I. -Ibuild
build/fkredis-arm64.o: fkredis.c fkredis.h fklua.h
	$(CC) -c $(CFLAGS) $< -o $@

build/fkredis-i386.o: CC=$(IOS_CC)
build/fkredis-i386.o: CFLAGS=-isysroot $(IOS_SIM_SDK_ROOT) -arch i386 \
	-miphoneos-version-min=4.0 -I. -Ibuild
build/fkredis-i386.o: fkredis.c fkredis.h fklua.h
	$(CC) -c $(CFLAGS) $< -o $@

build/fkredis-x86_64.o: CC=$(IOS_CC)
build/fkredis-x86_64.o: CFLAGS=-isysroot $(IOS_SIM_SDK_ROOT) -arch x86_64 \
	-miphoneos-version-min=4.0 -I. -Ibuild
build/fkredis-x86_64.o: fkredis.c fkredis.h fklua.h
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f build/*.o
	rm -f build/libfakeredis.a

.PHONY: clean
