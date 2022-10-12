from data_calibrator import *
from influxdb_writer import *
from serial_hub import *
from tcp_listener import *
from xbee_frame_decoder import *
from xbee_frame_encoder import *
from xbee_population_model import *

import yaml, time
import sys, signal


if __name__ == "__main__":
    # open and load config file
    f = open('collector.yaml')
    config = yaml.load(f, Loader=yaml.FullLoader)
    f.close()

    # owns serial to collector
    serial = SerialHub(config)
    # allow TCP connection to serial (for XCTU)
    tcp_listener = TcpListener(config, serial)
    # extracte frames and decode it to dict
    decoder = XbeeFrameDecoder()
    # population model, keep tracks of enpoint
    population_model = XbeePopulationModel(config)
    # apply calib and makes meaningful values from ADC readings
    data_calib = DataCalibrator(config)
    # write to influxdb
    db_writer = InfluxDBWriter(config)
    
    encoder = XbeeFrameEncoder()
    
    
    (hub_r_q, hub_w_q) = serial.request_r_w_queues()
    
    try:
        while True:
            # cnt for bytes received, bytes to send, and records to write
            cnt = 0
            
            #### RX
            if not hub_r_q.empty():
                b = hub_r_q.get()
                if len(b) > 0 :
                    cnt += len(b)
                    # get frame from bytes
                    decoder.consume(b)
                    # deal with all received/decoded frames
                    while not decoder.frames.empty():
                        f = decoder.frames.get()
                        #print('>>> f:', f)
                        population_model.consume(f)
            
            #### TX
            # Check population model for frames to send
            f_to_send = population_model.frames_to_send()
            for f in f_to_send:
                f_coded = encoder.encode(f)
                cnt += len(f_coded)
                hub_w_q.put(f_coded)
                pass
            
            #### Record (influxdb)
            # Check population model for records to write to db
            f_to_record = population_model.frames_to_record()
            for f in f_to_record:
                data_calib.apply(f)
                db_writer.write(f)
                cnt += 1
                pass
            
            # if nothing received, sent or written, sleep
            if cnt == 0:
                time.sleep(0.1)
    except KeyboardInterrupt:
        print("Keyboard Interrupt ------> exiting all...")

    serial.stop()
    tcp_listener.stop()
    population_model.stop()
    
    print("Aaarrggh..........")




