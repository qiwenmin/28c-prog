import argh
import serial
import sys

EMULATE = True

if EMULATE:
    import ptyprocess

CLI_VERSION = '0.0.1'

SERIAL_PORT = "/dev/cu.usbmodem14101"
BAUDRATE = 38400
EMULATOR_PATH = '../../firmware/emulator/emulator'

class ptyproc:
    def __enter__(self):
        self._p = ptyprocess.PtyProcess.spawn([EMULATOR_PATH])
        return self._p

    def __exit__(self, exc_type, exc_val, exc_tb):
        self._p.close()


def open_serial():
    return ptyproc() if EMULATE else serial.Serial(SERIAL_PORT, BAUDRATE, timeout=0.1)


def expect(ser, str):
    buf = ""
    while True:
        s = ser.read(1024)
        if s:
            s = s.replace(b'\r', b'')
            buf = buf + s.decode()

        if buf[-2:] == str:
            break

    return buf

def wait_prompt(ser):
    return expect(ser, '> ')

def exec(ser, cmd):
    cmd_bytes = (cmd + '\n').encode()
    ser.write(cmd_bytes)
    buf = wait_prompt(ser)
    ret = buf[len(cmd_bytes):-3]
    return ret

def version():
    "Print the cli version and the firmware version."
    firmware_version = 'Unknown'

    try:
        with open_serial() as ser:
            wait_prompt(ser)
            firmware_version = exec(ser, 'v')
    except:
        print(sys.exc_info()[0])

    print(f"CLI Version: {CLI_VERSION}\nFirmware {firmware_version}")


def demo():
    "Run the demo."
    with open_serial() as ser:

        wait_prompt(ser)
        print(exec(ser, 'v'))
        print(exec(ser, 'h'))
        print(exec(ser, ''))


parser = argh.ArghParser()
parser.add_commands([demo, version])

if __name__ == '__main__':
    parser.dispatch()
