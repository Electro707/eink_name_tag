"""
    A test file used while developing the firmware of the epaper display board.
    Probably nothing too useful here
"""

import epaper
import sys
import logging
import time
from PIL import Image


def open_image(invert=True) -> list:
    # Declare variables
    finalhex = []  # Declare the output array's variable
    # Open image and determine width and height
    im = Image.open('Jamal3.png')
    im = im.convert('L')
    w = im.width
    h = im.height
    print("Width of image is :", w)
    print("Height of image is :", h)
    # Check image size. If not correct size, exit program.
    if w != 212:
        print("The width isn't 212 pixels. Returning")
    if h != 104:
        print("The height isn't 104 pixels. Returning")
    # Append 0's to the buffer rray
    for i in range(((w * h) // 8)):
        if not invert:  # If output is inverted, set default to 0
            finalhex.append(0)
        else:  # If output is non-inverted, set default to 1
            finalhex.append(0xFF)
    # Create the output array.
    for x in range(w):  # Scan thru all vertical strips of pixels
        for y in range(h // 8):  # Scan thru 8 vertical pixels at a time
            for k in range(8):  # Scan thru each pixel
                pixe = im.getpixel((x, 103 - ((y * 8) + k)))  # Get the selected pixel's color data
                if (pixe <= 100):  # If pixel is bright
                    if not invert:
                        finalhex[(x * (h // 8)) + y] = finalhex[(x * (h // 8)) + y] | (
                                    1 << (7 - k))  # Set the k bit in byte
                    else:
                        finalhex[(x * (h // 8)) + y] = finalhex[(x * (h // 8)) + y] & (
                                    (1 << (7 - k)) ^ 0xFF)  # Unset the k bit in byte
    return finalhex

lg = logging.getLogger()
lg.setLevel(logging.DEBUG)
formatter = logging.Formatter(fmt='%(asctime)s - %(name)s - %(funcName)s - %(levelname)s - %(message)s')
stream_handler = logging.StreamHandler()
stream_handler.setLevel(logging.DEBUG)
stream_handler.setFormatter(formatter)
lg.addHandler(stream_handler)

time.sleep(1)

# Use first available device and connect to it
device_port = epaper.get_available_devices()
if device_port is None:
    print("No devices found :(")
    sys.exit(-1)
if len(device_port) == 0:
    print("No devices found :(")
    sys.exit(-1)
device_port = device_port[0]
# Try to connect to device
try:
    device = epaper.EpaperDevice(device_port.device)
    device.connect()
except epaper.EpaperConnectionError:
    print('Unable to connect to device')
    sys.exit(-1)
print('Connected to device')

print(device.get_device_version())

data = open_image()
while len(data) != device.SECTION_SIZE*device.NUMB_OF_SECTION_PER_FRAME:
    data.append(0)
#data = [x % 256 for x in range(device.SECTION_SIZE*device.NUMB_OF_SECTION_PER_FRAME)]
device.write_frame(0, data)
device.display_frame(0)

time.sleep(1)

# print(device.get_frame(0))
#while 1:
    #a = device.get_device_version()
    #print(len(a), a)
    #print(device.get_frame_section(0, 0))
    #time.sleep(0.5)

