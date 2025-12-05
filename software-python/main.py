import serial
import argparse
import re
import json

from models import Config

TIMEOUT = 1
BAUD = 115200

def do_regex(line: str):
    m = re.search(r"\[([0-9A-F:]{17}) .*?\] RSSI:\s*(-?\d+)", line)
    if m:
        mac = m.group(1)
        rssi = int(m.group(2))
        print(f"Got mac and RSSI: {mac}, {rssi}")


def main(port: str, config: dict):
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
        help="Serial port name"
    )
    parser.add_argument(
        "--config-file", type=str, default="config.json",
        help="Configuration file path"
    )
    args = parser.parse_args()

    with open(args.config_file) as f:
        raw_cfg = json.load(f)
        if (raw_cfg is not None):
            cfg = Config(**raw_cfg)
            main(args.port, cfg)
        else:
            raise ValueError("Cannot get json configuration")
