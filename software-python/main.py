import sys
import json
import re
import threading
import serial
import argparse
import math

from PyQt6.QtWidgets import QApplication, QWidget
from PyQt6.QtGui import QPainter, QPen, QColor, QFont
from PyQt6.QtCore import Qt, QTimer

from models import Config

TIMEOUT = 1
BAUD = 115200
REGEX = r"\[([0-9A-F:]{17}) .*?\] RSSI:\s*(-?\d+)"

# --- Trilateration Math Helper ---
def get_trilateration(beacons_data):
    """
    Calculates the estimated position based on beacon locations and radii.
    beacons_data: list of tuples [(x, y, radius), (x, y, radius), ...]
    Returns: (x, y) or None if insufficient data
    """
    n = len(beacons_data)
    if n < 1:
        return None

    # 1. Start with the centroid (average position) as the initial guess
    guess_x = sum(b[0] for b in beacons_data) / n
    guess_y = sum(b[1] for b in beacons_data) / n

    # If we only have 1 beacon, we can't trilaterate, return the beacon pos (or edge of radius)
    # But for visualization, let's just return the beacon center to be safe.
    if n == 1:
        return (beacons_data[0][0], beacons_data[0][1])

    # 2. simple Gradient Descent to find the point where circles intersect
    # We want to minimize error: sum((distance_to_beacon - radius)^2)
    learning_rate = 0.05
    iterations = 50

    for _ in range(iterations):
        grad_x = 0
        grad_y = 0
        
        for bx, by, br in beacons_data:
            # Calculate distance from guess to beacon
            dx = guess_x - bx
            dy = guess_y - by
            dist = math.sqrt(dx*dx + dy*dy)
            
            if dist == 0: 
                continue # Avoid division by zero

            # The error is how far off the distance is from the radius
            # We want dist to equal br.
            error = dist - br
            
            # Derivative of error^2
            grad_x += (dx / dist) * error
            grad_y += (dy / dist) * error

        # Update guess
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
        
        # Store the calculated position
        self.estimated_pos = None

        self.setWindowTitle("Beacon RSSI Visualizer + Trilateration")
        self.setFixedSize(cfg.canvas.width, cfg.canvas.height)

        self.timer = QTimer()
        self.timer.timeout.connect(self.update_state) # Renamed to avoid confusion with repaint
        self.timer.start(200)

    def update_rssi(self, mac: str, rssi: int):
        if mac in self.rssi_values:
            self.rssi_values[mac] = rssi

    def get_radius_pixels(self, rssi):
        """
        Convert RSSI to pixels using the same logic used for drawing circles.
        """
        if rssi is None:
            return None
        # Using your existing visualization logic for consistency
        return max(10, min(200, abs(rssi) * 2))

    def update_state(self):
        # Prepare data for trilateration
        active_circles = []
        
        for mac, pos in self.beacons.items():
            rssi = self.rssi_values.get(mac)
            if rssi is not None:
                r = self.get_radius_pixels(rssi)
                active_circles.append((pos.x, pos.y, r))
        
        # Calculate intersection
        self.estimated_pos = get_trilateration(active_circles)
        
        # Trigger repaint
        self.update()

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)

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
            painter.setPen(QPen(Qt.GlobalColor.black, 1))
            painter.drawText(x - 30, y - 10, mac)

            # Draw RSSI circle
            rssi = self.rssi_values.get(mac)
            if rssi is not None:
                radius = self.get_radius_pixels(rssi)
                
                # Draw the radius circle
                painter.setPen(QPen(QColor(255, 0, 0, 150), 2)) # Red, slightly transparent
                painter.setBrush(Qt.GlobalColor.transparent)
                painter.drawEllipse(int(x - radius), int(y - radius), int(radius * 2), int(radius * 2))

        # --- Visualize Trilateration Result ---
        if self.estimated_pos:
            ex, ey = self.estimated_pos
            
            # Draw Crosshair
            painter.setPen(QPen(Qt.GlobalColor.darkGreen, 3))
            painter.drawLine(int(ex - 10), int(ey), int(ex + 10), int(ey))
            painter.drawLine(int(ex), int(ey - 10), int(ex), int(ey + 10))
            
            # Draw Center Dot
            painter.setBrush(Qt.GlobalColor.green)
            painter.drawEllipse(int(ex - 4), int(ey - 4), 8, 8)
            
            # Draw Coordinates text
            painter.setPen(QPen(Qt.GlobalColor.darkGreen, 1))
            painter.setFont(QFont("Arial", 10, QFont.Weight.Bold))
            painter.drawText(int(ex + 15), int(ey + 5), f"Target\n({int(ex)}, {int(ey)})")


def serial_reader(port: str, visualizer: BeaconVisualizer):
    try:
        ser = serial.Serial(port, BAUD, timeout=TIMEOUT)
    except serial.SerialException as e:
        print(f"Error opening serial port {port}: {e}")
        return

    print(f"Listening on {port}...")
    
    while True:
        try:
            data = ser.readline()
            if not data:
                continue

            try:
                line = data.decode("utf-8").strip()
            except UnicodeDecodeError:
                continue

            # print("RAW:", line) # Optional: comment out to reduce spam

            m = re.search(REGEX, line)
            if not m:
                continue

            mac = m.group(1)
            rssi = int(m.group(2))

            # print(f"Got mac and RSSI: {mac}, {rssi}")
            visualizer.update_rssi(mac, rssi)
            
        except Exception as e:
            print(f"Serial Error: {e}")
            break


def main(port: str, cfg: Config):
    app = QApplication([])

    visualizer = BeaconVisualizer(cfg)

    # Start serial thread
    thread = threading.Thread(target=serial_reader, args=(port, visualizer), daemon=True)
    thread.start()

    visualizer.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--port", type=str, default="/dev/ttyACM0")
    parser.add_argument("--config-file", type=str, default="config.json")
    args = parser.parse_args()

    try:
        with open(args.config_file) as f:
            raw_cfg = json.load(f)
            if raw_cfg is None:
                raise ValueError("Cannot load JSON configuration")
            
            # Ensure proper structure for Config if needed, assuming Pydantic or similar
            cfg = Config(**raw_cfg)
            main(args.port, cfg)
    except FileNotFoundError:
        print(f"Config file not found: {args.config_file}")
    except Exception as e:
        print(f"Error: {e}")