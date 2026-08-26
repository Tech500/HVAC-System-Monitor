# HVAC System Monitor 

This repository contains a three‑node HVAC monitoring system designed as the successor to the Heating System Monitor IV project. It uses:

- LoRa Wake‑On‑Radio (WOR) for outdoor sensing
- ESP‑NOW for node‑to‑node communication
- Google Sheets for perpetual month‑to‑month and year‑to‑year logging

## Nodes

1. **WOR Sensor Node (Outdoor BME280)**  
   - SX1262 LoRa WOR RX  
   - BME280 outdoor readings  
   - ESP‑NOW TX → Receiver Node  

2. **Receiver Node (Indoor BME280 + Data collector + Google Sheets)**  
   - ESP‑NOW TX/RX (outdoor BME280 Node + Blower Node)  
   - Indoor BME280 readings  
   - Initiates WOR wake cycle  
   - Uploads combined HVAC data to Google Sheets  

3. **Blower Node (MPU6050 Vibration Sensing)**  
   - Detects blower ON/OFF via variance threshold, vibration  
   - ESP‑NOW TX → Receiver Node  
   - No electrical hookup to HVAC wiring  

## Folder Structure

```text
HVAC_System_Monitor/
│
├── Node_WOR_Sensor/
│   ├── BME280_WOR_Node.ino
│   ├── boards.h
│   ├── utilities.h
│   ├── sx1262_commands.h
│   ├── sx1262_commands.cpp
│   └── README.md
│
├── Node_Receiver/
│   ├── Receiver_Node.ino
│   ├── boards.h
│   ├── utilities.h
│   └── README.md
│
└── Node_Blower/
    ├── Blower_Node.ino
    ├── boards.h
    ├── utilities.h
    └── README.md

---

## Acknowledgements

This project was the result of a collaborative engineering effort involving both hands-on
experimentation and AI-assisted development. Each AI assistant contributed in different ways
throughout the project.

- **Claude** – Lead AI for all aspects of the project. Helped maintain technical focus,
  reviewed the evolving documentation for clarity and consistency, and provided extensive
  assistance in coding, organizing, and refining the final Markdown documentation.

- **Google Gemini** – Provided valuable guidance during the migration to **ESP32 Arduino
  Core 3.3.10**, helping identify and avoid legacy code patterns from earlier ESP32 core
  releases. Gemini also contributed to development discussions involving SX1262
  **Channel Activity Detection (CAD)**, Wake-on-Radio (WOR), and ESP32-S3 Deep Sleep
  integration.

- **ChatGPT** – Assisted throughout firmware development, debugging, architecture
  discussions, RadioLib integration, power optimization, Nordic PPK2 measurement
  interpretation, battery-life analysis, and technical review of the final documentation.

- **GitHub Copilot** – Assisted with firmware implementation, code completion, and
  day-to-day development within the programming environment.

- **Jan Gromeš** – Special thanks for the outstanding **RadioLib** library!

The successful implementation of an ESP32-S3 Deep Sleeping Wake-on-Radio node using the
SX1262 and RadioLib was made possible through this collaborative process. While the hardware
design, firmware integration, measurements, testing, and final engineering decisions were
performed by the author, the insights provided by each AI assistant significantly accelerated
development and helped improve the quality and accuracy of the final project.

> *"Project not possible without everyone's help."*

---

## License

MIT License — see `LICENSE` for details.
