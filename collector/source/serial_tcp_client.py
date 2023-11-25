#!/usr/bin/python3

from tcp_listener import *
import subprocess, time, sys, os

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
    # wait for popen to startup
    time.sleep(1.0)
    
    # Open pty to negotiate the endpoint to hold
    pty = os.open(PTY_PATH, os.O_RDWR)
    os.write(pty, mac.to_bytes(8, byteorder='big'))
    # wait for targeted endpoint to be up and held (no cycle sleep)
    print('Waiting for target to become available...')
    t0 = time.time()
    t_last = t0
    while True:
        fb = os.read(pty, 1)
        fb = int.from_bytes(fb, byteorder='big')
        if fb == TcpListener.STATUS_READY:
            break;
        elif fb == TcpListener.STATUS_WAIT:
            print('.', end='', flush=True)
            t = time.time()
            if t - t_last >= 10:
                print('\nBeen waiting for {:.0f} sec'.format(t - t0), end='', flush=True)
                t_last = t
            continue;
        elif fb == TcpListener.STATUS_ERROR:
            print('Received STATUS_ERROR from TcpListener, exit...')
            sys.exit(51)
    os.close(pty)

    # keep alive to keep alive subprocess
    print("Serial ready at", PTY_PATH)
    print("press Ctrl+c to exit and remove serial")
    while True:
        try:
            time.sleep(1)
        except KeyboardInterrupt:
            print("Exiting...")
            break

    # kill socat
    p.kill()
    
