import serial
import ptyprocess

EMULATE = True

SERIAL_PORT = "/dev/cu.usbmodem14101"
BAUDRATE = 38400

class ptyproc:
    def __enter__(self):
        return ptyprocess.PtyProcess.spawn(['../../firmware/emulator/emulator'])

    def __exit__(self, exc_type, exc_val, exc_tb):
        pass


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


def prog():

    with open_serial() as ser:

        wait_prompt(ser)
        print(exec(ser, 'v'))
        print(exec(ser, 'h'))
        print(exec(ser, ''))


if __name__ == '__main__':
    prog()
