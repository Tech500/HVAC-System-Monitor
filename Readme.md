# HVAC System Monitor 

This repository contains a three‑node HVAC monitoring system designed as the successor to the Heating System Monitor IV project. It uses:

- LoRa Wake‑On‑Radio (WOR) for outdoor sensing
- ESP‑NOW for node‑to‑node communication
- Google Sheets for perpetual month‑to‑month and year‑to‑year logging

How blower detection works (variance-based thresholding): Blower state is detected with an MPU-6050 6-axis IMU attached to the outside of the blower enclosure. The sketch computes the statistical variance of accelerometer samples over a short window. A running blower produces mechanical vibration (high variance); a stopped blower produces almost none (low variance). Hysteresis between the ON and OFF thresholds prevents chatter at the transitions. No electrical hookup to the heating/cooling system is required, and no microphone is involved — it is immune to room noise.<br>

Logged data includes: NTP timestamp, outside temperature, inside temperature, inside humidity, thermostat setpoint, elapsed blower minutes (per cycle), daily total blower minutes, outside pressure, inside pressure, pressure difference (out − in), cycles today, coast minutes (hold time between cycles), and average cycle minutes.
Each record is written to both a local LittleFS log file and a perpetual Google Sheet (month-to-month, year-to-year).<br>

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
  
## Schematics and PCB Designs
The Schematics/ folder contains all hardware design files for the three‑node HVAC System Monitor.
All boards were designed in KiCad 10, and each PCB includes:

Full schematic (.kicad_sch)

PCB layout (.kicad_pcb)

Fabrication outputs (Gerber + drill files)

3D board previews (where applicable)

Supported Hardware Platforms
All PCBs in this project are designed around two specific ESP32‑S3 platforms:

Ebyte EoRa‑S3‑900TB  
A compact, battery‑optimized ESP32‑S3 + SX1262 LoRa module used for the WOR outdoor node and indoor receiver node.
Includes onboard battery charger, dual LDOs, SD slot, OLED header, and SX1262 radio — ideal for ultra‑low‑power sensing.

ESP32‑S3 SuperMini  
A minimal ESP32‑S3 module used for the Blower Node.
Provides a small footprint, simple power domain, and reliable ESP‑NOW performance for vibration sensing.

PCB Overview
The HVAC System Monitor requires three PCBs, corresponding to the three nodes:

ESP-NOW BME280 Node; with Wake-On-Radio (WOR) preamble (LoRa WOR + BME280)

ESP-NOW Indoor Node (BME280 + ESP‑NOW aggregator + Google Sheets uploader)

ESP-NOW Blower Node (MPU6050 vibration sensing + ESP‑NOW)

The ESP-NOW BME280 Node and ESP-NOW Inside Node use the same ESP‑NOW/BME280 PCB with GY-BME280-3.3V, 6 pin, built around the EoRa‑S3‑900TB.

The Blower Node uses a dedicated PCB built around the ESP32‑S3 SuperMini and the MPU6050 IMU.  
Note: not all ESP32-S3 SuperMini clones have the same pin layout!!!

PCB Designs Included
ESP-NOW BME280 Node and ESP‑NOW Blower Node

Single PCB design used for both ESP-NOW BME280 Node and ESP-NOW Inside Nodees

SX1262 LoRa WOR support (via EoRa-S3-900TB onboard SX1262 radio)

BME280 I²C header for GY-BME280-3.3V, 6 pin

EoRa-S3-900TB Battery‑optimized power domain

ESP‑NOW antenna layout

Mounting holes for outdoor enclosure use

ESP32‑S3 SuperMini Blower Node PCB

MPU6050 vibration sensor header

Vibration‑optimized mounting pattern

Simple 3.3V regulation

ESP‑NOW antenna layout

Compact footprint for blower, cabinet housing attachment
Note:  No electrical hook uprequired!

Fabrication Status
Gerber and drill files for all boards are included and ready for manufacturing.
The first batch of PCBs has been submitted and is currently awaiting delivery for hardware verification, fit‑testing, and firmware bring‑up.

## Folder Structure

```text
HVAC_System_Monitor/
│
├── BME280_Outside Node/
│   ├── BME280_WOR_Node.ino
│   ├── boards.h
│   ├── utilities.h
│   └── sx1262_commands.h
│  
│   
│
├── ESP-NOW_Inside_Node/
│   ├──  ESP-NOW_Inside_Node.ino  uses RadioLib library
|   ├── boards.h
│   └── utilities.h
│   
│
└── ESP-NOW_Blower_MPU6050/
    └──  ESP-NOW Blower_MPU6050ino

---

## Google Apps Script Setup

1. In Google Sheets, create a new Google Sheet.
2. Note the **Sheet ID** from the URL:
   `https://docs.google.com/spreadsheets/d/`**`<YOUR_SHEET_ID>`**`/edit`
3. Open the Script Editor: **Extensions → Apps Script**.
4. Delete the default `myFunction()` stub entirely.
5. Copy the full text contents of `Code.gs` from this repository.
6. Paste into the Script Editor.
7. Replace the placeholder Sheet ID in the script with the Sheet ID noted in Step 2.
8. **Save** (Ctrl+S or the save icon).
9. Click **Deploy → New Deployment**.
10. Select type: **Web App**.
11. Set **Execute as:** Me.
12. Set **Who has access:** Anyone.
13. Click **Authorize** → **Advanced** → click your Gmail account → **Allow**.
14. Copy the deployment ID and paste it into the Receiver sketch as the Google Script endpoint.

> **Note:** If you redeploy after changes, create a **New Deployment** each time and update
> the deployment ID in the Receiver sketch to match. (Editing an existing deployment to a
> new version can preserve the ID, but a fresh deployment always works.)

Numeric fields pass through a `numOrText()` helper in `Code.gs`: numbers are stored as
numbers, and the receiver's "Offline" sentinel (sent when the outdoor node fails to reply)
is preserved as text. Sheets functions like AVERAGE skip text cells automatically; in
pandas, load with `na_values=['Offline']`.

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

*"Project not possible without everyone's help."*

## License

MIT License — see `LICENSE` for details.
