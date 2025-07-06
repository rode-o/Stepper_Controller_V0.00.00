
# Xiao Stepper Firmware

Minimal non‑blocking firmware for a single DRV8825‑driven stepper.

* **Board:** Seeed XIAO RP2040  
* **Driver:** DRV8825 StepStick  
* **Library:** Rob Tillaart/DRV8825 (install via Library Manager)

## Build

1. Install the **Seeed RP2040** board package in the Arduino IDE.  
2. Install the **DRV8825** library.  
3. Open `xiao_stepper.ino`, adjust pins/constants in `config.h`.  
4. Compile & upload.

## Serial protocol

| Command | Example | Meaning |
|---------|---------|---------|
| `V <rpm>` | `V 120` | Set target speed in RPM |
| `G <steps>` | `G 6400` | Move relative micro‑steps |
| `A <deg>` | `A -90` | Rotate degrees (simple linear map) |
| `S` | `S` | Stop/disable motor |
| `?` | `?` | Query status |

Each command is ASCII, terminated by `\n`.
