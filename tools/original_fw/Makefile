FILENAME = piuio.bin

VID = 0547
PID = 1002

force-to-bootloader:
	python -m fx2.fx2tool -d $(VID):$(PID) -B reenumerate; \
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
	python -m fx2.fx2tool program -V $(VID) -D $(PID) -N -f $(FILENAME); python -m fx2.fx2tool reenumerate