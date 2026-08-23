# Receiver Node (Indoor BME280 + ESP‑NOW Aggregator + Google Sheets)

The Receiver Node collects data from the WOR Sensor Node and Blower Node via ESP‑NOW, reads its own indoor BME280 sensor, merges all values, and uploads the result to Google Sheets.

## Features

- ESP‑NOW RX (outdoor BME280 + blower ON/OFF)
- Local indoor BME280 sensor
- Combines indoor + outdoor + blower state
- Uploads HVAC data to Google Sheets
- Perpetual month‑to‑month and year‑to‑year logging

## Files

- `Receiver_Node.ino`  
- `boards.h`  
- `utilities.h`  
- `README.md`

## Year‑round logging

Every record is appended to Google Sheets; an Apps Script can organize data by month and year, creating a multi‑year HVAC history.
