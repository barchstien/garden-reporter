import queue
from datetime import datetime

'''Consumes bytes, extract frames, parse for ADC values &co'''
class XbeeFrameDecoder:
    byte_stash = list()
    frames = queue.Queue()
    
    API_SYNC = 0x7e
    API_REMOTE_AT_REQUEST = 0x17
    API_REMOTE_AT_RESPONSE = 0x97
    API_64_BIT_IO_SAMPLE = 0x82
    
    AT_IS_FORCE_SAMPLE = "IS"
    AT_EA_ACK_FAILURE = "EA"
    AT_EC_CCA_FAILURE = "EC"
    AT_DB_LAST_RSSI = "DB"
    AT_FR_SW_RESET = "FR"
    AT_NI_IDENTIFIER = "NI"
    AT_V_SUPPLY_MONITOR = "%V"
    AT_HV_HARDWARE_VERSION = "HV"
    
    AT_SM_CYCLE_SLEEP = "SM"
    
    AT_SM_VALUE_NO_SLEEP = 0
    AT_SM_VALUE_CYCLE_SLEEP = 4
    
    AT_CMD_RESPONSE = {
        0:"OK", 1:"ERROR", 2:"Invalid cmd", 3:"Invalid param", 4:"Tx failure", 5:"Encrypt error"
    }
    
    def consume(self, b):
        # add bytes to working stash
        self.byte_stash = self.byte_stash + list(b)
        if len(self.byte_stash) < 3 :
            # need first 3 bytes to start decode
            # wait for more bytes
            return
        while self.byte_stash[0] != 0x7e :
            print('-- xbee serial stream unsynced: ', (self.byte_stash[0]))
            self.byte_stash.pop(0)
            if len(self.byte_stash) == 0 :
                return
        # loop as long there are at least 3 bytes in stash
        while len(self.byte_stash) >= 3 :
            length = (int(self.byte_stash[1]) << 8) + self.byte_stash[2]
            # length does not include 0x7e and 2 bytes field length
            # ... nor checksum byte
            length += 4
            if len(self.byte_stash) >= length :
                # frame completed
                frame = self.byte_stash[:length]
                # flush completed frame from stash
                self.byte_stash = self.byte_stash[length:]
                # validate frame checksum
                s = 0
                for i in range(3, length) :
                    s += int(frame[i])
                if (s & 0xff) != 0xff :
                    print('Checksum failed !', hex(s))
                    # ignore frame
                    return
                # parse the frame based on type
                self.parse_frame(frame)
            else :
                # not enough byte to complete the frame
                return


    def parse_frame(self, f) :
        frame = dict()
        frame['type'] = f[3]
        if f[3] == XbeeFrameDecoder.API_64_BIT_IO_SAMPLE :
            # 64 bit IO Sample Indicator
            frame['source_id'] = int.from_bytes(f[4:12], byteorder='big')
            frame['rssi'] = -1 * f[12]
            # 1 sample containes 4 ADC values
            num_of_samples = f[14]
            # expect bitmask with ADC{0-4} enabled
            if f[15] != 0b00011110 or f[16] != 0 :
                print('drop frame coz it\'s not ADC (0b00011110) but', 
                    "{0:08b} {1:08b}".format(f[15], f[16]))
            # extract ADC values
            # 0 - battery level
            # 1 - soil moisture
            # 2 - temperature
            # 3 - light
            frame['battery_level'] = float(int.from_bytes(f[17:19], "big"))
            frame['soil_moisture'] = int.from_bytes(f[19:21], "big")
            frame['temp'] = float(int.from_bytes(f[21:23], "big"))
            frame['light'] = int.from_bytes(f[23:25], "big")
            
            self.frames.put(frame)
            
        elif f[3] == self.API_REMOTE_AT_RESPONSE :
            # Remote Command Response
            frame['frame_id'] = f[4]
            frame['source_id'] = int.from_bytes(f[5:13], byteorder='big')
            frame['AT'] = ''.join([chr(f[15]), chr(f[16])])
            # status error
            status = f[17]
            if status != 0x00:
                print("{}".format(datetime.now()),
                    "AT cmd:", frame['AT'], 
                    "returned non OK (0): ", status, 
                    ", ie ", XbeeFrameDecoder.AT_CMD_RESPONSE[status])
                return
            #print("---- AT response:", frame['AT'])
            if frame['AT'] == self.AT_IS_FORCE_SAMPLE:
                # expect 1 sample, contains 4 ADC values
                num_of_samples = f[18]
                # expect 00011110 which enables A3 to A0
                if f[19] != 0b00011110 or f[20] != 0 :
                    print('drop frame coz it\'s not (0b00011110) 0x0 but', 
                        "{0:08b} {1:08b}".format(f[19], f[20]))
                
                frame['battery_level'] = float(int.from_bytes(f[21:23], "big"))
                frame['soil_moisture'] = int.from_bytes(f[23:25], "big")
                frame['temp'] = float(int.from_bytes(f[25:27], "big"))
                frame['light'] = int.from_bytes(f[27:29], "big")
            elif frame['AT'] == self.AT_DB_LAST_RSSI:
                frame['remote_rssi'] = -1 * f[18]
            elif frame['AT'] == self.AT_EA_ACK_FAILURE:
                frame['error_ack'] = int.from_bytes(f[18:20], "big")
            elif frame['AT'] == self.AT_EC_CCA_FAILURE:
                frame['error_cca'] = int.from_bytes(f[18:20], "big")
            elif frame['AT'] == self.AT_V_SUPPLY_MONITOR:
                frame['v_supply'] = int.from_bytes(f[18:20], "big")
            
            self.frames.put(frame)
        
        else:
            # other frames are ignored
            # log it for curiosity
            print('Ignore incoming frame (unsupported type :', f)


