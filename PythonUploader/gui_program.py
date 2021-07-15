# /usr/bin/python3
import tkinter as tk
from tkinter import ttk
from tkinter import filedialog
from PIL import Image, ImageTk
import serial
import serial.tools.list_ports
import queue
import threading
import logging
import time
import typing
import epaper

NUMB_OF_FRAMES = 2

frame_options = ["Frame #%s" % x for x in range(1, NUMB_OF_FRAMES+1)]


class FlatUiColors:
    turqLt = '#1abc9c'
    turqDk = '#16a085'
    greenLt = '#2ecc71'
    greenDk = '#27ae60'
    blueLt = '#3498db'
    blueDk = '#2980b9'
    purpleLt = '#9b59b6'
    purpleDk = '#8e44ad'
    yellowLt = '#f1c40f'
    yellowDk = '#f39c12'
    orangeLt = '#e67e22'
    orangeDk = '#d35400'
    redLt = '#e74c3c'
    redDk = '#c0392b'
    whiteLt = '#ecf0f1'
    whiteDk = '#bdc3c7'
    greyLt = '#95a5a6'
    greyDk = '#7f8c8d'
    blackLt = '#34495e'
    blackDk = '#2c3e50'


class Ui:
    class UiElements:
        # General Stuff
        title: tk.Label = None
        # Frames
        img_frame: tk.Frame = None
        serial_frame: tk.Frame = None
        # Serial Frame Crap
        ser_list: ttk.Combobox = None
        ser_list_var: tk.StringVar = None
        ser_connect: tk.Button = None
        ser_getconfig: tk.Button = None
        ser_upload: tk.Button = None
        # Image Frame Crap
        import_pic: tk.Button = None
        pic_canvas: tk.Canvas = None
        pic_cav_id: int = None
        frames_listbox: tk.Listbox = None

    class ImageElements:
        photo_im: tk.PhotoImage = None
        im: Image = None
        orig_im: Image = None

    class GuiObjectConf:
        label = {'bg': FlatUiColors.blueDk, 'fg': FlatUiColors.whiteLt, 'font': 'Verdana 14'}
        title = {'bg': FlatUiColors.blueDk, 'fg': FlatUiColors.whiteLt, 'font': 'Verdana 24 bold'}
        button = {'bg': FlatUiColors.blueDk, 'font': 'Verdana 16 bold'}
        listbox = {'bg': FlatUiColors.whiteLt, 'font': 'Verdana 16'}
        combobox = {'font': 'Verdana 16', 'justify': 'center', 'state': 'readonly'}

    def __init__(self, out_q: queue.Queue, in_q: queue.Queue):
        self.log = logging.getLogger('ui')
        self.out_q = out_q
        self.in_q = in_q
        self.root = tk.Tk()
        self.root.tk_setPalette(background=FlatUiColors.blueDk)
        self.o = self.UiElements()
        self.i = self.ImageElements()
        self.comport_list: list = []
        self.exit = False
        # Setup the UI
        self.setup_ui()

    def setup_ui(self):
        # Init overall ui
        self.o.title = tk.Label(self.root, text='E-Paper Label Uploader Project', **self.GuiObjectConf.title)
        self.o.img_frame = tk.Frame(self.root)
        self.o.serial_frame = tk.Frame(self.root)
        # Init serial frame stuff
        self.o.ser_list_var = tk.StringVar(self.o.serial_frame)
        self.o.ser_list = ttk.Combobox(self.o.serial_frame, textvariable=self.o.ser_list_var, values=self.comport_list, **self.GuiObjectConf.combobox)
        self.o.ser_upload = tk.Button(self.o.serial_frame, text='Upload Frame', state='disabled', **self.GuiObjectConf.button)
        self.o.ser_connect = tk.Button(self.o.serial_frame, text='Connect to Device', **self.GuiObjectConf.button)
        self.o.ser_getconfig = tk.Button(self.o.serial_frame, text='Get Frames from Device', state='disabled', **self.GuiObjectConf.button)
        # Init picture frame stuff
        self.o.import_pic = tk.Button(self.o.img_frame, text='Import Image', **self.GuiObjectConf.button)
        self.o.pic_canvas = tk.Canvas(self.o.img_frame)
        self.o.frames_listbox = tk.Listbox(self.o.img_frame, **self.GuiObjectConf.listbox)

        # Pack overall UI stuff
        self.o.title.pack()
        self.o.serial_frame.pack()
        self.o.img_frame.pack(expand=True, fill='x')
        # Pack serial frame stuff
        self.o.ser_list.grid(row=1, column=1, sticky='nsew')
        self.o.ser_connect.grid(row=2, column=1, sticky='nsew')
        self.o.ser_getconfig.grid(row=1, column=2, sticky='nsew')
        self.o.ser_upload.grid(row=2, column=2, sticky='nsew')
        # Pack picture frame stuff
        self.o.import_pic.grid(row=1, column=1, columnspan=2)
        self.o.pic_canvas.grid(row=2, column=2, sticky='we')
        self.o.frames_listbox.grid(row=2, column=1)
        self.o.img_frame.grid_columnconfigure(2, weight=1)

        #TODO: Update the root to create an empty image, then resize the canvas to an appropriate size and update the GUI again
        self.root.update()
        self.init_empty_pic()
        self.on_resize_canvas()
        self.root.update()

        # Add GUI bindings and protocol callbacks
        self.root.protocol("WM_DELETE_WINDOW", self.on_closing)
        self.o.pic_canvas.bind("<Configure>", self.on_resize_canvas)
        self.o.import_pic.configure(command=self.press_import)
        self.o.ser_connect.configure(command=self.press_connect)
        self.o.ser_upload.configure(command=self.upload)

    def on_resize_canvas(self, envt=None):
        w = self.o.pic_canvas.winfo_width()
        self.i.im = self.i.orig_im.resize(size=(w, int((104/212)*w)), resample=Image.NEAREST)
        self.i.photo_im = ImageTk.PhotoImage(self.i.im)
        self.o.pic_canvas.itemconfigure(self.o.pic_cav_id, image=self.i.photo_im)
        self.o.pic_canvas.configure(width=self.i.photo_im.width(), height=self.i.photo_im.height())

    def init_empty_pic(self):
        w = self.o.pic_canvas.winfo_width()
        self.i.orig_im = Image.new(mode='1', size=(w, int((104/212)*w)), color=255)
        self.i.im = self.i.orig_im.copy()
        self.i.photo_im = ImageTk.PhotoImage(self.i.im)
        self.o.pic_cav_id = self.o.pic_canvas.create_image(0, 0, anchor='nw', image=self.i.photo_im)

    def queue_checker(self):
        while not self.in_q.empty():
            d = self.in_q.get()
            if not isinstance(d, list):
                continue
            if d[0] == 'ret_get_com_list':
                self.comport_list = d[1]
                self.o.ser_list.configure(values=self.comport_list)
            elif d[0] == 'update_connection_status':    # [message, is_connected]
                self.update_connection_status(d[1])
            elif d[0] == 'update_frame_display':        # [message, frame_number, image_data]
                self.update_frame(d[1], d[2])
            self.in_q.task_done()
        if not self.exit:
            self.root.after(50, self.queue_checker)

    def start(self):
        self.root.after(50, self.queue_checker)
        self.out_q.put(['get_com_list'])
        self.root.mainloop()

    def update_connection_status(self, is_connected: bool):
        if is_connected:
            self.o.ser_upload.configure(state='normal')
            self.o.ser_getconfig.configure(state='normal')
        else:
            self.o.ser_upload.configure(state='disabled')
            self.o.ser_getconfig.configure(state='disabled')

    def update_frame(self, frame_number: int, image_data: Image):
        if frame_number > NUMB_OF_FRAMES:
            return
        w = self.o.pic_canvas.winfo_width()
        self.i.orig_im = image_data.copy()
        self.i.im = self.i.orig_im.resize(size=(w, int((104 / 212) * w)), resample=Image.NEAREST)
        self.i.photo_im = ImageTk.PhotoImage(self.i.im)
        self.o.pic_canvas.itemconfigure(self.o.pic_cav_id, image=self.i.photo_im)

    def press_import(self):
        file_path = tk.filedialog.askopenfilename(filetypes=[('PNG Files', '*.png'), ('JPG Files', 'jpg'), ('JPEG Files', '*.jpeg')])
        if file_path is None or file_path == '':
            return
        w = self.o.pic_canvas.winfo_width()
        self.i.orig_im = Image.open(file_path).convert(mode='L')
        self.i.im = self.i.orig_im.resize(size=(w, int((104/212)*w)), resample=Image.NEAREST)
        self.i.photo_im = ImageTk.PhotoImage(self.i.im)
        self.o.pic_canvas.itemconfigure(self.o.pic_cav_id, image=self.i.photo_im)

    def press_connect(self):
        com_port = self.o.ser_list_var.get()
        if com_port is None or com_port == '':
            return
        self.log.debug('Attempting to connect to device %s' % com_port)
        self.out_q.put(['connect_ser', com_port])

    def upload(self):
        send_image = self.i.orig_im.convert(mode='L')
        if send_image.width != 212 or send_image.height != 104:
            send_image = send_image.resize(size=(212, 104), resample=Image.NEAREST)
        self.out_q.put(['upload_frame', send_image, 0])

    def on_closing(self):
        self.exit = True
        self.root.destroy()
        self.out_q.put(['program_stop'])


