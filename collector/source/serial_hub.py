import serial, threading, queue

'''
Connect to Xbee via Serial. Allows TCP client to read/Write to the same port as collector
Data received from XBEE is duplictaed to any connected clients
Data sent by collector and any other tcp client is sent to Xbee
|- colletor does not write
|- if more than 1 tcp client, they must sync up
'''
class SerialHub:

    def thread_handler(self, config):
        port = config['xbee']['port']
        baud = config['xbee']['baudrate']
        serial_dev = serial.Serial(
            port = port,
            baudrate = baud,
            parity = serial.PARITY_NONE,
            stopbits = serial.STOPBITS_ONE,
            bytesize = serial.EIGHTBITS,
            timeout = 0.1 # sec
        )
        while self.serial_thread_run.is_set():
            # read, for maximum 0.1s
            b = serial_dev.read(1024)
            if len(b) > 0:
                self.read_q.put(b)
            # write, everything that's in the q
            # but loop after 1024 bytes written to be nice
            w_cnt = 0
            while not self.write_q.empty() and w_cnt <= 1024:
                b = self.write_q.get()
                w_cnt += len(b)
                # blocks until all is written
                serial_dev.write(b)
        serial_dev.close()
                

    def __init__(self, config):
        self.read_q = queue.Queue()
        self.write_q = queue.Queue()
        #self.serial_thread_lock = threading.Lock()
        self.serial_thread_run = threading.Event()
        self.serial_thread_run.set()
        self.serial_thread = threading.Thread(target=self.thread_handler, args=(config,))
        self.serial_thread.start()
    
    def __del__(self):
        self.serial_thread_run.clear()
        self.serial_thread.join()

    def read(self):
        # return all read bytes
        l = list()
        # blocks until it gets some data
        b = self.read_q.get()
        l.append(b)
        while not self.read_q.empty():
            b = self.read_q.get()
            l.append(b)
        return b''.join(l)

    def write(self, data):
        self.write_q.put(data)

