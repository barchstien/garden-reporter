from serial_hub import *
from xbee_frame_decoder import *
from influxdb_writer import *
from data_calibrator import *
import yaml, time


if __name__ == "__main__":
    # open and load config file
    f = open('collector.yaml')
    config = yaml.load(f, Loader=yaml.FullLoader)
    f.close()

    serial = SerialHub(config)
    decoder = XbeeFrameDecoder()
    data_calib = DataCalibrator(config)
    db_writer = InfluxDBWriter(config)
    
    #while run_loop.is_set():
    while True:
        # blocks when no data available
        b = serial.read()
        if len(b) > 0 :
            decoder.consume(b)
            while not decoder.frames.empty():
                f = decoder.frames.get()
                data_calib.apply(f)
                db_writer.write(f)

