#
# $Id$
#
# Shamelessly lifted and hacked from the
# free bios project.
#
# $Log$
# Revision 1.13  2003/02/09 18:41:08  warmcat
# Merged Ed's WD drive fixes
# Made a start on USB init
#
# Revision 1.12  2003/02/01 14:36:50  warmcat
# 800x600 video mode now default for PAL, thanks to Petteri Kangaslampi's work on nvtv.
# Set encoder bit for Luma LPF for 800x600, which voids Chroma artifacts on composite when the Luma if too HF.
# Confirmed working through to X (100x37 in fb text mode) with Oliver's driver.  Its a BIG improvement.
# Updated background JPEG to 1024x(768+64).
# Removed all fixed dependencies on 640 screen width.
# Updated Readme with sample XF86Config-4 files that worked for me with Oliver's driver.
# Cleaned up Vga init code.
# Don't think I broke anything for Xromwell this time.
#
# Revision 1.11  2003/01/21 22:05:45  warmcat
# v1.1 Xbox compatability; ability to boot Mandrake 9 boot CD as well as install CD, fix for 8.3 only CDs; Vsync-interrupt sprite :-) and other cosmetic stuff; temperature display; update backdrop JPG to 640x(576+64); improved CLI/STI code
#
# Revision 1.10  2003/01/14 15:24:18  warmcat
# New menu UI.  VSYNC IRQ up.  Franz's I2C check added.  Flashtype and Ed's EEPROM report added.
#
# Revision 1.3  2003/01/13 13:33:02  huceke
# Added a little function for reading the EEPROM content.
# Added support for setting the MAC Address at boottime.
# Updatet the Makefile and linkagescript for the xbe part.
#
# Revision 1.2  2003/01/13 12:33:52  warmcat
# Updated Makefile.rom
#
# Revision 1.9  2003/01/13 12:29:07  warmcat
# Left off BootInterrupts.c from last commit :-)
#
# Revision 1.5  2002/12/18 10:38:25  warmcat
# ISO9660 support added allowing CD boot; linuxboot.cfg support; some extra compiletime options and CD tray management stuff
#
# Revision 1.4  2002/12/15 21:40:52  warmcat
#
# First major release.  Working video; Native Reiserfs boot into Mdk9; boot instability?
#
# Revision 1.3  2002/09/19 10:39:19  warmcat
# Merged Michael's Bochs stuff with current cromwell
# note requires the cpp0-2.95 preprocessor
#
#
# 2002-09-14 andy@warmcat.com  changed to -Wall and -Werror, superclean
# 2002-09-11 andy@warmcat.com  branched for cromwell, removed all the linux boot stuff
#
# Revision 1.4  2002/08/19 13:53:33  meriac
# the need of nasm during linking removed
#
# Revision 1.2  2002/08/15 20:01:38  mist
# kernel and initrd needn't be patched into image.bin manually any more,
# due to Milosch
#
#

### compilers and options
CC	= gcc
#
# andy@warmcat.com 2003-01-11: problems with using -O2 tracked to BootPerformPicChallengeResponseAction ONLY
# this guy is handled specially with CFLAGSBR instead of CFLAGS, so almost everything is faster now
#
CFLAGS	= -g -O2 -Wall -Werror -DFSYS_REISERFS -Igrub -DSTAGE1_5 -DNO_DECOMPRESSION -Ijpeg-6b -DNO_GETENV -DCROMWELL
CFLAGSBR	= -g -Wall -Werror -DFSYS_REISERFS -Igrub -DSTAGE1_5 -DNO_DECOMPRESSION -Ijpeg-6b -DNO_GETENV -DCROMWELL
CFLAGSNV = -c -DLINUX -DEXPORT_SYMTAB -D__KERNEL__ -O \
        -Wstrict-prototypes -DCONFIG_PM  -fno-strict-aliasing \
        -mpreferred-stack-boundary=2 -march=i686 -falign-functions=4 \
        -DMODULE -I/lib/modules/$(uname -r)/build/include
LD	= ld
LDFLAGS	= -s -S -T ldscript.ld
OBJCOPY	= objcopy
# The BIOS make process requires the gcc 2.95 preprocessor, not 2.96 and ot 3.x
# if you have gcc 2.95, you can use the following line:
# GCC295 = ${GCC}
# if you don't, install at least the file cpp0 taken from a gcc 2.95 setup,
# and use a line like this one:
GCC295 = cpp0-2.95
# note that SuSE 8 cannot compile the BIOS, although the GCC version would
# be correct.
BCC = /usr/lib/bcc/bcc-cc1

