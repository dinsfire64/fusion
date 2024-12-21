TARGET    = piuio
LIBRARIES = fx2 fx2isrs fx2usb
MODEL     = small

SOURCES = io main

VID       = 0547
PID       = 1002

LIBFX2  = lib/libfx2/firmware/library
include $(LIBFX2)/fx2rules.mk

FX2FLASH   = python3 -m fx2.fx2tool program -V $(VID) -P $(PID) -f 

make-libs:
	cd $(LIBFX2); make all
	
force-to-bootloader:
	python -m fx2.fx2tool -d $(VID):$(PID) -B reenumerate; \
	python -m fx2.fx2tool -B reenumerate; \
	sleep 2;
	
flash: all force-to-bootloader 
	$(FX2FLASH) $(TARGET).ihex; python -m fx2.fx2tool reenumerate