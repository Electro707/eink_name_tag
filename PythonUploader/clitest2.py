import usb.core
import usb.util
import sys
import time

dev = usb.core.find(idVendor=0x0483, idProduct=0x5740)

if dev is None:
    print("No Device Found")
    sys.exit(-1)

if dev.is_kernel_driver_active(0):
    try:
        dev.detach_kernel_driver(0)
        print("kernel driver detached")
    except usb.core.USBError as e:
        sys.exit("Could not detach kernel driver: ")
else:
    print("no kernel driver attached")

dev.set_configuration()
cfg = dev.get_active_configuration()

print(cfg[(1, 0)])

out_ep = cfg[(1, 0)][0]
in_ep = cfg[(1, 0)][1]
#print(out_ep, in_ep)


for i in range(10):
    dev.write(0x01, 'v\n')
    ret = dev.read(0x81, 17, 1000)
    print(ret)
    time.sleep(0.3)

#if not dev.is_kernel_driver_active(0):
#    dev.attach_kernel_driver(0)
