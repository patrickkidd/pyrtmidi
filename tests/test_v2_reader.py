import time
import collector
import rtmidi

PORT_NAME = 'PK Blah'

def GoRandom():

    bin = collector.CollectorBin()
    print('START...')
    bin.start()

    try:
        time.sleep(10000)
    except KeyboardInterrupt:
        pass

    bin.stop()


if __name__ == '__main__':    
    GoRandom()
