#!/usr/bin/env python3

import fx2.fx2tool
import sys
import time
from dataclasses import dataclass
import usb1
import os.path
import argparse

from fx2 import FX2Device, FX2DeviceError

ENVIRON_VAR_BACKUP = "BACKUP_LOCATION"
STOCK_BACKUP_FILENAME = "stock_piuio.ihex"

if ENVIRON_VAR_BACKUP in os.environ:
    STOCK_FILE_LOC = os.environ[ENVIRON_VAR_BACKUP]
else:
    print("No exported backup location, using {}".format(STOCK_BACKUP_FILENAME))
    STOCK_FILE_LOC = STOCK_BACKUP_FILENAME


@dataclass
class UsbMatching:
    vid: int
    pid: int
    prod_string: str
    friendly_name: str


# these we are most interested in.
bootloader = UsbMatching(0x04b4, 0x8613,
                         "FX2 series Cypress-class bootloader", "FX2 Bootloader")
stock_piuio_dev = UsbMatching(0x0547, 0x1002,
                              "EZ-USB FX2", "Stock PIUIO")
wiped_fx2_dev = UsbMatching(0x0547, 0x1002,
                              None, "Wiped FX2")

potentional_devices = [
    UsbMatching(0x04b4, 0x8613, None, "FX2 No EEPROM"),
    bootloader,
    wiped_fx2_dev,
    stock_piuio_dev,
    UsbMatching(0x0547, 0x1002,
                "fusion-piuio", "Fusion PIUIO"),
    UsbMatching(0x0547, 0x1337,
                "fusion-gamepad", "Fusion HID Gamepad"),
    UsbMatching(0x0547, 0x4b1d,
                "fusion-keyboard", "Fusion Keyboard"),
    UsbMatching(0x0d2f, 0x1020,
                "fusion-lxio", "Fusion LXIO"),
]


def call_tool(cmd):
    cmd = "main " + cmd
    sys.argv = cmd.split()

    # print(cmd)
    return fx2.fx2tool.main()


def backup_eeprom_to_file(filename):
    ensure_bootloader()
    print("Backing up eeprom to {}...".format(filename))
    call_tool("dump -f {}".format(filename))


def flash_file(vid, pid, filename):
    print("Flashing {}...".format(filename))
    call_tool("program -V {} -P {} -f {}".format(hex(vid), hex(pid), filename))


def ensure_bootloader():
    print("Pushing to bootloader...")
    for i in potentional_devices:
        try:
            call_tool("-d {}:{} -B reenumerate".format(hex(i.vid), hex(i.pid)))
        except:
            # we are allowed to have errors here as the device(s) might not exist.
            pass

    # let the usb system settle
    time.sleep(2)

    # ensure we have the bootloader.
    result = None
    while result is None:
        try:
            device = FX2Device()
            result = True
        except:
            print('.', end='')
            time.sleep(0.005)
            pass


def find_device():
    found_device = None

    for dev in potentional_devices:
        # print("Checking for {}".format(dev.friendly_name))

        with usb1.USBContext() as context:
            handle = context.openByVendorIDAndProductID(
                dev.vid,
                dev.pid,
                skip_on_error=True,
            )

            if handle:
                productString = handle.getProduct()

                if productString == dev.prod_string:
                    print("Found device: {}".format(dev.friendly_name))
                    found_device = dev
                    break

    return found_device


def main(file_to_flash):
    first_dev = find_device()

    if not first_dev:
        print("No compatible device found.")
        exit()

    # if there is a stock piuio without a backupfile, then take a backup file.
    if first_dev == stock_piuio_dev:
        if not os.path.exists(STOCK_FILE_LOC):
            print("No backup exists, taking...")
            backup_eeprom_to_file(STOCK_FILE_LOC)

    # check for a valid flashing file
    target_filename = sys.argv[1]

    ensure_bootloader()
    flash_file(stock_piuio_dev.vid, stock_piuio_dev.pid, file_to_flash)

    print("Flashing complete! Please turn off the device.")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='fusion flasher')
    parser.add_argument("file", type=str, help='file to flash to hardware')
    args = parser.parse_args()

    main(args.file)
