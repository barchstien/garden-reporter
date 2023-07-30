from serial_hub import *
import threading, socket, select

'''
Listen on given ip:port
Interpret the first 8 bytes received as the MAC of the endpoint that should be held in config
Remaining bytes read/written are forwarded to collector serial
'''
class TcpListener:

    STATUS_READY = 0 # requested endpoint is awake
    STATUS_WAIT = 1  # ask client to wait, test that link is up
    STATUS_ERROR = 2 # error, try again later

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
        # future single client socket, returned by accept()
        self.connection = None
        # thread
        self.thread_stop = threading.Event()
        self.thread_stop.clear()
        self.thread = threading.Thread(target=self.thread_handler)
        self.thread.start()
        #
        self.xbee_held = None
    
    def stop(self):
        self.thread_stop.set()
        # cancel accept()
        self.sock.shutdown(socket.SHUT_RDWR)
        if self.connection != None:
            # cancel client recv()
            self.connection.shutdown(socket.SHUT_RDWR)
        self.thread.join()
        self.sock.close()
    
    def thread_write_to_stream(self, hub_r_q):
        while not self.thread_stop.is_set():
            try:
                # send to tcp client
                # blocks until it gets some
                b = hub_r_q.get()
                if b == None:
                    # happens when got to stop
                    break
                self.connection.sendall(b)
            except BrokenPipeError:
                print("BrokenPipeError, quit thread_write_to_stream()")
                break
            except Exception as e:
                print("tcp connection exception, quit thread_write_to_stream(), what:", e)
                break
    
    def thread_handler(self):
        while not self.thread_stop.is_set():
            self.sock.listen(1)
            try:
                # blocks here, canceled by shutdown()
                self.connection, client_address = self.sock.accept()
            except:
                print("TcpListener stop accepting")
                break
            print("Accepted connection from:", client_address)
            
            # Wait for 8 bytes that could be
            #  - MAC of endpoint to hold in config
            #  - 0 no endpoint is held in config
            #    usefull to config collector without having to wait
            #    which is usefull when an xbee is already stuck in "no sleep"
            #  - TODO 0xffffffffffffffff to hold all registered endpoints
            # 3 sec is long enough for client to request a MAC to hold
            self.connection.settimeout(3.0)
            # receives 8 first bytes as command
            try:
                print('--- TCP Listener waiting for first 8 bytes')
                mac_bytes = self.connection.recv(8)
                if len(mac_bytes) == 0:
                    raise
                print('--- first 8 bytes:', mac_bytes.hex())
                # waits until target is held in config
                held_mac = int.from_bytes(mac_bytes, 'big')
                if held_mac == 0:
                    self.connection.sendall((TcpListener.STATUS_READY).to_bytes(1, byteorder='big'))
                    print ('--- Tcp Listener no MAC to wait for')
                else:
                    self.xbee_held = self.xbee_pop.hold_in_config(held_mac)
                    if self.xbee_held == None:
                        # no such mac, return error to client
                        self.connection.sendall((TcpListener.STATUS_ERROR).to_bytes(1, byteorder='big'))
                        raise
                    # wait for target to be held
                    while not self.xbee_held.config_holding.is_set():
                        time.sleep(1.0)
                        # send a byte every sec, to check that client is connected
                        # if it fails, exception is raised, and tcp socket is closed
                        self.connection.sendall((TcpListener.STATUS_WAIT).to_bytes(1, byteorder='big'))
                    # target is up, notify client
                    self.connection.sendall((TcpListener.STATUS_READY).to_bytes(1, byteorder='big'))
                    print('--- TCP Listener target is up (no cycle sleep)')
            except Exception as e:
                print('TCP Listener failed to get MAC, closing...\nwhat(): ', e)
                self.connection.close()
                self.connection = None
                if self.xbee_held != None:
                    self.xbee_pop.release_from_config(self.xbee_held.mac)
                continue
            
            ### now going to normal serial <--> tcp mode
            # get serial read and write queues
            (hub_r_q, hub_w_q) = self.serial_hub.request_r_w_queues()
            # set as non-blocking, to read whatever is available using select()
            self.connection.settimeout(0)
            # start write thread
            w_thread = threading.Thread(target=self.thread_write_to_stream, 
                args=(hub_r_q,)
            )
            w_thread.start()
            # this thread does recv
            while not self.thread_stop.is_set():
                can_read, can_write, events = select.select([self.connection], [], [])
                if self.connection in can_read:
                    # receive from tcp client
                    data = self.connection.recv(4096)
                    if len(data) > 0:
                        hub_w_q.put(data)
                    else:
                        print('TCP Listener read 0 bytes, exit')
                        break
                else:
                    print('TCP Listener select returned for nothing, shouldnt happen, exit')
                    break
            
            # stop write thread
            hub_r_q.put(None)
            w_thread.join()
            # release serial_hub queues
            self.serial_hub.release_r_w_queues(hub_r_q, hub_w_q)
            # release from config
            self.xbee_pop.release_from_config(held_mac)
            # close current client connection
            self.connection.close()
            self.connection = None
            print("Connection closed")