class Program:
    def __init__(self):
        self.log = logging.getLogger('program')
        self.main_to_gui_q = queue.Queue()
        self.gui_to_main_q = queue.Queue(maxsize=1)
        self.exit_t = False
        self.main_t = threading.Thread(target=self.main_thread)
        self.ui = Ui(self.gui_to_main_q, self.main_to_gui_q)
        self.dev: epaper.EpaperDevice = None

    def main_thread(self):
        while not self.exit_t:
            while not self.gui_to_main_q.empty():
                d = self.gui_to_main_q.get()
                if not isinstance(d, list):
                    continue
                if d[0] == 'connect_ser':
                    self.connect_to_device(d[1])
                elif d[0] == 'get_com_list':
                    comport_list = self.get_all_available_comports()
                    self.main_to_gui_q.put(['ret_get_com_list', comport_list])
                elif d[0] == 'program_stop':
                    self.exit_t = True
                elif d[0] == 'upload_frame':
                    self.upload_frame(d[1], d[2])
                self.gui_to_main_q.task_done()
            time.sleep(0.1)

    def begin(self):
        self.main_t.start()
        self.ui.start()

    def connect_to_device(self, dev_name):
        self.dev = epaper.EpaperDevice(dev_name)
        if not self.dev.connect():
            self.log.info('Unable to connect to device %s. Deleting object and returning' % dev_name)
            del self.dev
            return
        # self.log.info("Device ID: %s" % self.dev.get_device_version())
        self.main_to_gui_q.put(['update_connection_status', True])
        # TODO: Temporary test
        self.get_frame_from_device(0)

    def get_all_available_comports(self):
        # TODO: Change so that it checks for something to know if it's the epaper
        all_ports = epaper.get_available_devices()
        port_names = [x.device for x in all_ports]
        self.log.debug('Found the following COM ports: %s' % port_names)
        return port_names

    def get_frame_from_device(self, frame_number):
        if frame_number > NUMB_OF_FRAMES:
            return
        read_data = self.dev.get_frame(frame_number)
        print('Got', read_data)
        if len(read_data) == 0:
            return
        image = Image.new(mode='1', size=(212, 104))
        for x in range(212):  # Scan thru all vertical strips of pixels
            for y in range(104 // 8):  # Scan thru 8 vertical pixels at a time
                for k in range(8):
                    pixel_data = read_data[(x * (104 // 8)) + y] & (1 << (7 - k))
                    if pixel_data != 0:
                        pixel_data = 255
                    else:
                        pixel_data = 0
                    image.putpixel((x, 103 - ((y * 8) + k)), pixel_data)

        self.main_to_gui_q.put(['update_frame_display', frame_number, image])

    def upload_frame(self, image_data: Image, frame_number: int):
        if image_data.width != 212 or image_data.height != 104:
            self.log.warning('Image give to upload is %sx%s, not 212x104' % (image_data.width, image_data.height))
            return
        frame_arr = []
        # for i in range(self.dev.SECTION_SIZE * self.dev.NUMB_OF_SECTION_PER_FRAME):
        #     byte_append = 0
        #     for k in range(8):
        #         pixel_numb = i * k
        #         if pixel_numb < 212*104:
        #             to_append = image_data.getpixel((pixel_numb % 212, pixel_numb // 212))
        #             if to_append != 0:
        #                 byte_append |= (1 << k)
        #     frame_arr.append(byte_append)
        for i in range(((212 * 104) // 8)):
            frame_arr.append(0xFF)
        for x in range(212):  # Scan thru all vertical strips of pixels
            for y in range(104 // 8):  # Scan thru 8 vertical pixels at a time
                for k in range(8):  # Scan thru each pixel
                    pixe = image_data.getpixel((x, 103 - ((y * 8) + k)))  # Get the selected pixel's color data
                    if pixe <= 100:  # If pixel is bright
                        frame_arr[(x * (104 // 8)) + y] = frame_arr[(x * (104 // 8)) + y] & ((1 << (7 - k)) ^ 0xFF)  # Unset the k bit in byte
        while len(frame_arr) != self.dev.SECTION_SIZE * self.dev.NUMB_OF_SECTION_PER_FRAME:
            frame_arr.append(0)
        self.dev.write_frame(frame_number, frame_arr)
        self.dev.display_frame(frame_number)


def setup_logger():
    """Create the logging format, by setting the root logger"""

    lg = logging.getLogger()
    lg.setLevel(logging.DEBUG)

    formatter = logging.Formatter(fmt='%(asctime)s - %(name)s - %(funcName)s - %(levelname)s - %(message)s')

    stream_handler = logging.StreamHandler()
    stream_handler.setLevel(logging.DEBUG)
    stream_handler.setFormatter(formatter)

    lg.addHandler(stream_handler)


if __name__ == '__main__':
    setup_logger()
    m = Program()
    m.begin()
