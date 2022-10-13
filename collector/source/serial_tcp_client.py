#!/usr/bin/python3

import subprocess, time, sys

COLLECTOR_IP = "192.168.1.66"
COLLECTOR_PORT = 8087

PTY_PATH = "/tmp/garden0"

def help():
    print(
'''
Connect to collector TCP Listener, which exposes the serial port
A local serial port, TCP connected, is created at {1}
This serial can be used by Digi's XCTU

Meanwhile, it also...
Order the collector to hold an endpoint in config mode
While in config mode: 
  - an endpoint cycle sleep is disabled
  - collector ignores that enpoint
Upon disconnection, endpoint is put back in cycle sleep mode

NOTE : depending on previously setup cycle sleep period,
       it mays take a while before getting a hold on target endpoint

Usage:
 {0} --help   print that help
 {0} mac      mac as decimal or hexadecimal
'''.format(sys.argv[0], PTY_PATH))


if __name__ == "__main__":
    if len(sys.argv) != 2:
        help()
        sys.exit(51)
    
    # here we do have 2 args
    if sys.argv[1] == "--help":
        help()
        sys.exit(51)
    
    # here 2nd arg is considered to be the MAC
    mac = 0
    try:
        mac = int(sys.argv[1], 0)
    except Exception as e:
        print('This is not a valid MAC: {}'.format(sys.argv[1]))
        help()
        sys.exit(51)
    
    print('Connecting to {}:{} and holding MAC {:X}'.format(COLLECTOR_IP, COLLECTOR_IP, mac))
    
    #sys.exit(51)
    
    # socat pty,link=/dev/garden0,raw,echo=0 tcp:192.168.1.66:8087
    #subprocess.Popen(["socat","pty,link=/dev/garden0,raw,echo=0","tcp:192.168.1.66:8087"], shell=True)
    p = subprocess.Popen(
        [
            "socat pty,link=" + PTY_PATH + ",raw,echo=0 " +\
            "tcp:" + COLLECTOR_IP + ":" + str(COLLECTOR_PORT)
        ],
        #["ls","-al"], 
        shell=True, 
        stdout=subprocess.PIPE
    )
    
    # TODO open pty
    # TODO Send 8 bytes MAC
    # TODO close pty

    # keep alive to keep alive subprocess
    print("Serial ready")
    print("press Ctrl+c to exit and remove serial")
    while True:
        try:
            time.sleep(1)
        except KeyboardInterrupt:
            print("Exiting...")
            break

    # kill socat
    p.kill()
    
