from influxdb_client.client.write_api import SYNCHRONOUS
from influxdb_client import InfluxDBClient, Point
import os

'''Get influxdb secrets from env and expose a function to write a record'''
class InfluxDBWriter:
    # You can generate an API token from the "API Tokens Tab" in the UI
    token = "set by env_file, see Readme.md"
    org = "set by env_file"
    bucket = "set by env_file"
    client = None

    def __init__(self, config):
        self.url         = config['influxdb']['hostname']
        self.measurement = config['influxdb']['measurement']
        # Get influxdb API token and friends from env_file
        self.token = os.environ['INFLUX_TOKEN']
        self.org = os.environ['ORG']
        self.bucket = os.environ['BUCKET']
        
        self.client = InfluxDBClient(
            url = self.url,
            token = self.token,
            org = self.org
        )
        self.write_api = self.client.write_api(write_options=SYNCHRONOUS)
    
    def __exit__(self, exc_type, exc_value, traceback):
        self.client.close()
    
    def write(self, record):
        #print('InfluxDBWriter write:', record)
        try:
            record['measurement'] = self.measurement
            p = Point(
                record['measurement'])\
                .tag("probe_id", record['source_id'])\
                .field("batt", record['battery_level'])\
                .field("moist", record['soil_moisture'])\
                .field("temp", record['temp'])\
                .field("light", record['light'])\
                .field("rssi", record['local_rssi'])
            # disabled writting to avoid recording zero coz it's not pulled
            # TODO get error count once a day or so, else put null
            '''    .field("remote_rssi", record['remote_rssi'])\
                .field("error_cca", record['error_cca'])\
                .field("error_ack", record['error_ack'])'''
            self.write_api.write(bucket=self.bucket, record=p)
        except Exception as e:
            print('Cant reach influx-db')
            print(e)
