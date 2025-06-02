
# Host tools

## GUI (PyQt6)

```
pip install -r host/gui/requirements.txt
python host/gui/stepper_gui.py
```

Select the serial port, set RPM, move steps or degrees, and view status responses
in the log window.

## CLI

```
python host/cli/stepper_cli.py COM5 "V 240"
```

Replace `COM5` with your actual port (e.g., `/dev/ttyACM0` on Linux/Mac).
