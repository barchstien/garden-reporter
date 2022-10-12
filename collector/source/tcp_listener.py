from serial_hub import *
import threading, socket

'''
Listen on given ip:port
Interpret the first 8 bytes received as the MAC of the endpoint that should be held in config
Remaining bytes read/written are forwarded to collector serial
'''
class TcpListener:

    def __init__(self, config, serial_hub, xbee_pop):
        # config
        self.host = config['serial-tcp']['host']
        self.port = config['serial-tcp']['port']
        self.serial_hub = serial_hub
        self.xbee_pop = xbee_pop
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
            connection.settimeout(0.1)
            # TODO wait for 8 byte that could be
            #  - MAC of endpoint to hold in config
            #  - 0 no endpoint is held in config
            #  - 0xffffffffffffffff to hold all registered endpoints
            self.xbee_pop.hold_in_config("blop")
            
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
            # TODO release from config
            self.xbee_pop.release_from_config("blop")
            
            connection.close()
            print("Connection closed")

