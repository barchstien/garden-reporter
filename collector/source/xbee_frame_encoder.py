from xbee_frame_decoder import *

class XbeeFrameEncoder:

    API_REMOTE_AT_REQUEST_MIN_LEN = 15

    def checksum(self, bytes):
        cs = 0
        for i in range(len(bytes)):
            cs += bytes[i]
        cs = cs % 256
        return 0xff - cs

    '''
    Takes a decoded frame (ie dict) and pack it as Xbee API frame
    @param f {
        'type' : 
        'frame_id' : 
        'dest_id' : 
        'options' : 
        'AT' : 
        'AT_value' : only if setting an AT value, oly SM supported
    }
    @warning all AT command can be encoded for read
             but only SM Cycle Sleep set is supported
    '''
    def encode(self, f):
        #print('encode:', f)
        try:
            if f['type'] == XbeeFrameDecoder.API_REMOTE_AT_REQUEST:
                coded_f = []
                # start delimiter
                coded_f.append((XbeeFrameDecoder.API_SYNC).to_bytes(1, byteorder='big'))
                # length (excludes start, length, checksum --> 4 bytes)
                length = XbeeFrameEncoder.API_REMOTE_AT_REQUEST_MIN_LEN
                # look for value now, so it is included in length
                AT_value_bytes = None
                if 'AT_value' in f:
                    if f['AT'] != XbeeFrameDecoder.AT_SM_CYCLE_SLEEP:
                        raise Excpetion('Setting AT value other than SM Cycle Sleep is not suported')
                    # get value length first, to include it in frame length
                    AT_value_bytes = (f['AT_value']).to_bytes(1, byteorder='big')
                    length += 1
                coded_f.append((length).to_bytes(2, byteorder='big'))
                # frame type, AT cmd request
                coded_f.append((f['type']).to_bytes(1, byteorder='big'))
                # frame id, to correlate reply to request
                coded_f.append((f['frame_id']).to_bytes(1, byteorder='big'))
                # targeted remote device MAC
                coded_f.append((f['dest_id']).to_bytes(8, byteorder='big'))
                # reserved always 0xfffe
                coded_f.append(b'\xFF\xFE')
                # options
                if 'options' in f:
                    coded_f.append((f['options']).to_bytes(1, byteorder='big'))
                else:
                    # options to 0, ie default
                    coded_f.append(b'\x00')
                # AT command bytes
                coded_f.append((ord(f['AT'][0])).to_bytes(1, byteorder='big'))
                coded_f.append((ord(f['AT'][1])).to_bytes(1, byteorder='big'))
                # parameter value (only if it is a set, not a get)
                if AT_value_bytes != None:
                    # only code AT SM (Cycle Sleep), ie 1 byte value
                    coded_f.append(AT_value_bytes)
                # checksum
                # makes a byte, and exclude first 3 that aren't counted in checksum
                cs = self.checksum(b''.join(coded_f)[3:])
                coded_f.append((cs).to_bytes(1, byteorder='big'))
                return b''.join(coded_f)
            else:
                raise Exception('Frame type not supported: ' + str(hex(f['type'])))
        except Exception as e:
            raise e
            print('XbeeFrameEncoder failed encode: ', e)
            return None


# Debug test
if __name__ == "__main__":
    f_AT_bad_type = {
        'type': 0x1234567 # wrong type
    }
    f_AT_bad_T = {
        'type': XbeeFrameDecoder.API_REMOTE_AT_REQUEST,
        'AT': "prout" # wrong AT cmd
    }
    f_AT_EC = {
        'type': XbeeFrameDecoder.API_REMOTE_AT_REQUEST,
        'frame_id': 0xaa,
        'dest_id': int.from_bytes(bytes.fromhex('112233445566'), "big"),
        'AT': XbeeFrameDecoder.AT_EC_CCA_FAILURE
    }
    
    encoder = XbeeFrameEncoder()
    encoder.encode(f_AT_bad_type)
    encoder.encode(f_AT_bad_T)
    f = encoder.encode(f_AT_EC)
    print('AT EC', f.hex())

