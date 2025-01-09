# fusion
New firmware for the PIUIO to add new features to old hardware. 

Faster polling is just a firmware upgrade away.

# Features:
* Faster polling (1ms/1kHz in LXIO and gamepad modes)
* Full compatibility with existing arcade software
* Multiple targets: LXIO, original PIUIO, generic HID Gamepad
* Reactive lighting
* No additional hardware needed

# Flashing:
For deploying the firmware, check out the flasher program [here](https://github.com/dinsfire64/fusion-deployer). It is a self contained USB stick that you can boot on your computer/MK9, backup your stock original firmware, and flash the new firmware of your choice. After a reboot you are free to boot your target software/game. To flash the original back or change modes, simply boot the flash drive again, flash, and reboot.

<h1 style="text-align:center;"><a href="https://github.com/dinsfire64/fusion-deployer">CLICK HERE TO FLASH</a></h1>

# ...How?
The original PIUIO hardware was never the issue at hand...it was a combination of the firmware and the software. The original PIUIO firmware and the software that the games used to talk to it require that each individual sensor be polled and checked against. This requires that the game at the high level executes four writes and four reads to the device in question to know the full pad state. Over time different types of computer software was written in an attempt to speed up this process, but by writing our own firmware we can now individually poll each sensor asynchronously at a very low level.

This provides a speedup of up to 4x as the game no longer needs to tell the PIUIO to change sensors, it is constantly doing so on the device itself. It only takes one read to get the full pad state instead of four. This speedup comes at a compromise since this makes the test menu show all four sensors are being pressed when any one of them are being pressed. This is a minor tradeoff for a speedup of four times in a rhythm game.

For games that support the "LXIO"/ATXMEGA/PIU HID/NUVOTON PUMP IO systems, we can do even better. Since we can control the firmware of this device, we can also **pretend to be a different device entirely** and benefit from the newer software. The newer software is designed ask the "LXIO" device at a 1ms/1kHz interrupting intervals for the state of the input devices, which is faster than the software intended to talk to the PIUIO. Since we are pretending to be an LXIO, we can answer in the same fashion and even show the operator the state of all of the sensors in the test menu. This provides full compatibility with newer games for lights, input, and older hardware.

For further expansion of the concept, an HID Gamepad target is provided. This shows up in Windows/Linux/etc as a generic 32 button HID Gamepad, allowing mapping in any game that supports HID gamepads. It also supports HID lighting, with string endpoints for mapping of games that support it.

# Tested against:
* LXIO modes in Korean Pump: Prime 2, XX
* PIUIO modes in Korean Pump: NX2, Prime 2015, Prime 2, XX
* Western Pump mixes: Pro, Pro 2, Infinity
* In the Groove series: ITG (with r16 kernel update), OpenITG (kernel and libusb variants)
* Computer Software: piuio kernel module, piuio2vjoy, lxio2vjoy
* Hardware CPLD based Stage PCBs and newer revision Stage PCBs
* FX, GX (ITG), SXv2 cabinets
* Players like you, thank you!

# Compiling:
This software is compiled against [libfx2](https://github.com/whitequark/libfx2), which depends on [sdcc](https://sdcc.sourceforge.net/) as its compiler.

Please read the [documentation of libfx2](https://libfx2.readthedocs.io/) to install the flasher and its dependencies.

```
git clone --depth=1 $repo
cd $repo
git submodule update --init --recursive
make -j$(nproc) all
```

# Testing:
For testing purposes you can load the firmware into work ram of the fx2 using `make load` in the individual projects and test using the tools provided in the directory. This will not flash the firmware onto the eeprom of the device permanently. Do to do so use the flasher in the tools directory or the `flash` target in the makefile.

# Special Thanks
* [Racerxdl’s Schematic](https://github.com/racerxdl/piuio_clone/blob/master/schematics/original_schematic.png) for answering questions about the pinout of the original board.
* [Devin’s kernel module readme](https://github.com/djpohly/piuio?tab=readme-ov-file#implementation-and-accuracy) for polling accuracy of the device.
* [ghidra-cypress-fx2](https://github.com/gsuberland/ghidra-cypress-fx2) for ensuring I was matching the original code.
* [libfx2](https://github.com/whitequark/libfx2) for making an easy to use library with wonderful example code. I went from zero fx2 experience to working proof of concept firmware in about 48h with the documentation.
* Friends who helped rubber duck and test timing in game. Thank you!

# Please support the official release!

# Thank you for playing.
