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
        self.url = config['influxdb']['hostname']
        # Get influxdb API token and friends from env_file
        self.token = os.environ['INFLUX_TOKEN']
        self.org = os.environ['ORG']
        self.bucket = os.environ['BUCKET']
        self.config = config
        # debug. It's ok to print coz whoever has access to logs has access to env_file
        print(f'Token: {self.token}\nOrg: {self.org}\nBucket: {self.bucket}\nConfig: {self.config}')
        
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
        measurement = self.config['influxdb']['measurement']['probe']
        # if calibration is enabled, store as calibration measurement
        if record.get('calibration', False):
            measurement = self.config['influxdb']['measurement']['probe-calibration']
        try:
            p = Point(
                measurement) \
                .tag("probe_id", record['source_id'])\
                .tag("location", record['location'])\
                .field("batt", float(record['battery_level']))\
                .field("moist", float(record['soil_moisture']))\
                .field("temp", float(record['temp']))\
                .field("light", int(record['light']))\
                .field("rssi", float(record['local_rssi']))
            if record['error_cca'] != None:
                p.field("error_cca", record['error_cca'])
            if record['error_ack'] != None:
                p.field("error_ack", record['error_ack'])
            if record['v_supply'] != None:
                p.field("v_supply", record['v_supply'])
            self.write_api.write(bucket=self.bucket, record=p)
        except Exception as e:
            print('Cant reach influx-db for record:', record)
            print(e)
    
    def write_water_counter(self, record):
        try:
            p = Point(
                self.config['influxdb']['measurement']['water-web-control']) \
                .field("water", float(record['water_liter']))\
                .field("battery", float(record['battery_volt']))\
                .field("rssi", float(record['rssi_dbm']))\
                .field("temp", float(record['temp_celsius']))
            self.write_api.write(bucket=self.bucket, record=p)
        except Exception as e:
            print('Cant reach influx-db for record:', record)
            print(e)
