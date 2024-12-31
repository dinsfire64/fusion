TARGET    = lxio
LIBRARIES = fx2 fx2isrs fx2usb
MODEL     = small

SOURCES = main lxio

VID       = 0547
PID       = 1002

#VID       = 0D2F
#PID       = 1020

LIBFX2  = ../../lib/libfx2/firmware/library
include $(LIBFX2)/fx2rules.mk

#include our io routines.
LIB_NAME = piuio-lib
SDCCFLAGS += -I../$(LIB_NAME)/ ../$(LIB_NAME)/build/io.rel

FX2FLASH   = python3 -m fx2.fx2tool program -V $(VID) -P $(PID) -f 
	
force-to-bootloader:
	python -m fx2.fx2tool -d $(VID):$(PID) -B reenumerate; \
	python -m fx2.fx2tool -B reenumerate; \
	sleep 2;
	
flash: all force-to-bootloader 
	$(FX2FLASH) $(TARGET).ihex; python -m fx2.fx2tool reenumerate