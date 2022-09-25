import serial

'''
Connect to Xbee via Serial. Allows TCP client to read/Write to the same port.
Data received from XBEE is duplictaed to any connected clients
'''
class SerialHub:

    def __init__(self, config):
        self.port = config['xbee']['port']
        self.baud = config['xbee']['baudrate']
        self.serial = serial.Serial(
            port = self.port,
            baudrate = self.baud,
            parity = serial.PARITY_NONE,
            stopbits = serial.STOPBITS_ONE,
            bytesize = serial.EIGHTBITS,
            timeout = 0.1 # sec
        )

    def read(self, n_bytes):
        return self.serial.read(n_bytes)


