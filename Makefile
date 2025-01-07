PIUIO_DIR = src/piuio
LXIO_DIR = src/lxio
GAMEPAD_DIR = src/gamepad

DATE := $(shell date '+%Y%m%d-%H%M%S')

#this order is the dependency order.
SUBDIRS = lib/libfx2/firmware/library src/piuio-lib $(PIUIO_DIR) $(LXIO_DIR) $(GAMEPAD_DIR)

all:
	@set -e; for dir in $(SUBDIRS); do $(MAKE) -C $${dir} all; done

clean:
	@set -e; for dir in $(SUBDIRS); do $(MAKE) -C $${dir} clean; done

#required for communicating with the fx2 bootloader.
install-flasher:
	cd lib/libfx2/software; python3 setup.py develop --user

#for loading the firmware into ram
load-piuio: all
	cd $(PIUIO_DIR); make load
	
load-lxio: all
	cd $(LXIO_DIR); make load
	
load-gamepad: all
	cd $(GAMEPAD_DIR); make load

#for commiting the firmware onto the eeprom
flash-piuio: all
	cd $(PIUIO_DIR); make flash
	
flash-lxio: all
	cd $(LXIO_DIR); make flash
	
flash-gamepad: all
	cd $(GAMEPAD_DIR); make flash

#get all of the firmware in one place.
release: all
	mkdir -p build; \
	cp $(PIUIO_DIR)/*.ihex build; \
	cp $(LXIO_DIR)/*.ihex build; \
	cp $(GAMEPAD_DIR)/*.ihex build; \
	cp tools/flasher/flash.py build/flash; \
	zip -r fusion-rel-$(DATE).zip build/*