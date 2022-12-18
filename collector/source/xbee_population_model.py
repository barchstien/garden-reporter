from xbee_frame_decoder import *
import threading, queue, time
from datetime import datetime, timedelta

'''
An Xbee Awakening is when an xbee comes out of cycle sleep
It notifies itslef by an XbeeFrameDecoder.API_64_BIT_IO_SAMPLE
This marks the beginning of the awakening
All data related to the very awakening is represented by that class
Things such as :
 * the API_64_BIT_IO_SAMPLE initial frame
 * request to pull data
 * response received regarding all the data to pull
 * the build of a record aggregating various pulled data
'''
class XbeeAwakening:
    TIMEOUT_SEC = 10.0
    
    def __init__(self, io_sample_frame):
        # io_sample received upon awakening
        self.io_sample_frame = io_sample_frame
        # timestamp
        self.start = datetime.now()
        # record that will be sent to influxdb
        self.record = {
            'source_id' : io_sample_frame['source_id'],
            'local_rssi': io_sample_frame['rssi'],
            'error_cca' : None,
            'error_ack' : None,
            'v_supply'  : None
        }
        # log messages sent and their answers
        self.log = ()
        # debug
        self.debug = 1


'''
Represent an Xbee endpoint
Implements the logic required to interface with a single xbee
... in order to pull data out of it (ADCs, rssi, error cnt, etc)
'''
class Xbee:
    TIMEOUT_ERROR_POLL = timedelta(hours=24)
    
    def __init__(self, mac):
        self.mac = mac
        # incoming decoded messages
        self.incoming = []
        # awakening session
        self.awakening = None
        # counter for frame_id
        self.frame_id = -1
        # errors last values
        self.error_ack = -1
        self.error_cca = -1
        self.error_last_time = datetime.min
        # config mode
        # when enable, that device is connected to XCTU
        # and collector does not read/write anything to that device
        self.config_mode = threading.Event()
        self.config_holding = threading.Event()
    
    def get_next_frame_id(self):
        self.frame_id = (self.frame_id + 1) % 256
        if self.frame_id == 0:
            self.frame_id += 1
        return self.frame_id
    
    '''
    Consume frames, look for timeout, send request to endpoint, create records for db
    @return (frames_to_send, records_to_write)
    '''
    def do_your_thing(self):
        to_send = []
        to_rec = []
        
        # check that current awakening hasn't timed out
        if self.awakening != None:
            if (datetime.now() - self.awakening.start).total_seconds() > XbeeAwakening.TIMEOUT_SEC:
                # awakening has timed out, delete it
                self.awakening = None
        
        # check if Cycle sleep should be re-enabled
        if not self.config_mode.is_set() and self.config_holding.is_set():
            print(str(hex(self.mac)) + ' Re-enable Cycle Sleep !')
            to_send.append({
                'type': XbeeFrameDecoder.API_REMOTE_AT_REQUEST,
                'frame_id': self.get_next_frame_id(),
                'dest_id': self.mac,
                'options': 0x02, # apply changes now
                'AT': XbeeFrameDecoder.AT_SM_CYCLE_SLEEP,
                'AT_value' : XbeeFrameDecoder.AT_SM_VALUE_CYCLE_SLEEP
            })
            self.config_holding.clear()
        
        # process incoming packets
        while len(self.incoming) > 0:
            in_f = self.incoming[0]
            self.incoming.pop(0)
            
            # deal with conifg mode
            if self.config_mode.is_set() or self.config_holding.is_set():
                if not self.config_holding.is_set():
                    print(str(hex(self.mac)) + ' Disable Cycle Sleep !')
                    # Just consider that it always returns OK
                    to_send.append({
                        'type': XbeeFrameDecoder.API_REMOTE_AT_REQUEST,
                        'frame_id': self.get_next_frame_id(),
                        'dest_id': self.mac,
                        'options': 0x02, # apply changes now
                        'AT': XbeeFrameDecoder.AT_SM_CYCLE_SLEEP,
                        'AT_value' : XbeeFrameDecoder.AT_SM_VALUE_NO_SLEEP
                    })
                    self.config_holding.set()
                else:
                    print(str(hex(self.mac)) + ' Held in config, ignore rx data')
                # avoid the rest of the loop, including ACK of sleep off cmd
                # ... until config mode is exited
                continue
            
            #print('### in_f:', in_f)
            if in_f['type'] == XbeeFrameDecoder.API_64_BIT_IO_SAMPLE:
                # This is considered as a wakeup frame
                # ADCs values are ignored coz sensors need startup delay
                self.awakening = XbeeAwakening(in_f)
                
                # DEBUG
                # TODO use timeout instead of wait if this proves useful
                # delay request by 20 msec
                # ... to give sensors time to load up
                #time.sleep(0.02)
                
                if datetime.now() - self.error_last_time > Xbee.TIMEOUT_ERROR_POLL :
                    # poll errors and xbee voltage, coz it's been a while
                    to_send.append({
                        'type': XbeeFrameDecoder.API_REMOTE_AT_REQUEST,
                        'frame_id': self.get_next_frame_id(),
                        'dest_id': in_f['source_id'],
                        'AT': XbeeFrameDecoder.AT_V_SUPPLY_MONITOR
                    })
                    self.error_last_time = datetime.now()
                else :
                    to_send.append({
                        'type': XbeeFrameDecoder.API_REMOTE_AT_REQUEST,
                        'frame_id': self.get_next_frame_id(),
                        'dest_id': in_f['source_id'],
                        'AT': XbeeFrameDecoder.AT_IS_FORCE_SAMPLE
                    })
            elif self.awakening == None:
                # no need to go any further if no awakening is going on
                print('no awakening unexpected incoming frame:', in_f)
                break
            elif in_f['frame_id'] != self.frame_id:
                print('Error, received frame_id({}) does not match expected frame_id({})'.format(
                    in_f['frame_id'], self.frame_id
                ))
                break
            elif in_f['type'] == XbeeFrameDecoder.API_REMOTE_AT_RESPONSE:
                if in_f['AT'] == XbeeFrameDecoder.AT_V_SUPPLY_MONITOR:
                    ## xbee monitor its own supply
                    self.awakening.record['v_supply'] = in_f['v_supply']
                    to_send.append({
                        'type': XbeeFrameDecoder.API_REMOTE_AT_REQUEST,
                        'frame_id': self.get_next_frame_id(),
                        'dest_id': in_f['source_id'],
                        'AT': XbeeFrameDecoder.AT_EC_CCA_FAILURE
                    })
                elif in_f['AT'] == XbeeFrameDecoder.AT_EC_CCA_FAILURE:
                    error_cca = in_f['error_cca']
                    print("raw error CCA received:", in_f['error_cca'])
                    if self.error_cca != -1:
                        # get diff from last value received
                        error_cca = in_f['error_cca'] - self.error_cca
                        if error_cca < 0:
                            # probe has been rebooted, counter is reset
                            error_cca = in_f['error_cca']
                    else:
                        # first read, assume no error that round
                        error_cca = 0
                    self.error_cca = in_f['error_cca']
                    self.awakening.record['error_cca'] = error_cca
                    to_send.append({
                        'type': XbeeFrameDecoder.API_REMOTE_AT_REQUEST,
                        'frame_id': self.get_next_frame_id(),
                        'dest_id': in_f['source_id'],
                        'AT': XbeeFrameDecoder.AT_EA_ACK_FAILURE
                    })
                elif in_f['AT'] == XbeeFrameDecoder.AT_EA_ACK_FAILURE:
                    error_ack = in_f['error_ack']
                    if self.error_ack != -1:
                        # get diff from last value received
                        error_ack = in_f['error_ack'] - self.error_ack
                        if error_ack < 0:
                            # probe has been rebooted, counter is reset
                            error_ack = in_f['error_ack']
                    else:
                        # first read, assume no error that round
                        error_ack = 0
                    self.error_ack = in_f['error_ack']
                    self.awakening.record['error_ack'] = error_ack
                    to_send.append({
                        'type': XbeeFrameDecoder.API_REMOTE_AT_REQUEST,
                        'frame_id': self.get_next_frame_id(),
                        'dest_id': in_f['source_id'],
                        'AT': XbeeFrameDecoder.AT_IS_FORCE_SAMPLE
                    })
                elif in_f['AT'] == XbeeFrameDecoder.AT_DB_LAST_RSSI:
                    ### NOT USED
                    self.awakening.record['remote_rssi'] = in_f['remote_rssi']
                    to_send.append({
                        'type': XbeeFrameDecoder.API_REMOTE_AT_REQUEST,
                        'frame_id': self.get_next_frame_id(),
                        'dest_id': in_f['source_id'],
                        'AT': XbeeFrameDecoder.AT_IS_FORCE_SAMPLE
                    })
                elif in_f['AT'] == XbeeFrameDecoder.AT_IS_FORCE_SAMPLE:
                    self.awakening.record['battery_level'] = in_f['battery_level']
                    self.awakening.record['soil_moisture'] = in_f['soil_moisture']
                    self.awakening.record['temp'] = in_f['temp']
                    self.awakening.record['light'] = in_f['light']
                    to_rec.append(self.awakening.record)
                    '''to_send.append({
                        'type': XbeeFrameDecoder.API_REMOTE_AT_REQUEST,
                        'frame_id': self.get_next_frame_id(),
                        'dest_id': in_f['source_id'],
                        'AT': XbeeFrameDecoder.AT_IS_FORCE_SAMPLE
                    })'''
                    print('awakening end after sec:', 
                        (datetime.now() - self.awakening.start).total_seconds())
                    # no more to request, clean up awakening
                    self.awakening = None
        return (to_send, to_rec)
    
    def __str__(self):
        s = [
            "{}".format(self.mac),
        ]
        return "\n".join(s)


