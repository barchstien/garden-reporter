from data_calibrator import *
from influxdb_writer import *
from serial_hub import *
from tcp_listener import *
from xbee_frame_decoder import *
from xbee_frame_encoder import *
from xbee_population_model import *
from water_web_server import *

import yaml, time
import sys, signal
import hashlib

CONFIG_PATH = '/var/lib/garden-collector/collector.yaml'
last_config_md5sum = 0

if __name__ == "__main__":
    # open and load config file
    f = open(CONFIG_PATH)
    config = yaml.load(f, Loader=yaml.FullLoader)
    f.close()
    # save md5 
    # TODO check md5sum and reload config if needed
    with open(CONFIG_PATH, 'rb') as file:
        data = file.read()
        last_config_md5sum = hashlib.md5(data).hexdigest()

    # owns serial to xbee collector device
    serial = SerialHub(config)
    # extracte frames and decode it to dict
    decoder = XbeeFrameDecoder()
    # encode frames for xbee network
    encoder = XbeeFrameEncoder()
    # population model, keep tracks of xbee enpoint
    # do the logic to querry and config them
    population_model = XbeePopulationModel(config)
    # Listen TCP connection to share serial (for XCTU)
    tcp_listener = TcpListener(config, serial, population_model)
    # apply calib and makes meaningful values from ADC readings
    data_calib = DataCalibrator(config)
    # write to influxdb
    db_writer = InfluxDBWriter(config)
    # web server for human and for water control unit
    water_web_server = WaterWebServer()
    water_web_server.run_server_in_background()
    
    # Serial hub read and write queues
    # that the main loop read/write from/to
    (hub_r_q, hub_w_q) = serial.request_r_w_queues()
    
    try:
        while True:
        
            #### RX
            # Decode bytes, consume frames
            try:
                b = hub_r_q.get(timeout=0.01)
                if len(b) > 0 :
                    # get frame from bytes
                    decoder.consume(b)
                    # deal with all received/decoded frames
                    while not decoder.frames.empty():
                        f = decoder.frames.get()
                        #print('>>> f:', f)
                        population_model.consume(f)
            except queue.Empty as e:
                # nothing received, but run the pop model
                # ... to release from config for exemple
                population_model.consume(None)
            
            #### TX
            # Check population model for frames to send
            f_to_send = population_model.frames_to_send()
            for f in f_to_send:
                f_coded = encoder.encode(f)
                hub_w_q.put(f_coded)
            
            #### Record (influxdb)
            # Check population model for records to write to db
            f_to_record = population_model.frames_to_record()
            for f in f_to_record:
                data_calib.apply(f)
                db_writer.write(f)

            # Poll web water controller
            water_counter_msg = water_web_server.pop_water_counter_volume_liter()
            while water_counter_msg != None:
                db_writer.write_water_counter(water_counter_msg)
                water_counter_msg = water_web_server.pop_water_counter_volume_liter()
            
    except KeyboardInterrupt:
        print("Keyboard Interrupt ------> exiting all...")

    serial.stop()
    tcp_listener.stop()
    
    print("Aaarrggh..........")




