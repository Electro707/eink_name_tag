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

NUMB_OF_FRAMES = 3

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


class GuiObjectConf:
    label = {'bg': FlatUiColors.blueDk, 'fg': FlatUiColors.whiteLt, 'font': 'Verdana 14'}
    title = {'bg': FlatUiColors.blueDk, 'fg': FlatUiColors.whiteLt, 'font': 'Verdana 24 bold'}
    button = {'bg': FlatUiColors.blueDk, 'font': 'Verdana 16 bold'}
    listbox = {'bg': FlatUiColors.whiteLt, 'font': 'Verdana 16'}
    combobox = {'font': 'Verdana 16', 'justify': 'center', 'state': 'readonly'}


class GuiSerialFrame(tk.Frame):
    def __init__(self, *args):
        super().__init__(*args)

        self.ser_list_var = tk.StringVar(self)
        self.ser_list = ttk.Combobox(self, textvariable=self.ser_list_var, values=[], **GuiObjectConf.combobox)
        self.ser_upload = tk.Button(self, text='Upload Frame', state='disabled', **GuiObjectConf.button)
        self.ser_connect = tk.Button(self, text='Connect to Device', **GuiObjectConf.button)
        self.ser_getconfig = tk.Button(self, text='Get Frames from Device', state='disabled', **GuiObjectConf.button)

        self.ser_list.grid(row=1, column=1, sticky='nsew')
        self.ser_connect.grid(row=2, column=1, sticky='nsew')
        self.ser_getconfig.grid(row=1, column=2, sticky='nsew')
        self.ser_upload.grid(row=2, column=2, sticky='nsew')

    def update_connection_status(self, is_connected: bool):
        if is_connected:
            self.ser_upload.configure(state='normal')
            self.ser_getconfig.configure(state='normal')
        else:
            self.ser_upload.configure(state='disabled')
            self.ser_getconfig.configure(state='disabled')


class GuiImageFrame(tk.Frame):
    def __init__(self, *args):
        super().__init__(*args)

        self.photo_im: tk.PhotoImage = None
        self.pic_cav_id: int = None
        self.im: Image = None
        self.orig_im: Image = None

        self.import_pic = tk.Button(self, text='Import Image', **GuiObjectConf.button)
        self.pic_canvas = tk.Canvas(self)
        self.frames_combo_var = tk.IntVar(self)
        self.frames_combo = ttk.Combobox(self, textvariable=self.frames_combo_var, values=[0, 1, 2], **GuiObjectConf.combobox)

        self.import_pic.grid(row=1, column=1)
        self.frames_combo.grid(row=1, column=2)
        self.pic_canvas.grid(row=2, column=1, sticky='nswe', columnspan=2)

        self.columnconfigure([0, 3], weight=1)
        self.rowconfigure(2, weight=1)

        self.import_pic.configure(command=self.press_import)

    def on_resize_canvas(self, envt=None):
        w = self.pic_canvas.winfo_width()
        self.im = self.orig_im.resize(size=(w, int((104/212)*w)), resample=Image.NEAREST)
        self.photo_im = ImageTk.PhotoImage(self.im)
        self.pic_canvas.itemconfigure(self.pic_cav_id, image=self.photo_im)
        # self.pic_canvas.configure(width=self.photo_im.width(), height=self.photo_im.height())

    def init_empty_pic(self):
        w = self.pic_canvas.winfo_width()
        self.orig_im = Image.new(mode='1', size=(w, int((104/212)*w)), color=255)
        self.im = self.orig_im.copy()
        self.photo_im = ImageTk.PhotoImage(self.im)
        self.pic_cav_id = self.pic_canvas.create_image(0, 0, anchor='nw', image=self.photo_im)

    def update_current_frame(self, image_data: Image):
        w = self.pic_canvas.winfo_width()
        self.orig_im = image_data.copy()
        self.im = self.orig_im.resize(size=(w, int((104 / 212) * w)), resample=Image.NEAREST)
        self.photo_im = ImageTk.PhotoImage(self.im)
        self.pic_canvas.itemconfigure(self.pic_cav_id, image=self.photo_im)

    def press_import(self):
        file_path = tk.filedialog.askopenfilename(filetypes=[('PNG Files', '*.png'), ('JPG Files', 'jpg'), ('JPEG Files', '*.jpeg')])
        if file_path is None or file_path == '':
            return
        w = self.pic_canvas.winfo_width()
        self.orig_im = Image.open(file_path).convert(mode='L')
        self.im = self.orig_im.resize(size=(w, int((104/212)*w)), resample=Image.NEAREST)
        self.photo_im = ImageTk.PhotoImage(self.im)
        self.pic_canvas.itemconfigure(self.pic_cav_id, image=self.photo_im)


