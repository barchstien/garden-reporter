import serial

class SerialHub:

    def __init__(self):
        self.serial = serial.Serial(
            port = '/dev/ttyUSB0',
            baudrate = 9600,
            parity = serial.PARITY_NONE,
            stopbits = serial.STOPBITS_ONE,
            bytesize = serial.EIGHTBITS,
            timeout = 0.5 # sec
        )

    def read(self, n_bytes):
        return self.serial.read(n_bytes)


