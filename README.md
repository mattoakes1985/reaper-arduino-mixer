# 🎚️ REAPER Arduino In-Ear Mixer

This project turns an Arduino Nano, OLED screen, and two rotary encoders into a **custom REAPER DAW controller** for personal in-ear monitor mixing.

🎛️ **Features**
- Control REAPER track volume and pan with encoder 1 (toggle mode with button press)
- Jog through tracks using encoder 2
- OLED shows:
  - Track name
  - Volume in dB
  - Pan position (L/C/R)
  - Current control mode `<VOL>` or `<PAN>`

---

## 🛠 Hardware Requirements
- Arduino Nano (or compatible)
- 128x64 I2C OLED display (SSD1306)
- 2x Rotary encoders with push buttons
- USB cable (for Arduino)
- REAPER DAW with Python support enabled

---

## ⚡ Wiring Diagram
| Component         | Arduino Pin |
|--------------------|-------------|
| **OLED SDA**       | A4          |
| **OLED SCL**       | A5          |
| **Encoder1 CLK**   | D3          |
| **Encoder1 DT**    | D2          |
| **Encoder1 SW**    | D4          |
| **Encoder2 CLK**   | D5          |
| **Encoder2 DT**    | D6          |
| **Encoder2 SW**    | D7          |

📷 See the `wiring/diagram.png` for a visual.

---

## 🖥 Software Setup

### 1️⃣ Arduino
- Flash `arduino/reaper_mixer.ino` to your Arduino Nano.  
- Uses:  
  - `Adafruit_GFX` and `Adafruit_SSD1306` libraries (install via Arduino Library Manager)  

### 2️⃣ REAPER
- Copy `python/reaper_controller.py` into your REAPER scripts folder.  
- Enable Python in REAPER preferences.  
- Load the script via REAPER Actions list.

---

## 📂 Repo Structure
reaper-arduino-mixer/
│
├── arduino/
│ └── reaper_mixer.ino
│
├── python/
│ └── reaper_controller.py
│
├── wiring/
│ └── diagram.png
│
├── README.md
└── LICENSE


---

## 📜 License
MIT License. Free to use, modify, and share.

---

## 💡 Credits
Inspired by the need for simple, tactile control in live in-ear monitoring with REAPER.
