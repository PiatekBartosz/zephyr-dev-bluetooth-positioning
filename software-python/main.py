import serial
import argparse
import re

TIMEOUT = 1
BAUD = 115200

def do_regex(line: str):
    m = re.search(r"\[([0-9A-F:]{17}) .*?\] RSSI:\s*(-?\d+)", line)
    if m:
        mac = m.group(1)
        rssi = int(m.group(2))
        print(f"Got mac and RSSI: {mac}, {rssi}")

def main(port: str):
    ser = serial.Serial(port, BAUD, timeout=TIMEOUT)

    while True:
        data = ser.readline()
        if data:
            try:
                decoded_data = data.decode("utf-8").strip()
                print(f"RAW: {decoded_data}")
                do_regex(decoded_data)
            except UnicodeDecodeError:
                print("Cannot decode RAW:", data.hex())

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--port", type=str, default="/dev/ttyACM0",
        help="serial port name"
    )
    args = parser.parse_args()
    main(args.port)