### objects
OBJECTS	= BootStartup.o BootResetAction.o BootPerformPicChallengeResponseAction.o  \
BootPciPeripheralInitialization.o BootVgaInitialization.o BootIde.o \
BootHddKey.o rc4.o sha1.o BootVideoHelpers.o vsprintf.o filtror.o BootStartBios.o setup.o BootFilesystemIso9660.o \
BootLibrary.o BootInterrupts.o \
grub/fsys_reiserfs.o grub/char_io.o grub/disk_io.o \
jpeg-6b/jdapimin.o jpeg-6b/jdapistd.o jpeg-6b/jdtrans.o jpeg-6b/jdatasrc.o jpeg-6b/jdmaster.o \
jpeg-6b/jdinput.o jpeg-6b/jdmarker.o jpeg-6b/jdhuff.o jpeg-6b/jdphuff.o jpeg-6b/jdmainct.o jpeg-6b/jdcoefct.o \
jpeg-6b/jdpostct.o jpeg-6b/jddctmgr.o jpeg-6b/jidctfst.o jpeg-6b/jidctflt.o jpeg-6b/jidctint.o jpeg-6b/jidctred.o \
jpeg-6b/jdsample.o jpeg-6b/jdcolor.o jpeg-6b/jquant1.o jpeg-6b/jquant2.o jpeg-6b/jdmerge.o jpeg-6b/jmemnobs.o \
jpeg-6b/jmemmgr.o jpeg-6b/jcomapi.o jpeg-6b/jutils.o jpeg-6b/jerror.o \
BootFlash.o BootEEPROM.o\
BootUsbOhci.o BootParser.o BootFATX.o
# !!! killed temporarily to allow clean CVS checkin
#BootEthernet.o \
#nvn/nvnetlib.o



#RESOURCES = rombios.elf amended2bl.elf xcodes11.elf
#RESOURCES = rombios.elf xcodes11.elf
#RESOURCES = rombios.elf
RESOURCES = xcodes11.elf backdrop.elf

# target:
all	: image.bin

clean	:
#	mv nvn/nvnetlib.o nvn/nvnetlib.o.tmp
	rm -rf *.o *~ core *.core ${OBJECTS} ${RESOURCES} image.elf image.bin
#	mv nvn/nvnetlib.o.tmp nvn/nvnetlib.o
	rm -f  *.a _rombios_.c _rombios_.s rombios.s rombios.bin rombios.txt
	rm -f  backdrop.elf image_1024.bin

image.elf : ${OBJECTS} ${RESOURCES}
	${LD} -o $@ ${OBJECTS} ${RESOURCES} ${LDFLAGS}

rombios.elf : rombios.bin
	${LD} -r --oformat elf32-i386 -o $@ -T rombios.ld -b binary rombios.bin

amended2bl.elf : amended2bl.bin
	${LD} -r --oformat elf32-i386 -o $@ -T amended2bl.ld -b binary amended2bl.bin

xcodes11.elf : xcodes11.bin
	${LD} -r --oformat elf32-i386 -o $@ -T xcodes11.ld -b binary xcodes11.bin

backdrop.elf : backdrop.jpg
	${LD} -r --oformat elf32-i386 -o $@ -T backdrop.ld -b binary backdrop.jpg


### rules:
%.o	: %.c boot.h consts.h BootFilesystemIso9660.h config-rom.h config-xbe.h
	${CC} ${CFLAGS} -o $@ -c $<

%.o	: %.S consts.h config-rom.h config-xbe.h
	${CC} -DASSEMBLER ${CFLAGSBR} -o $@ -c $<

BootEthernet.o: BootEthernet.c boot.h consts.h config-rom.h config-xbe.h
	${CC} ${CFLAGSNV} -O2 -o $@ -c $<

BootPerformPicChallengeResponseAction.o: BootPerformPicChallengeResponseAction.c boot.h consts.h config-rom.h config-xbe.h
	${CC} ${CFLAGSBR} -o $@ -c $<

image.bin : image.elf
	${OBJCOPY} --output-target=binary --strip-all $< $@
	cat $@ $@ $@ $@ > image_1024.bin
	@ls -l $@

#     the following send the patched result to a Filtror and starts up the
#     Xbox with the new code
#     you can get lmilk from http://warmcat.com/milksop/milk.html
#	@lmilk -f -p image.bin -q
#    this one is the same but additionally runs terminal emulator
#	@lmilk -f -p image.bin -q -t

install:
	lmilk -f -p image.bin
	lmilk -f -a c0000 -p image.bin -q

1m:
	cat image.bin image.bin image.bin image.bin > image1m.bin

bios: rombios.bin

# note that the 16-bit Boch BIOS (rombios.*) is now deprecated

rombios.bin: rombios.c biosconfig.h
	${GCC295} -E $< > _rombios_.c
	${BCC} -o rombios.s -c -D__i86__ -0 _rombios_.c
	sed -e 's/^\.text//' -e 's/^\.data//' rombios.s > _rombios_.s
	as86 _rombios_.s -b rombios.bin -u- -w- -g -0 -j -O -l rombios.txt
	ls -l rombios.bin

