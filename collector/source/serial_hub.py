import serial, threading, queue, time

'''
Connect to Xbee via Serial and allow to read and write bytes
Creates a Y with the foot towards serial device
Data received from Xbee is duplictaed to each branch of the Y head
Data sent from any branch of Y head is sent to Xbee

Collector receives the serial data for ADC samples
Tcp listener for serial port for config with XCTU
'''
class SerialHub:

    def thread_handler_read(self):
        while self.serial_thread_run.is_set():
            b = self.serial_dev.read(1)
            if len(b) > 0:
                self.read_q_mutex.acquire()
                for q in self.read_q_list:
                    q.put(b)
                self.read_q_mutex.release()
    
    def thread_handler_write(self):
        while self.serial_thread_run.is_set():
            # blocks until it gets some
            b = self.write_q.get()
            if b == None:
                break
            # blocks until all is written
            self.serial_dev.write(b)
        

    def __init__(self, config):
        # list of read q for multiple clients
        self.read_q_mutex = threading.Lock()
        self.read_q_list = []
        # bytes to write to serial_dev
        self.write_q = queue.Queue()
        # serial
        port = config['xbee']['port']
        baud = config['xbee']['baudrate']
        self.serial_dev = serial.Serial(
            port = port,
            baudrate = baud,
            parity = serial.PARITY_NONE,
            stopbits = serial.STOPBITS_ONE,
            bytesize = serial.EIGHTBITS
        )
        # thread
        self.serial_thread_run = threading.Event()
        self.serial_thread_run.set()
        # thread read
        self.serial_thread_read = threading.Thread(target=self.thread_handler_read)
        self.serial_thread_read.start()
        # thread write
        self.serial_thread_write = threading.Thread(target=self.thread_handler_write)
        self.serial_thread_write.start()
    
    def stop(self):
        self.serial_thread_run.clear()
        # read
        self.serial_dev.cancel_read()
        self.serial_thread_read.join()
        # write
        self.write_q.put(None)
        self.serial_thread_write.join()
        self.serial_dev.close()
    
    '''@return (read queue, write queue)'''
    def request_r_w_queues(self):
        self.read_q_mutex.acquire()
        self.read_q_list.append(queue.Queue())
        r_q = self.read_q_list[-1]
        self.read_q_mutex.release()
        return (r_q, self.write_q)

    def release_r_w_queues(self, r_q, w_q):
        self.read_q_mutex.acquire()
        self.read_q_list.remove(r_q)
        self.read_q_mutex.release()



