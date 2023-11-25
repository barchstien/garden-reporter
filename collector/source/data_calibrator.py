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
            # get config entry corresponding to serial
            p = self.probe_from_id(record['source_id'])
            # get calibration factors
            battery_v_divider = float(p['battery']['v-divider'])
            temp_coef = float(p['temperature']['coef'])
            temp_offset = float(p['temperature']['offset'])
            moisture_dry = float(p['soil-moisture']['dry-value'])
            moisture_immerged = float(p['soil-moisture']['immerged-value'])
            light_coef = float(p['light']['coef'])
            light_offset = float(p['light']['offset'])
            # get location
            record['location'] = p['location']
        except Exception as e:
            print('Error while applying calibration of record:', record, '\nException :', e)
            return
        if p == None:
            print('ERROR, dropping record:', record)
            return
        
        # raw values
        print("{} >{:04x} rssi:{:d} loc:{} adcs: {: 5.1f} {: 5.1f} {: 5.1f} {: 5.1f}".format(
            datetime.now(), record['source_id'], record['local_rssi'], record['location'], 
            record['battery_level'], record['soil_moisture'], record['temp'], record['light']))
        
        # Battery
        # ADC to V (1023 = 1.2V)
        record['battery_level'] = (record['battery_level']*1.2/1023.0) * battery_v_divider
        
        # TODO use 5K resistor for indoor (max 1333 Lux)
        # Light
        # light : 18mA <=> 100 000 Lux
        # R = 68 Ohm (or whatever config yaml says)
        # max : 1.2 / R = 17.6mA = 98 039 Lux
        # Lux = I * 100 000 / 0.018
        # I = U / R
        # ---> Lux = U / R * 100 000 / 0.018
        # U = ADC * 1.2 / 1023
        # ---> Lux = ADC * 1.2 / 1023 / R * 100 000 / 0.018
        # Ignore above ...
        # Apply coef and offset at found with ref and linear regression
        record['light'] = record['light'] * light_coef + light_offset
        # round and trunkate to integer
        record['light'] = int(record['light'] + 0.5)
        
        # Temperature
        # ADC to V (1023 = 1.2V)
        # 750mV at 25deg, 10mV per deg (from datasheet)
        # --> deg = V*100 - 50
        # --> deg = ADC * 1.2/1023 * 100 - 50
        # --> deg = ADC * 0.1173 - 50
        # This is datasheet calibration factors
        # To calibrate it, record raw ADC values vs trustful referebce
        # Linear regression on spreedsheat gives a and b with
        # --> deg = ADC * a + b
        record['temp'] = record['temp'] * temp_coef + temp_offset
        
        # Soil Moisture
        # ADC value to %
        # min = immerged
        # max = dry
        # range = max - min
        # % = 100 - (record - min) / range * 100
        moisture_range = moisture_dry - moisture_immerged
        if moisture_range != 0:
            record['soil_moisture'] = \
                100.0 - (float(record['soil_moisture']) - moisture_immerged) / moisture_range * 100.0
            # round and make an integer
            record['soil_moisture'] = int(record['soil_moisture'] + 0.5)
        else:
            record['soil_moisture'] = int(0)
        
        
        print("{} >{:04x} rssi:{:d} loc:{} adcs: {: 5.1f} {: 5.1f} {: 5.1f} {: 5.1f}".format(
            datetime.now(), record['source_id'], record['local_rssi'], record['location'],
            record['battery_level'], record['soil_moisture'], record['temp'], record['light']))


