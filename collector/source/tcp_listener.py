from serial_hub import *
import threading, socket, select

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
        self.thread_stop = threading.Event()
        self.thread_stop.clear()
        self.thread = threading.Thread(target=self.thread_handler)
        self.thread.start()
    
    def stop(self):
        self.thread_stop.set()
        self.sock.shutdown(socket.SHUT_RDWR)
        self.sock.close()
        self.thread.join()
    
    def thread_handler_read(self, s, w_q):
        while not self.thread_stop.is_set():
            try:
                # receive from tcp client
                data = connection.recv(4096)
                if len(data) == 0:
                    break;
                hub_w_q.put(data)
            except TimeoutError:
                    pass
            except BrokenPipeError:
                print("BrokenPipeError")
                break;
            except Exception as e:
                print("tcp connection exception:", e.what())
                break;
    
    def thread_handler_write(self, s, hub_r_q):
        while not self.thread_stop.is_set():
            try:
                # send to tcp client
                # blocks until it gets some
                b = hub_r_q.get()
                if b == None:
                    # happens when got to stop
                    break
                s.sendall(b)
            except BrokenPipeError:
                print("BrokenPipeError")
                break;
            except Exception as e:
                print("tcp connection exception:", e.what())
                break;
        self.thread_stop.set()
    
    def thread_handler(self):
        while not self.thread_stop.is_set():
            self.sock.listen(1)
            try:
                # blocks here, canceled by shutdown()
                connection, client_address = self.sock.accept()
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
            (hub_r_q, hub_w_q) = self.serial_hub.request_r_w_queues()
            # 
            stop_connection = threading.Event()
            stop_connection.clear()
            
            r_thread = thread.Thread(
                target=self.thread_handler_read,
                connection,
                hub_r_q)
            w_thread = thread.Thread(
                target=self.thread_handler_write,
                connection,
                hub_w_q)
            
            self.thread_stop.wait()
            
            # TODO stop r/w
            hub_r_q.put(None)
            connection.shutdown(socket.SHUT_RDWR)
            r_thread.join()
            w_thread.join()
            
            # release serial_hub queues
            self.serial_hub.release_r_w_queues(hub_r_q, hub_w_q)
            # TODO release from config
            self.xbee_pop.release_from_config("blop")
            
            connection.close()
            print("Connection closed")

