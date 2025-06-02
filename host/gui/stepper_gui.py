import sys, time, queue, threading, serial, serial.tools.list_ports
from PyQt6.QtWidgets import (
    QApplication, QWidget, QLabel, QPushButton, QComboBox, QSpinBox,
    QTextEdit, QRadioButton, QLineEdit, QSizePolicy, QGridLayout, QHBoxLayout,
    QVBoxLayout, QFrame
)
from PyQt6.QtCore import QTimer

# ────────── worker thread ──────────
class SerialWorker(threading.Thread):
    def __init__(self, port: str, baud: int = 115200):
        super().__init__(daemon=True)
        self._run = True
        self._ser = None
        self.q_out, self.q_in = queue.Queue(), queue.Queue()
        try:
            self._ser = serial.Serial(port, baud, timeout=0.1)
        except serial.SerialException as e:
            self.q_in.put(f"Error: {e}")
            self._run = False

    def run(self):
        while self._run and self._ser:
            try:
                self._ser.write((self.q_out.get_nowait() + '\n').encode())
            except queue.Empty:
                pass
            line = self._ser.readline().decode(errors="ignore").strip()
            if line:
                self.q_in.put(line)
            time.sleep(0.01)

    def send(self, cmd: str): self.q_out.put(cmd)
    def close(self):
        self._run = False
        if self._ser:
            self._ser.close()

# ────────── GUI ──────────
class StepperGUI(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("DRV8825 Continuous Control")
        self.setMinimumWidth(480)

        root = QVBoxLayout(self)
        root.setContentsMargins(14, 14, 14, 14)
        root.setSpacing(10)

        # ── 1. control grid ──
        ctl = QGridLayout()
        ctl.setHorizontalSpacing(8)
        ctl.setVerticalSpacing(6)

        ctl.addWidget(QLabel("Port:"), 0, 0)
        self.portBox = QComboBox(); self.refresh_ports()
        ctl.addWidget(self.portBox,     0, 1)
        self.btnConn = QPushButton("Connect")
        ctl.addWidget(self.btnConn,     0, 2)

        ctl.addWidget(QLabel("RPM:"),   1, 0)
        self.spinRPM = QSpinBox(); self.spinRPM.setRange(1, 1200); self.spinRPM.setValue(60)
        ctl.addWidget(self.spinRPM,     1, 1)

        dirbox = QHBoxLayout()
        self.radCW  = QRadioButton("CW");  self.radCW.setChecked(True)
        self.radCCW = QRadioButton("CCW")
        dirbox.addWidget(self.radCW); dirbox.addWidget(self.radCCW)
        ctl.addLayout(dirbox,           1, 2)

        self.btnStart = QPushButton("Start")
        self.btnStop  = QPushButton("Stop")
        self.btnQuery = QPushButton("Query")
        btnRow = QHBoxLayout(); btnRow.setSpacing(6)
        btnRow.addWidget(self.btnStart); btnRow.addWidget(self.btnStop)
        btnRow.addStretch();             btnRow.addWidget(self.btnQuery)
        ctl.addLayout(btnRow,            2, 0, 1, 3)

        ctl.addWidget(QLabel("Cmd:"),    3, 0)
        self.lineCmd = QLineEdit()
        ctl.addWidget(self.lineCmd,      3, 1)
        self.btnSend = QPushButton("Send")
        ctl.addWidget(self.btnSend,      3, 2)

        root.addLayout(ctl)

        # ── 2. status label ──
        self.lblStat = QLabel("POS 0 RPM 0 ACT 0 BUSY 0")
        self.lblStat.setFrameShape(QFrame.Shape.Panel)
        self.lblStat.setFrameShadow(QFrame.Shadow.Sunken)
        self.lblStat.setStyleSheet("padding:4px")
        root.addWidget(self.lblStat)

        # ── 3. unified log ──
        self.txtLog = QTextEdit()
        self.txtLog.setReadOnly(True)
        self.txtLog.setPlaceholderText("Commands ↔ Replies …")
        self.txtLog.setFixedHeight(260)
        root.addWidget(self.txtLog)

        # ── wiring ──
        self.worker: SerialWorker | None = None
        self.btnConn.clicked.connect(self.toggle_conn)
        self.spinRPM.valueChanged.connect(lambda v: self.send(f"V {v}"))
        self.btnStart.clicked.connect(self.start_run)
        self.btnStop.clicked.connect(lambda: self.send("S"))
        self.radCW.toggled.connect(self.dir_change)
        self.btnQuery.clicked.connect(lambda: self.send("?"))
        self.btnSend.clicked.connect(self.raw_send)
        self.lineCmd.returnPressed.connect(self.raw_send)

        self.timer = QTimer(self); self.timer.timeout.connect(self.poll); self.timer.start(100)

        self.setStyleSheet("""
            QWidget { font-size:10pt; }
            QSpinBox, QLineEdit, QTextEdit, QComboBox { background:#202020; color:#e0e0e0; }
            QPushButton { padding:4px 12px; }
        """)

    # ── helpers ──
    def refresh_ports(self):
        self.portBox.clear()
        for p in serial.tools.list_ports.comports():
            self.portBox.addItem(p.device)

    def toggle_conn(self):
        if self.worker:
            self.worker.close(); self.worker = None
            self.btnConn.setText("Connect")
            self.txtLog.append("Disconnected")
            return
        port = self.portBox.currentText()
        if not port: self.txtLog.append("No port selected"); return
        self.worker = SerialWorker(port); self.worker.start()
        self.btnConn.setText("Disconnect")
        self.txtLog.append(f"Connected {port}")
        QTimer.singleShot(300, lambda: self.send("?"))

    def send(self, cmd: str):
        if not self.worker:
            self.txtLog.append("!! not connected"); return
        self.worker.send(cmd)
        self.txtLog.append(f"> {cmd}")

    # ── slots ──
    def start_run(self): self.send("E"); self.dir_change()
    def dir_change(self):
        if self.worker: self.send(f"R {1 if self.radCW.isChecked() else 0}")
    def raw_send(self):
        txt = self.lineCmd.text().strip()
        if txt: self.send(txt)
        self.lineCmd.clear()

    # ── poll incoming ──
    def poll(self):
        if not self.worker: return
        try:
            while True:
                line = self.worker.q_in.get_nowait()
                self.txtLog.append(line)
                if line.startswith("POS"):
                    self.lblStat.setText(line)
        except queue.Empty:
            pass

    def closeEvent(self, e):
        if self.worker: self.worker.close()
        super().closeEvent(e)

# ────────── main ──────────
if __name__ == "__main__":
    app = QApplication(sys.argv)
    gui = StepperGUI(); gui.show()
    sys.exit(app.exec())
