# /usr/bin/python3
import serial
import serial.tools.list_ports
import logging
import typing
import time
import crcmod
# import zlib

epaper_log = logging.getLogger('epaper')

crc32_func = crcmod.mkCrcFun(0x104c11db7, rev=False, initCrc=0xFFFFFFFF, xorOut=0xFFFFFFFF)


class EpaperConnectionError(Exception):
    def __init__(self):
        super().__init__("Unable to connect to the Epaper display")


class EpaperUnconnectedError(Exception):
    def __init__(self):
        super().__init__("Not connected to device at all")


class EpaperDataError(Exception):
    def __init__(self, message=None):
        super().__init__("Data Error"+message)


class EpaperGenericError(Exception):
    def __init__(self, message=None):
        super().__init__(message)


def get_available_devices() -> list[serial.tools.list_ports_common.ListPortInfo]:
    """
        Function to find all available devices
        :return:
    """
    valid_ports = []
    all_ports = serial.tools.list_ports.comports()
    for port in all_ports:
        if port.pid == 0x5740:
            valid_ports.append(port)
            epaper_log.debug('Found device %s as valid' % port.device)
    return valid_ports


class EpaperDevice:
    COMMAND_GET_VERSION = b'v'
    COMMAND_GET_FRAME_WITH_SECTION = [0x03]
    COMMAND_WRITE_FRAME_WITH_SECTION = [0x02]
    COMMAND_DISPLAY_FRAME = [0x20]
    # COMMAND_GET_FRAME_WITH_SECTION = b'r'
    SECTION_SIZE = 64
    NUMB_OF_SECTION_PER_FRAME = 44

    def __init__(self, comport_name: str):
        self.log = epaper_log
        self.ser: serial.Serial = None
        self.com_name = comport_name

    def connect(self) -> bool:
        """
        Function to connect to the device
        TODO: Check device UUID/PID for correct device
        :return: False if not able to connected, True otherwise
        """
        self.log.info('Connecting to %s' % self.com_name)
        try:
            self.ser = serial.Serial(self.com_name, timeout=2, baudrate=115200)
        except serial.SerialException:
            self.log.warning("serial exception while connecting to %s" % self.com_name)
            raise EpaperConnectionError()
        self.log.info('Connected to device')
        return True

    def get_device_version(self) -> str:
        """
        Function to get the device version
        :return: The device version as a string
        """
        self.log.debug('Getting device version')
        self.send_command(self.COMMAND_GET_VERSION)
        ver = self.ser.read_until(expected=b'\n')
        # ver = ver.decode('utf-8').strip('\n')
        return ver

    def get_frame(self, frame_number: int) -> list:
        """
        Function to get a frame from the device
        :param frame_number: The frame number to read
        :return: A list that includes the frame
        """
        ret = []
        self.log.debug("Getting frame number %s" % frame_number)
        for s in range(self.NUMB_OF_SECTION_PER_FRAME):
            r = self.get_frame_section(frame_number, s)
            ret += r
        return ret

    def get_frame_section(self, frame_number: int, section: int) -> list:
        """
        Function to get a frame's section from the device
        :param frame_number: The frame number
        :param section: The section number in the frame
        :return: A list of the frame's section data read
        """
        self.log.debug("Getting frame %s section %s" % (frame_number, section))
        self.send_command(self.COMMAND_GET_FRAME_WITH_SECTION + [frame_number, section])
        ret = self.ser.read(self.SECTION_SIZE)
        if ret is None:
            raise EpaperDataError()
        if len(ret) != self.SECTION_SIZE:
            raise EpaperDataError('Got EEPROM packet size of %s' % len(ret))
        self.log.debug('Got %s from device' % list(ret))

        self.ser.flush()

        return list(ret)

    def write_frame_section(self, frame_number: int, section: int, data: list):
        if len(data) != self.SECTION_SIZE:
            raise EpaperGenericError()
        command = self.COMMAND_WRITE_FRAME_WITH_SECTION + [frame_number, section]
        self.send_command(command)
        # ret = self.ser.read_until(expected=b'\n')
        ret = self.ser.read(size=2)
        if len(ret) == 0:
            raise EpaperGenericError()
        if ret[0] != 0xFE:
            raise EpaperGenericError()
        self.log.debug('Get ACK from write_frame_section for frame %s section %s' % (frame_number, section))
        self.log.debug('Sending %s' % data)
        self.ser.write(bytes(data))
        # ret = self.ser.read_until(expected=b'\n')
        ret = self.ser.read(size=5)
        if len(ret) == 0:
            raise EpaperGenericError()
        self.log.debug('Got %s for CRC' % list(ret))
        # TODO: Add check if CRC isn't 4 bytes long
        ret = self.bytes_to_int32(ret[0:4])
        if crc32_func(bytes(data)) != ret:
            self.log.debug('Got %s back from write section number %s. Should be %s' % (ret, section, crc32_func((bytes(data)))))
            raise EpaperGenericError()

    def write_frame(self, frame_number: int, data: list):
        """

            :param frame_number:
            :param data:
            :return:
        """
        if len(data) != self.SECTION_SIZE*self.NUMB_OF_SECTION_PER_FRAME:
            raise EpaperGenericError('Wrong input size (expected %s, gave %s)' % (self.SECTION_SIZE*self.NUMB_OF_SECTION_PER_FRAME, len(data)))
        for s in range(self.NUMB_OF_SECTION_PER_FRAME-1):
            self.write_frame_section(frame_number, s, [d for d in data[(s*self.SECTION_SIZE):((s+1)*self.SECTION_SIZE)]])

    def send_command(self, command: typing.Union[bytes, list]):
        """
        Function to send a command to the device
        :param command: The command to send
        :return:
        """
        try:
            self.log.debug('Sent %s to device' % list(command))
            self.ser.write((bytes(command) + b'\n'))
        except AttributeError:
            if self.ser is None:
                raise EpaperUnconnectedError()

    def display_frame(self, frame_number: int):
        self.send_command(self.COMMAND_DISPLAY_FRAME + [frame_number])

    @staticmethod
    def bytes_to_int32(b: list):
        ret = 0
        for i in range(0, 3+1):
            ret += b[i] << (i*8)
        return ret
