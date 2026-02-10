# ⚡ NEURAL INTERFACE: SLUSHI3.AI // ESP32 CORE
> ACCESSING SYSTEM DATA... [OK]
> INITIALIZING HACKER_THEME... [OK]

Welcome, User. You have accessed the repository for the Slushi3 AI Terminal. 
This is an ESP32-powered neural gateway connected directly to the Gemini 2.5 Flash API.

---

### 🟢 CORE ARCHITECTURE (HARDWARE)
The system is built on low-level hardware designed for portable neural link operations.

* **Processor:** ESP32 Quantum Core
* **Visual Output:** SSD1306 OLED Display (128x64)
* **Power Cell:** 3.7V 1000mAh Li-Po Battery
* **Energy Management:** TP4056 Charging Module
* **Storage:** SD Card Module (Interface Logging)

---

### 🛠️ HARDWARE SCHEMATICS (PINOUT)
To establish a physical link, ensure your wiring matches the following encrypted protocols:

| COMPONENT | PIN (ESP32) | PIN (DEVICE) |
| :--- | :--- | :--- |
| **OLED SDA** | GPIO 21 | DATA |
| **OLED SCL** | GPIO 22 | CLOCK |
| **BATTERY (+)** | VIN | TP4056 OUT+ |
| **BATTERY (-)** | GND | TP4056 OUT- |

---

### 🎨 VISUAL INTERFACE THEMES
The system supports three distinct UI layers for environment adaptation:

1.  **MATRIX TERMINAL (FREE):** The classic green-on-black hacker aesthetic. Default protocol.
2.  **NEON EMERALD (PREMIUM):** High-contrast modern green gradient interface.
3.  **VIOLET VORTEX (PREMIUM):** Sleek purple/blue deep-space aesthetic.

> [!IMPORTANT]  
> **The Matrix Terminal theme is included in this build.**
> To unlock the **Neon Emerald** or **Violet Vortex** visual protocols, you must request an authorization key.
> **CONTACT:** [Insert Your Email Here]

---

### 🚀 INITIALIZATION
1. Update `WIFI_SSID` and `API_KEY` in the `SYSTEM CONFIG` menu.
2. Compile and Flash using the Arduino IDE.
3. Open Serial Monitor at `115200` baud.
4. Access the web terminal via the local IP address displayed on the OLED.

**// END OF LINE.**
