from serial_hub import *
import socket

class TcpListener:

    def __init__(self, config, serial_hub):
        # config
        self.host = config['serial-tcp']['host']
        self.port = config['serial-tcp']['port']
        self.serial_hub = serial_hub
        # create/bind socket
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        print('Listenning on: ', self.host, self.port)
        self.sock.bind((self.host, int(self.port)))
        # thread
        self.thread_run = threading.Event()
        self.thread_run.set()
        self.thread = threading.Thread(target=self.thread_handler)
        self.thread.start()
    
    def __del__(self):
        self.thread_run.clear()
        self.thread.join()
    
    def thread_handler(self):
        self.sock.listen(1)
        while self.thread_run.is_set():
            connection, client_address = self.sock.accept()
            print("Accepted connection from:", client_address)
            connection.settimeout(0.2)
            # get read and write queues
            (self.hub_r_q, self.hub_w_q) = self.serial_hub.request_r_w_queues()
            while self.thread_run.is_set():
                try:
                    # receive from tcp client
                    data = connection.recv(4096)
                    if len(data) > 0:
                        self.hub_w_q.put(data)
                    # send to tcp client
                    cnt = 0
                    while not self.hub_r_q.empty() and cnt <= 4096:
                        b = self.write_q.get()
                        cnt += len(b)
                        connection.sendall(b)
                except:
                    print("tcp connectino exception")
                    break;
            # release serial_hub queues
            self.serial_hub.release_r_w_queues(self.hub_r_q, self.hub_w_q)
            connection.close()
            print("Connection closed")
