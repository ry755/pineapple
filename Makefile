PI = 1

CSTDLIB = circle-stdlib
include $(CSTDLIB)/Config.mk

CIRCLEHOME = circle-stdlib/libs/circle
NEWLIBDIR = circle-stdlib/install/$(NEWLIB_ARCH)
LUADIR = lua-5.4.6/src

OBJS = src/main.o src/kernel.o src/lconsole.o src/lfilesystem.o

include $(CIRCLEHOME)/Rules.mk

CFLAGS += -I $(LUADIR) -I "$(NEWLIBDIR)/include" -I $(STDDEF_INCPATH) -I $(CSTDLIB)/include
LIBS := \
	"$(NEWLIBDIR)/lib/libm.a" "$(NEWLIBDIR)/lib/libc.a" "$(NEWLIBDIR)/lib/libcirclenewlib.a" \
	$(CIRCLEHOME)/addon/SDCard/libsdcard.a \
	$(CIRCLEHOME)/lib/usb/libusb.a \
	$(CIRCLEHOME)/lib/input/libinput.a \
	$(CIRCLEHOME)/addon/fatfs/libfatfs.a \
	$(CIRCLEHOME)/lib/fs/libfs.a \
	$(CIRCLEHOME)/lib/net/libnet.a \
	$(CIRCLEHOME)/lib/sched/libsched.a \
	$(CIRCLEHOME)/lib/libcircle.a \
	$(LUADIR)/liblua.a

-include $(DEPS)

libs:
	$(MAKE) -C $(CSTDLIB)
	$(MAKE) -C $(CIRCLEHOME)/addon/fatfs
	$(MAKE) -C $(CIRCLEHOME)/addon/SDCard
	$(MAKE) -C $(LUADIR) liblua.a

qemu: kernel.img
	# note that this requires a patched qemu build!
	# https://github.com/smuehlst/qemu
	rm -f disk.img
	qemu-img create disk.img 128M
	mkfs.vfat -F32 disk.img
	mcopy -i disk.img root/* ::
	qemu-system-arm -machine raspi1ap -serial stdio -kernel kernel.elf -device usb-kbd -sd disk.img -append keymap=US

clean-all: clean
	rm -rf src/*.d src/*.o
