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
