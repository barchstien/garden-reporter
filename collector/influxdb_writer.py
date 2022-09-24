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

    def __init__(self):
        # Get influxdb API token and friends from env_file
        self.token = os.environ['INFLUX_TOKEN']
        self.org = os.environ['ORG']
        self.bucket = os.environ['BUCKET']
        
        self.client = InfluxDBClient(url="http://localhost:8086", 
        token=self.token, org=self.org)
        self.write_api = self.client.write_api(write_options=SYNCHRONOUS)
    
    def __exit__(self, exc_type, exc_value, traceback):
        self.client.close()
    
    def write(self, record):
        try:
            p = Point(record['measurement']).tag("probe_id", record['source_id'])\
                .field("batt", record['battery_level'])\
                .field("moist", record['soil_moisture'])\
                .field("temp", record['temp'])\
                .field("light", record['light'])\
                .field("rssi", record['rssi'])
            self.write_api.write(bucket=self.bucket, record=p)
        except:
            print('Cant reach influx-db')
