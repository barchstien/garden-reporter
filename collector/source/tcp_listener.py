from serial_hub import *
import threading, socket

'''
Listen on given ip:port

'''
class TcpListener:

    def __init__(self, config, serial_hub):
        # config
        self.host = config['serial-tcp']['host']
        self.port = config['serial-tcp']['port']
        self.serial_hub = serial_hub
        # create/bind socket
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        print('Listenning on: ', self.host, self.port)
        self.sock.bind((self.host, int(self.port)))
        # thread
        self.thread_run = threading.Event()
        self.thread_run.set()
        self.thread = threading.Thread(target=self.thread_handler)
        self.thread.start()
    
    def stop(self):
        self.thread_run.clear()
        self.sock.close()
        self.thread.join()
    
    def thread_handler(self):
        self.sock.settimeout(1.0)
        while self.thread_run.is_set():
            self.sock.listen(1)
            try:
                connection, client_address = self.sock.accept()
            except socket.timeout:
                continue
            except:
                print("TcpListener stop accepting")
                break;
            print("Accepted connection from:", client_address)
            connection.settimeout(0.01)
            # get serial read and write queues
            (self.hub_r_q, self.hub_w_q) = self.serial_hub.request_r_w_queues()
            while self.thread_run.is_set():
                try:
                    # receive from tcp client
                    try:
                        data = connection.recv(4096)
                        if len(data) == 0:
                            break;
                        self.hub_w_q.put(data)
                    except TimeoutError:
                        pass
                    # send to tcp client
                    cnt = 0
                    while not self.hub_r_q.empty():# and cnt <= 4096:
                        b = self.hub_r_q.get()
                        cnt += len(b)
                        connection.sendall(b)
                except BrokenPipeError:
                    print("BrokenPipeError")
                    break;
                except Exception as e:
                    print("tcp connection exception:", e.what())
                    break;
            # release serial_hub queues
            self.serial_hub.release_r_w_queues(self.hub_r_q, self.hub_w_q)
            connection.close()
            print("Connection closed")

