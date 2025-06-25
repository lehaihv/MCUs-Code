import sys
import serial
import serial.tools.list_ports
from PyQt6.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QHBoxLayout, QLabel,
    QComboBox, QTextEdit, QLineEdit, QPushButton
)
from PyQt6.QtCore import QThread, pyqtSignal

class SerialReader(QThread):
    data_received = pyqtSignal(str)

    def __init__(self, ser):
        super().__init__()
        self.ser = ser
        self.running = True

    def run(self):
        while self.running:
            if self.ser and self.ser.is_open and self.ser.in_waiting:
                data = self.ser.readline().decode(errors='ignore')
                self.data_received.emit(data)

    def stop(self):
        self.running = False
        self.ser.close()

class SerialGUI(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("PySerial GUI")
        self.resize(1200, 800)  # Set window size to 1200x800
        self.serial = None
        self.reader_thread = None

        layout = QVBoxLayout()

        # Port selection
        port_layout = QHBoxLayout()
        port_layout.addWidget(QLabel("Port:"))
        self.port_combo = QComboBox()
        self.refresh_ports()
        port_layout.addWidget(self.port_combo)

        # Scan button
        self.scan_btn = QPushButton("Scan")
        self.scan_btn.clicked.connect(self.refresh_ports)
        port_layout.addWidget(self.scan_btn)

        # Baudrate selection
        port_layout.addWidget(QLabel("Baudrate:"))
        self.baud_combo = QComboBox()
        self.baud_combo.addItems(["9600", "19200", "38400", "57600", "115200"])
        self.baud_combo.setCurrentText("115200")  # Set default baudrate to 115200
        port_layout.addWidget(self.baud_combo)

        # Mode selection (optional, e.g., 8N1)
        port_layout.addWidget(QLabel("Mode:"))
        self.mode_combo = QComboBox()
        self.mode_combo.addItems(["8N1", "7E1", "7O1"])
        port_layout.addWidget(self.mode_combo)

        layout.addLayout(port_layout)

        # Start/Stop, Clear, and Exit buttons
        btn_layout = QHBoxLayout()
        self.start_btn = QPushButton("Start")
        self.start_btn.clicked.connect(self.start_serial)
        btn_layout.addWidget(self.start_btn)

        self.clear_btn = QPushButton("Clear")
        self.clear_btn.clicked.connect(self.clear_recv_text)
        btn_layout.addWidget(self.clear_btn)

        self.exit_btn = QPushButton("Exit")
        self.exit_btn.clicked.connect(self.exit_program)
        btn_layout.addWidget(self.exit_btn)

        layout.addLayout(btn_layout)

        # Received data display
        self.recv_text = QTextEdit()
        self.recv_text.setReadOnly(True)
        layout.addWidget(self.recv_text)

        # Send box
        send_layout = QHBoxLayout()
        self.send_line = QLineEdit()
        send_layout.addWidget(self.send_line)
        self.send_btn = QPushButton("Send")
        self.send_btn.clicked.connect(self.send_data)
        send_layout.addWidget(self.send_btn)
        layout.addLayout(send_layout)

        self.setLayout(layout)

    def refresh_ports(self):
        self.port_combo.clear()
        ports = serial.tools.list_ports.comports()
        for port in ports:
            self.port_combo.addItem(port.device)

    def start_serial(self):
        if self.serial and self.serial.is_open:
            self.reader_thread.stop()
            self.reader_thread.wait()
            self.serial = None
            self.start_btn.setText("Start")
            return

        port = self.port_combo.currentText()
        baud = int(self.baud_combo.currentText())
        mode = self.mode_combo.currentText()
        bytesize = serial.EIGHTBITS if mode[0] == "8" else serial.SEVENBITS
        parity = serial.PARITY_NONE if mode[1] == "N" else (serial.PARITY_EVEN if mode[1] == "E" else serial.PARITY_ODD)
        stopbits = serial.STOPBITS_ONE

        try:
            self.serial = serial.Serial(port, baudrate=baud, bytesize=bytesize, parity=parity, stopbits=stopbits, timeout=0.1)
            self.reader_thread = SerialReader(self.serial)
            self.reader_thread.data_received.connect(self.display_data)
            self.reader_thread.start()
            self.start_btn.setText("Stop")
        except Exception as e:
            self.recv_text.append(f"Error: {e}")

    def display_data(self, data):
        self.recv_text.append(data)

    def send_data(self):
        if self.serial and self.serial.is_open:
            text = self.send_line.text()
            self.serial.write((text + "\n").encode())
            self.send_line.clear()

    def clear_recv_text(self):
        self.recv_text.clear()

    def exit_program(self):
        # Cleanup: close serial port and exit application
        if self.serial and self.serial.is_open:
            self.reader_thread.stop()
            self.reader_thread.wait()
        QApplication.quit()

    def closeEvent(self, event):
        if self.serial and self.serial.is_open:
            self.reader_thread.stop()
            self.reader_thread.wait()
        event.accept()

if __name__ == "__main__":
    app = QApplication(sys.argv)
    gui = SerialGUI()
    gui.show()
    sys.exit(app.exec())