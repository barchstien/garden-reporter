import queue

'''Consumes bytes, extract frames, parse for ADC values &co'''
class XbeeFrameDecoder:
    byte_stash = list()
    records = queue.Queue()
    
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
                # parse the frame
                self.parse_frame(frame)
            else :
                # not enough byte to complete the frame
                return


    def parse_frame(self, f) :
        if f[3] == 0x83 :
            record = dict()
            # 16 bit IO Sample Indicator
            record['source_id'] = int(f[4]) << 8 + f[5]
            record['rssi'] = -1 * f[6]
            # expect 4 samples
            if f[8] != 1 :
                print('drop frame coz received ', f[8], 'samples instead of 1')
                return
            # expect bitmask with ADC{0-4} enabled
            if f[9] != 0b00011110 or f[10] != 0 :
                print('drop frame coz it\'s not ADC but', "{0:08b} {1:08b}".format(f[9], f[8]))
            # extract ADC values
            # 0 - battery level
            # 1 - soil moisture
            # 2 - temperature
            # 3 - light
            record['battery_level'] = int.from_bytes(f[11:13], "big")
            record['soil_moisture'] = int.from_bytes(f[13:15], "big")
            record['temp'] = int.from_bytes(f[15:17], "big")
            record['light'] = int.from_bytes(f[17:19], "big")
            record['measurement'] = "proto4"
            
            self.records.put(record)
        # other frames ignored
        else:
            # log it for curiosity
            print('drop frame :', f)


'''
RX (Receive) Packet 16-bit Address IO (API 1)

7E 00 10 83 01 00 2F 00 01 1E 00 02 A8 02 20 02 83 00 00 DC

Start delimiter: 7E
Length: 00 10 (16)
Frame type: 83 (RX (Receive) Packet 16-bit Address IO)
16-bit source address: 01 00
RSSI: 2F
Options: 00
Number of samples: 01
Digital channel mask: 00 00
Analog channel mask: 1E 00
Sample 1: - DIO0/AD0 analog value: 02 A8 (680)
- DIO1/AD1 analog value: 02 20 (544)
- DIO2/AD2 analog value: 02 83 (643)
- DIO3/AD3 analog value: 00 00 (0)

Checksum: DC
'''
