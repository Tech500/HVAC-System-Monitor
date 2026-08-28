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