class Gui(tk.Tk):
    def __init__(self, out_q: queue.Queue, in_q: queue.Queue):
        super().__init__()

        self.log = logging.getLogger('ui')
        self.out_q = out_q
        self.in_q = in_q

        self.tk_setPalette(background=FlatUiColors.blueDk)
        self.exit = False

        self.title = tk.Label(self, text='E-Paper Label Uploader Project', **GuiObjectConf.title)
        self.img_frame = GuiImageFrame(self)
        self.serial_frame = GuiSerialFrame(self)

        self.title.pack()
        self.serial_frame.pack()
        self.img_frame.pack(expand=True, fill='x')

        #TODO: Update the root to create an empty image, then resize the canvas to an appropriate size and update the GUI again
        self.update()
        self.img_frame.init_empty_pic()
        self.img_frame.on_resize_canvas()
        self.update()

        # Add GUI bindings and protocol callbacks
        self.protocol("WM_DELETE_WINDOW", self.on_closing)
        self.serial_frame.ser_connect.configure(command=self.press_connect)
        self.serial_frame.ser_upload.configure(command=self.upload)
        self.img_frame.frames_combo_var.trace('w', self.change_frame)

    def queue_checker(self):
        while not self.in_q.empty():
            d = self.in_q.get()
            if not isinstance(d, list):
                continue
            if d[0] == 'ret_get_com_list':
                self.serial_frame.ser_list.configure(values=d[1])
            elif d[0] == 'update_connection_status':    # [message, is_connected]
                self.serial_frame.update_connection_status(d[1])
            elif d[0] == 'update_frame_display':        # [message, frame_number, image_data]
                self.img_frame.update_current_frame(d[2])
            elif d[0] == 'set_max_frames':
                self.img_frame.frames_combo.configure(values=[i for i in range(d[1])])
            self.in_q.task_done()
        if not self.exit:
            self.after(50, self.queue_checker)

    def change_frame(self, *args):
        cuu_frame = self.img_frame.frames_combo_var.get()
        self.out_q.put(['get_frame_image', cuu_frame])

    def start(self):
        self.img_frame.pic_canvas.bind("<Configure>", self.img_frame.on_resize_canvas)
        self.after(50, self.queue_checker)
        self.out_q.put(['get_com_list'])
        self.mainloop()

    def press_connect(self):
        com_port = self.serial_frame.ser_list_var.get()
        if com_port is None or com_port == '':
            return
        self.log.debug('Attempting to connect to device %s' % com_port)
        self.out_q.put(['connect_ser', com_port])

    def upload(self):
        send_image = self.img_frame.orig_im.convert(mode='L')
        if send_image.width != 212 or send_image.height != 104:
            send_image = send_image.resize(size=(212, 104), resample=Image.NEAREST)
        self.out_q.put(['upload_frame', send_image, self.img_frame.frames_combo_var.get()])

    def on_closing(self):
        self.exit = True
        self.destroy()
        self.out_q.put(['program_stop'])


class Program:
    def __init__(self):
        self.log = logging.getLogger('program')
        self.main_to_gui_q = queue.Queue()
        self.gui_to_main_q = queue.Queue(maxsize=1)
        self.exit_t = False
        self.main_t = threading.Thread(target=self.main_thread)
        self.ui = Gui(self.gui_to_main_q, self.main_to_gui_q)
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
                elif d[0] == 'get_frame_image':
                    self.get_frame_from_device(d[1])
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
        numb_frames = self.dev.get_device_maximum_frames()
        self.main_to_gui_q.put(['set_max_frames', numb_frames])
        # TODO: Temporary test
        self.get_frame_from_device(0)

    def get_all_available_comports(self):
        # TODO: Change so that it checks for something to know if it's the epaper
        all_ports = epaper.get_available_devices()
        port_names = [x.device for x in all_ports]
        self.log.debug('Found the following COM ports: %s' % port_names)
        return port_names

    def get_frame_from_device(self, frame_number):
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
