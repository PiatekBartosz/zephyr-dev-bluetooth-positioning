import sys
import json
import re
import threading
import serial
import argparse

from PyQt6.QtWidgets import QApplication, QWidget
from PyQt6.QtGui import QPainter, QPen, QColor
from PyQt6.QtCore import Qt, QTimer

from models import Config

TIMEOUT = 1
BAUD = 115200
REGEX = r"\[([0-9A-F:]{17}) .*?\] RSSI:\s*(-?\d+)"


class BeaconVisualizer(QWidget):
    def __init__(self, cfg: Config):
        super().__init__()
        self.cfg = cfg
        self.beacons = cfg.beacons
        self.rssi_values = {mac: None for mac in self.beacons.keys()}
        self.room = cfg.room

        self.setWindowTitle("Beacon RSSI Visualizer")
        self.setFixedSize(cfg.canvas.width, cfg.canvas.height)

        self.timer = QTimer()
        self.timer.timeout.connect(self.update)
        self.timer.start(200)

    def update_rssi(self, mac: str, rssi: int):
        if mac in self.rssi_values:
            self.rssi_values[mac] = rssi

    def paintEvent(self, event):
        painter = QPainter(self)

        # Draw room rectangle
        painter.setPen(QPen(Qt.GlobalColor.blue, 3))
        painter.setBrush(Qt.GlobalColor.transparent)
        painter.drawRect(self.room.x, self.room.y, self.room.width, self.room.height)

        # Draw each beacon
        for mac, pos in self.beacons.items():
            x, y = pos.x, pos.y

            # Draw beacon marker
            painter.setPen(QPen(Qt.GlobalColor.black, 2))
            painter.setBrush(QColor(0, 0, 0))
            painter.drawEllipse(x - 5, y - 5, 10, 10)

            # Draw MAC label
            painter.drawText(x - 30, y - 10, mac)

            # Draw RSSI circle
            rssi = self.rssi_values.get(mac)
            if rssi is not None:
                radius = max(10, min(200, abs(rssi) * 2))
                painter.setPen(QPen(QColor(255, 0, 0), 2))
                painter.setBrush(Qt.GlobalColor.transparent)
                painter.drawEllipse(x - radius, y - radius, radius * 2, radius * 2)


def serial_reader(port: str, visualizer: BeaconVisualizer):
    ser = serial.Serial(port, BAUD, timeout=TIMEOUT)

    while True:
        data = ser.readline()
        if not data:
            continue

        try:
            line = data.decode("utf-8").strip()
        except UnicodeDecodeError:
            continue

        print("RAW:", line)

        m = re.search(REGEX, line)
        if not m:
            continue

        mac = m.group(1)
        rssi = int(m.group(2))

        print(f"Got mac and RSSI: {mac}, {rssi}")
        visualizer.update_rssi(mac, rssi)


def main(port: str, cfg: Config):
    app = QApplication([])

    visualizer = BeaconVisualizer(cfg)

    # Start serial thread
    thread = threading.Thread(target=serial_reader, args=(port, visualizer))
    thread.start()

    visualizer.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--port", type=str, default="/dev/ttyACM0")
    parser.add_argument("--config-file", type=str, default="config.json")
    args = parser.parse_args()

    with open(args.config_file) as f:
        raw_cfg = json.load(f)
        if raw_cfg is None:
            raise ValueError("Cannot load JSON configuration")

        cfg = Config(**raw_cfg)
        main(args.port, cfg)
