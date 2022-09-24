from serial_hub import *
from xbee_frame_decoder import *
from influxdb_writer import *
from data_calibrator import *
import yaml

if __name__ == "__main__":
    
    f = open('collector.yaml')
    config = yaml.load(f, Loader=yaml.FullLoader)
    f.close()

    serial = SerialHub(config)
    
    decoder = XbeeFrameDecoder()
    data_calib = DataCalibrator(config)
    db_writer = InfluxDBWriter(config)
    
    while True:
        b = serial.read(32)
        if len(b) > 0 :
            decoder.consume(b)
            while not decoder.records.empty():
                r = decoder.records.get()
                data_calib.apply(r)
                db_writer.write(r)

