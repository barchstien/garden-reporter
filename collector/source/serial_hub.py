import serial, threading, queue

'''
Connect to Xbee via Serial and allow to read and write bytes
Creates a Y with the foot towards serial device
Data received from Xbee is duplictaed to each branch of the Y head
Data sent from any branch of Y head is sent to Xbee

Collector receives the serial data for ADC samples
Tcp listener for serial port for config with XCTU
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
            b = serial_dev.read(4096)
            if len(b) > 0:
                #print("==== serial read bytes:", len(b))
                #print("==== serial self.read_q_list len:", len(self.read_q_list))
                self.read_q_mutex.acquire()
                for q in self.read_q_list:
                    #print("  == serial put bytes:", len(b))
                    q.put(bytes(b))
                self.read_q_mutex.release()
            # write, everything that's in the q
            # but loop after 1024 bytes written to be nice
            w_cnt = 0
            while not self.write_q.empty() and w_cnt <= 4096:
                b = self.write_q.get()
                w_cnt += len(b)
                # blocks until all is written
                serial_dev.write(b)
        serial_dev.close()
                

    def __init__(self, config):
        # list of read q for multiple clients
        self.read_q_mutex = threading.Lock()
        self.read_q_list = [queue.Queue(), ]
        print("==== serial init self.read_q_list:", self.read_q_list)
        print("==== serial init self.read_q_list len:", len(self.read_q_list))
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
        b = self.read_q_list[0].get()
        l.append(b)
        while not self.read_q_list[0].empty():
            b = self.read_q_list[0].get()
            l.append(b)
        return b''.join(l)

    def write(self, data):
        self.write_q.put(data)
    
    '''@return (read queue, write queue)'''
    def request_r_w_queues(self):
        self.read_q_mutex.acquire()
        self.read_q_list.append(queue.Queue())
        self.read_q_mutex.release()
        return (self.read_q_list[-1], self.write_q)

    def release_r_w_queues(self, r_q, w_q):
        self.read_q_list.remove(r_q)



