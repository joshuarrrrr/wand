#!/usr/bin/env python3

import evdev
import sys

import wand

REQUIRED_MT_CODES = [
    evdev.ecodes.ABS_MT_SLOT,
    evdev.ecodes.ABS_MT_POSITION_X,
    evdev.ecodes.ABS_MT_POSITION_Y,
    evdev.ecodes.ABS_MT_TRACKING_ID,
]


def is_multitouch_device(device):
    capabilities = device.capabilities()
    if not evdev.ecodes.EV_ABS in capabilities:
        return False

    abs_codes = [cap[0] for cap in capabilities[evdev.ecodes.EV_ABS]]
    return all([code in abs_codes for code in REQUIRED_MT_CODES])


def find_touch_device():
    devices = [evdev.InputDevice(path) for path in evdev.list_devices()]
    return None if not devices else devices[0]


if __name__ == "__main__":
    touch_dev = find_touch_device()
    if not touch_dev:
        print("No suitable multi-touch device found!")
        sys.exit(0)

    print(f"Using touch device {touch_dev.name}")

    mt_device = wand.MultitouchDevice(path=touch_dev.path)
    mt_device.start()

    try:
        while mt_device.running:
            new, updated, finished = mt_device.poll_events()

            for touch in new:
                print(f"new touch point {touch.id}")

            for touch in updated:
                print(f"updated {touch.id} {touch.pos}")

            for touch in finished:
                print(f"touch point {touch.id} was finished")
    except KeyboardInterrupt:
        mt_device.stop()

    sys.exit(1)
