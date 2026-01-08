import sys
import json
import re
import threading
import serial
import argparse
import math

from PyQt6.QtWidgets import QApplication, QWidget
from PyQt6.QtGui import QPainter, QPen, QColor
from PyQt6.QtCore import Qt, QTimer

# --- Constants ---
RSSI_1M = -60
N_PATH_LOSS = 2.5
SCALE = 100  # 100 pixels = 1 meter

HISTORY_SIZE = 20
TIMEOUT = 1
BAUD = 115200
REGEX = r"\[([0-9A-F:]{17}) .*?\] RSSI:\s*(-?\d+)"

# --- Configuration Model ---


class Config:
    def __init__(self, beacons, canvas, room):
        self.beacons = {mac: type('Pos', (object,), pos) for mac, pos in beacons.items()}
        self.canvas = type('Canvas', (object,), canvas)
        self.room = type('Room', (object,), room)


def get_trilateration(beacons_data):
    n = len(beacons_data)
    if n < 1:
        return None

    guess_x = sum(b[0] for b in beacons_data) / n
    guess_y = sum(b[1] for b in beacons_data) / n

    if n == 1:
        return (beacons_data[0][0], beacons_data[0][1])

    learning_rate = 0.05
    iterations = 100

    for _ in range(iterations):
        grad_x = 0
        grad_y = 0
        for bx, by, br in beacons_data:
            dx = guess_x - bx
            dy = guess_y - by
            dist = math.sqrt(dx * dx + dy * dy)
            if dist == 0:
                continue
            error = dist - br
            grad_x += (dx / dist) * error
            grad_y += (dy / dist) * error

        guess_x -= learning_rate * (grad_x / n)
        guess_y -= learning_rate * (grad_y / n)

    return (guess_x, guess_y)


class BeaconVisualizer(QWidget):
    def __init__(self, cfg: Config):
        super().__init__()
        self.cfg = cfg
        self.beacons = cfg.beacons
        self.rssi_values = {mac: None for mac in self.beacons.keys()}
        self.room = cfg.room

        self.raw_position_history = []
        self.estimated_pos = None

        self.setWindowTitle("Trilateration: 4.33m x 3.17m")
        self.setFixedSize(cfg.canvas.width, cfg.canvas.height)

        self.timer = QTimer()
        self.timer.timeout.connect(self.update_state)
        self.timer.start(100)

    def update_rssi(self, mac: str, rssi: int):
        if mac in self.rssi_values:
            self.rssi_values[mac] = rssi

    def get_radius_pixels(self, rssi):
        if rssi is None:
            return None
        distance_m = 10 ** ((RSSI_1M - rssi) / (10 * N_PATH_LOSS))
        return distance_m * SCALE

    def average_position(self):
        if not self.raw_position_history:
            return None

        x = sum(p[0] for p in self.raw_position_history) / len(self.raw_position_history)
        y = sum(p[1] for p in self.raw_position_history) / len(self.raw_position_history)
        return (x, y)

    def print_position_history(self):
        print("\n--- Last 20 Estimated Positions ---")
        for i, (x, y) in enumerate(self.raw_position_history):
            real_x = (x - self.room.x) / SCALE
            real_y = (y - self.room.y) / SCALE
            print(f"{i + 1:02d}: {real_x:.2f} m , {real_y:.2f} m")

    def update_state(self):
        active_circles = []
        for mac, pos in self.beacons.items():
            rssi = self.rssi_values.get(mac)
            if rssi is not None:
                r = self.get_radius_pixels(rssi)
                active_circles.append((pos.x, pos.y, r))

        raw_pos = get_trilateration(active_circles)

        if raw_pos:
            self.raw_position_history.append(raw_pos)
            if len(self.raw_position_history) > HISTORY_SIZE:
                self.raw_position_history.pop(0)

            self.estimated_pos = self.average_position()
            self.print_position_history()

        self.update()

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)

        # Room
        painter.setPen(QPen(Qt.GlobalColor.blue, 2))
        painter.setBrush(Qt.BrushStyle.NoBrush)
        painter.drawRect(self.room.x, self.room.y, self.room.width, self.room.height)

        # Beacons
        for mac, pos in self.beacons.items():
            x, y = pos.x, pos.y
            rssi = self.rssi_values.get(mac)

            if rssi is not None:
                radius = self.get_radius_pixels(rssi)
                painter.setPen(QPen(QColor(100, 100, 255, 100), 1))
                painter.setBrush(Qt.BrushStyle.NoBrush)
                painter.drawEllipse(int(x - radius), int(y - radius), int(radius * 2), int(radius * 2))

            painter.setPen(QPen(QColor("red"), 2))
            painter.setBrush(Qt.BrushStyle.NoBrush)
            painter.drawEllipse(int(x - 6), int(y - 6), 12, 12)

            painter.setPen(Qt.GlobalColor.black)
            painter.drawText(int(x + 10), int(y),
                             f"{mac[-5:]} ({rssi if rssi is not None else '?'})")

        # Estimated position (smoothed)
        if self.estimated_pos:
            ex, ey = self.estimated_pos
            painter.setPen(QPen(Qt.GlobalColor.darkGreen, 2))
            painter.setBrush(Qt.BrushStyle.NoBrush)
            painter.drawEllipse(int(ex - 8), int(ey - 8), 16, 16)

            real_x = (ex - self.room.x) / SCALE
            real_y = (ey - self.room.y) / SCALE
            painter.drawText(int(ex + 12), int(ey + 12),
                             f"POS: {real_x:.2f}m, {real_y:.2f}m")


def serial_reader(port: str, visualizer: BeaconVisualizer):
    try:
        ser = serial.Serial(port, BAUD, timeout=TIMEOUT)
    except Exception as e:
        print(f"Serial Error: {e}")
        return

    while True:
        line = ser.readline().decode("utf-8", errors="ignore").strip()
        m = re.search(REGEX, line)
        if m:
            visualizer.update_rssi(m.group(1), int(m.group(2)))


def main(port: str, cfg_data: dict):
    app = QApplication([])
    cfg = Config(**cfg_data)
    visualizer = BeaconVisualizer(cfg)
    thread = threading.Thread(target=serial_reader, args=(port, visualizer), daemon=True)
    thread.start()
    visualizer.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--port", type=str, default="/dev/ttyACM0")
    parser.add_argument("--config-file", type=str, default="config.json")
    args = parser.parse_args()

    with open(args.config_file) as f:
        main(args.port, json.load(f))
