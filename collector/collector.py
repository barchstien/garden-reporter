import serial
from xbee_frame_decoder import *
from influxdb_writer import *
from data_calibrator import *

if __name__ == "__main__":
    
    serial = serial.Serial(
        port = '/dev/ttyUSB0',
        baudrate = 9600,
        parity = serial.PARITY_NONE,
        stopbits = serial.STOPBITS_ONE,
        bytesize = serial.EIGHTBITS,
        timeout = 1
    )
    
    decoder = XbeeFrameDecoder()
    data_calib = DataCalibrator()
    db_writer = InfluxDBWriter()
    
    while True:
        b = serial.read(10)
        if len(b) > 0 :
            decoder.consume(b)
            if not decoder.records.empty():
                r = decoder.records.get()
                data_calib.apply(r)
                db_writer.write(r)

