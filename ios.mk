IOS_SDK=7.1

XCODE_PATH=/Applications/Xcode.app/Contents/Developer

IOS_SDK_ROOT=$(XCODE_PATH)/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS$(IOS_SDK).sdk
IOS_CC=$(shell xcrun -sdk iphoneos$(IOS_SDK) -find clang)

IOS_SIM_SDK_ROOT=$(XCODE_PATH)/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator$(IOS_SDK).sdk
IOS_SIM_CC=$(shell xcrun -sdk iphonesimulator$(IOS_SDK) -find clang)

IOS_LIPO=$(shell xcrun -sdk iphoneos$(IOS_SDK) -find lipo)
IOS_AR=$(shell xcrun -sdk iphoneos$(IOS_SDK) -find ar)

MYCFLAGS=-Wall -Werror -I. -Ibuild -Os

_ALL_A=libfakeredis-armv7.a libfakeredis-armv7s.a libfakeredis-arm64.a \
libfakeredis-i386.a libfakeredis-x86_64.a
ALL_A=$(addprefix build/,$(_ALL_A))

all: lib

lib: build/liblua.a build/libfakeredis.a

build/liblua.a:
	@$(MAKE) -f lua-ios.mk

build/libfakeredis.a: $(ALL_A)
	install -d $(shell dirname $@)
	$(IOS_LIPO) -output $@ -create $^
	cp fkredis.h $(shell dirname $@)

build/libfakeredis-armv7.a: build/fkredis-armv7.o build/sds-armv7.o
	$(IOS_AR) cq $@ $^

build/libfakeredis-armv7s.a: build/fkredis-armv7s.o build/sds-armv7s.o
	$(IOS_AR) cq $@ $^

build/libfakeredis-arm64.a: build/fkredis-arm64.o build/sds-arm64.o
	$(IOS_AR) cq $@ $^

build/libfakeredis-i386.a: build/fkredis-i386.o build/sds-i386.o
	$(IOS_AR) cq $@ $^

build/libfakeredis-x86_64.a: build/fkredis-x86_64.o build/sds-x86_64.o
	$(IOS_AR) cq $@ $^

build/%-armv7.o: CC=$(IOS_CC)
build/%-armv7.o: CFLAGS=-isysroot $(IOS_SDK_ROOT) -arch armv7 -mthumb \
	-miphoneos-version-min=4.0 $(MYCFLAGS)
build/%-armv7.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

build/%-armv7s.o: CC=$(IOS_CC)
build/%-armv7s.o: CFLAGS=-isysroot $(IOS_SDK_ROOT) -arch armv7s -mthumb \
	-miphoneos-version-min=4.0 $(MYCFLAGS)
build/%-armv7s.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

build/%-arm64.o: CC=$(IOS_CC)
build/%-arm64.o: CFLAGS=-isysroot $(IOS_SDK_ROOT) -arch arm64 \
	-miphoneos-version-min=7.0 $(MYCFLAGS)
build/%-arm64.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

build/%-i386.o: CC=$(IOS_CC)
build/%-i386.o: CFLAGS=-isysroot $(IOS_SIM_SDK_ROOT) -arch i386 \
	-miphoneos-version-min=4.0 $(MYCFLAGS)
build/%-i386.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

build/%-x86_64.o: CC=$(IOS_CC)
build/%-x86_64.o: CFLAGS=-isysroot $(IOS_SIM_SDK_ROOT) -arch x86_64 \
	-miphoneos-version-min=4.0 $(MYCFLAGS)
build/%-x86_64.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

fklua.h:
	@echo "converting lua -> C..."
	@./lua2c.sh

clean:
	rm -f build/*.o
	rm -f build/libfakeredis.a
	rm -f fklua.h

.PHONY: clean

build/fkredis-armv7.o: fkredis.h fklua.h
build/fkredis-armv7s.o: fkredis.h fklua.h
build/fkredis-arm64.o: fkredis.h fklua.h
build/fkredis-i386.o: fkredis.h fklua.h
build/fkredis-x86_64.o: fkredis.h fklua.h
build/sds-armv7.o: sds.h
build/sds-armv7s.o: sds.h
build/sds-arm64.o: sds.h
build/sds-i386.o: sds.h
build/sds-x86_64.o: sds.h
