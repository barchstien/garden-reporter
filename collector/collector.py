from serial_hub import *
from xbee_frame_decoder import *
from influxdb_writer import *
from data_calibrator import *

if __name__ == "__main__":
    
    serial = SerialHub()
    
    decoder = XbeeFrameDecoder()
    data_calib = DataCalibrator()
    db_writer = InfluxDBWriter()
    
    while True:
        b = serial.read(100)
        if len(b) > 0 :
            decoder.consume(b)
            while not decoder.records.empty():
                r = decoder.records.get()
                data_calib.apply(r)
                db_writer.write(r)

