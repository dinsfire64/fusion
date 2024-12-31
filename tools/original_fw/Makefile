FILENAME = piuio.bin

VID_PIUIO = 0547
PID_PIUIO = 1002

VID_LXIO       = 0D2F
PID_LXIO       = 1020

#use all possible vid/pid combos to get us into an eeprom writing state.
force-to-bootloader:
	python -m fx2.fx2tool -d $(VID_PIUIO):$(PID_PIUIO) -B reenumerate; \
	python -m fx2.fx2tool -d $(VID_LXIO):$(PID_LXIO) -B reenumerate; \
	python -m fx2.fx2tool -B reenumerate; \
	sleep 2;

#for backing up the existing firmware.
dump: force-to-bootloader
	python -m fx2.fx2tool dump -f $(FILENAME)

#for loading the firmware into ram with no eeprom attached/standard bootloader.
load: force-to-bootloader
	python -m fx2.fx2tool load $(FILENAME); python -m fx2.fx2tool reenumerate

#for overriding/flashing the eeprom on a blank device
flash: force-to-bootloader
	python -m fx2.fx2tool program -V $(VID_PIUIO) -D $(PID_PIUIO) -N -f $(FILENAME); python -m fx2.fx2tool reenumerate