
import subprocess, time

COLLECTOR_IP = "192.168.1.66"
COLLECTOR_PORT = 8087

if __name__ == "__main__":
    # TODO make a help
    # TODO get mac from arg
    
    # socat pty,link=/dev/garden0,raw,echo=0 tcp:192.168.1.66:8087
    #subprocess.Popen(["socat","pty,link=/dev/garden0,raw,echo=0","tcp:192.168.1.66:8087"], shell=True)
    p = subprocess.Popen(
        [
            "socat","pty,link=/tmp/garden0,raw,echo=0",
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

    # kill socat
    p.kill()
    
