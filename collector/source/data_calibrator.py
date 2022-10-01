from datetime import datetime

'''Apply calibration for various sensors'''
class DataCalibrator:
    def __init__(self, config):
        self.probes = config['probes']


    def probe_from_id(self, probe_id):
        for p in self.probes:
            if p['id'] == probe_id:
                return p
        # probe not found
        print('ERROR, cannot find probe with id {}\nProbe profiles are {}'.format(probe_id, self.probes))
        return None

    def apply(self, record):
        try:
            p = self.probe_from_id(record['source_id'])
            battery_v_divider = float(p['battery']['v-divider'])
            temp_offset = float(p['temperature']['offset'])
        except Exception as e:
            print('Error while applying calibration of record:', record, '\nExcpetion :', e)
            return
        if p == None:
            print('ERROR, dropping record:', record)
            return
        
        # ADC to V (1.2V 1024 resolution
        # v divider 6.25
        record['battery_level'] = (record['battery_level']*1.2/1023) * battery_v_divider
        
        # ADC to V (1.2V 1024 resolution
        # 750mV at 25deg, 10mV per deg
        # --> deg = V*100 - 50
        # and calibration offset
        record['temp'] = (record['temp']*1.2/1023) * 100 - 50 + temp_offset
        
        print("{} >{:04x} rssi:{:d} adcs: {: 5.1f} {: 5.1f} {: 5.1f} {: 5.1f}".format(
            datetime.now(), record['source_id'], record['rssi'], record['battery_level'], record['soil_moisture'], record['temp'], record['light']))


