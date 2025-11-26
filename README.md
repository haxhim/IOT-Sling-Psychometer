<!-- HERO -->
<p align="center">
  <img src="https://img.shields.io/badge/IOT%20Sling%20Psychrometer-ESP32%20Project-blue?style=for-the-badge&logo=esphome&logoColor=white">
</p>

<h1 align="center">🌡️ IOT Sling Psychrometer</h1>

<p align="center">
  A modern IoT-enabled <b>sling psychrometer</b> using the <b>LilyGO T-Display ESP32</b> to measure 
  <b>dry bulb</b>, <b>wet bulb</b>, calculate <b>relative humidity</b>, and upload readings to the cloud in real-time.
</p>

<p align="center">
  <img src="https://img.shields.io/badge/ESP32-T--Display-orange?style=for-the-badge">
  <img src="https://img.shields.io/badge/IoT-Cloud%20Enabled-green?style=for-the-badge&logo=cloudflare">
  <img src="https://img.shields.io/badge/Data%20Logging-n8n-blueviolet?style=for-the-badge&logo=apache">
  <img src="https://img.shields.io/badge/License-MIT-red?style=for-the-badge">
</p>

---

## 📸 Device Showcase

<details>
<summary><b>📷 Click to view device photos</b></summary><br>

### **Prototype**
<p align="center">
<img src="https://raw.githubusercontent.com/haxhim/IOT-Sling-Psychometer/refs/heads/main/assets/prototype1.jpg" width="460">
</p>

### **Base Measurement Setup**
<p align="center">
<img src="https://raw.githubusercontent.com/haxhim/IOT-Sling-Psychometer/refs/heads/main/assets/baase1.jpg" width="460">
</p>

</details>

---

## 🚀 Features

- 🌡️ Real-time dry & wet bulb temperature  
- 🔄 Automatic relative humidity calculation  
- 🖥️ Clear LCD output on T-Display  
- 📡 Sends JSON payload to n8n/webhook server  
- 📊 Ideal for environmental experiments  
- 🔋 Portable & lightweight  
- 🔧 Supports 3D-printed housing  

---

## 🛠️ Hardware Requirements

<details>
<summary><b>🔍 Click to expand</b></summary><br>

| Component | Description |
|----------|-------------|
| **ESP32 LilyGO T-Display** | Includes TFT LCD |
| **2x Thermistors** | Wet bulb & dry bulb measurement |
| **10k Resistors** | Voltage divider |
| **Cotton wick** | For wet bulb |
| **Water reservoir** | For evaporation |
| **Battery pack (optional)** | Portable mode |
| **USB Type-C cable** | Programming & power |

</details>

---

## 🔌 Wiring Diagram

<details>
<summary><b>🧩 Click to view wiring</b></summary><br>

ESP32 T-Display Pins
────────────────────────────

Wet Bulb Thermistor
• One side → 3.3V
• Other side → GPIO 36
• 10k resistor → GPIO 36 → GND

Dry Bulb Thermistor
• One side → 3.3V
• Other side → GPIO 39
• 10k resistor → GPIO 39 → GND

Battery Voltage Monitor
• Battery → Voltage Divider → GPIO 34

yaml
Copy code

</details>

---

## 🧠 System Architecture

<details>
<summary><b>📊 Click to expand system flow</b></summary><br>

┌─────────────────────┐
│ Wet Bulb Sensor │
└──────────┬──────────┘
│
┌──────────▼──────────┐
│ ESP32 T-Display │─── WiFi ───► n8n / API Server
└──────────▲──────────┘
│
┌──────────┴──────────┐
│ Dry Bulb Sensor │
└──────────────────────┘

yaml
Copy code

</details>

---

## 📦 Installation

```bash
git clone https://github.com/haxhim/IOT-Sling-Psychrometer
cd IOT-Sling-Psychrometer
Open the .ino file in Arduino IDE or PlatformIO.
```

⚙️ Configuration
Edit your WiFi + Webhook in the file:

```cpp
Copy code
const char* ssid = "your_wifi";
const char* password = "your_password";
const char* webhook_url = "https://your-n8n-webhook";
```
🧪 Usage

<details> <summary><b>▶️ Click to see how it works</b></summary><br>
On Boot:
Connects to WiFi

Reads wet bulb temperature

Reads dry bulb temperature

Calculates humidity

Displays results on TFT LCD

Sends JSON payload to webhook

Example JSON
```json
Copy code
{
  "device_id": "iot-psychrometer",
  "dry_temp": 30.12,
  "wet_temp": 25.87,
  "battery": 3.88,
  "timestamp": "2025-01-01T08:20:33+08:00"
}
```

Example LCD Output

```yaml
Copy code
 IOT PSYCHROMETER
 ─────────────────
 WET : 25.9 °C
 DRY : 30.1 °C
 BAT : 3.88 V
```

</details>
📁 Project Structure

```css
Copy code
/
├── espcode.cpp
├── assets/
│   ├── prototype1.jpg
│   └── baase1.jpg
└── README.md
```

##🤝 Contributing
    Pull requests are welcome!
    For larger changes, open an issue first.

##📄 License
    This project is licensed under MIT.

<p align="center"> Made with ❤️ by <b>Hashim</b> </p> 