'''
Keep an up to date model of xbee endpoints
Endpoints are noticed when they wake up, then probed
In practice XbeePopulationModel :
 * maintains a list of endpoint as defined in config
 * is executed by independent thread, to handle timing easily
 * consumes frames coming from the XbeeFrameDecoder
 * emit dict() that can be consumed by XbeeFrameEncoder
'''
class XbeePopulationModel:
    def __init__(self, config):
        # receive from decoder
        self.incoming = queue.Queue()
        # send to encoder
        self.outgoing = queue.Queue()
        # data to write to db
        self.db_records = queue.Queue()
        # list of Xbee objects
        self.register = []
        for p in config['probes']:
            self.register.append(Xbee(p['id']))
    
    def execute(self):
        # distribute frames to endpoint it is destined to
        while not self.incoming.empty():
            f = self.incoming.get()
            delivered = False
            for p in self.register:
                if p.mac == f['source_id']:
                    p.incoming.append(f)
                    delivered = True
                    break
            if not delivered:
                print('Warning, frame not delivered coz unknown destination mac:', \
                    hex(f['source_id']), "\n", f)
                

        # execute states of each endpoint
        for ep in self.register:
            # do logic of each xbee
            (frames, records) = ep.do_your_thing()
            # enque frames to send via collector
            while len(frames) > 0:
                self.outgoing.put(frames[0])
                frames.pop(0)
            # enque records to be put to influxdb
            while len(records) > 0:
                self.db_records.put(records[0])
                records.pop(0)


    def __str__(self):
        l = []
        for p in self.register:
            l.append("  - " + str(p))
        return "Population :\n" + "\n".join(l)

    '''Consume frames received by collector and decoded as dict'''
    def consume(self, frame):
        if frame != None:
            self.incoming.put(frame)
        self.execute()

    '''@return frames to send via (write to) collector'''
    def frames_to_send(self):
        l = []
        while not self.outgoing.empty():
            l.append(self.outgoing.get())
        return l
    
    '''@return frames to write to influxdb'''
    def frames_to_record(self):
        l = []
        while not self.db_records.empty():
            l.append(self.db_records.get())
        return l
    
    '''
    Set the Xbee with specified mac as config.
    Meaning the next time the endpoint wakes up, Cycle Sleep is disabled.
    '''
    def hold_in_config(self, mac):
        for p in self.register:
            if p.mac == mac:
                p.config_holding.clear()
                p.config_mode.set()
                return p
        print('Cannot hold_in_config, no such mac:', mac)
        return None
    
    '''Release endpoint from config, ie enable back Cycle Sleep'''
    def release_from_config(self, mac):
        for p in self.register:
            if p.mac == mac:
                p.config_mode.clear()
                return
        print('Cannot release_from_config, no such mac:', mac)


# Debug test
if __name__ == "__main__":
    pop_model = XbeePopulationModel(
        {"probes": [
            {"id": "prout"},
            {"id": "blop"},
            {"id": "croute"},
        ]}
    )
    print(pop_model)
