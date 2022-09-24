from datetime import datetime

'''Apply calibration for various sensors'''
class DataCalibrator:
    def apply(self, record):
        # TOFO load from YAML
        # apply ADC V -> unit convertion
        # v divider 6.25
        record['battery_level'] = (record['battery_level']*1.2/1024) * 6.3
        # 750mV at 25deg, 10mV per deg
        # --> deg = V*100 - 50
        # calibration -1.5
        record['temp'] = (record['temp']*1.2/1024) * 100 - 50 - 1.5
        print("{} >{:04x} rssi:{:d} adcs: {: 5.1f} {: 5.1f} {: 5.1f} {: 5.1f}".format(
            datetime.now(), record['source_id'], record['rssi'], record['battery_level'], record['soil_moisture'], record['temp'], record['light']))


