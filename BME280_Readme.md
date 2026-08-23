# WOR Sensor Node (Outdoor BME280 + LoRa WOR RX + ESP‑NOW TX)

This node is an ultra‑low‑power outdoor sensor that wakes only when a WOR cycle is triggered. It uses SX1262 Wake‑On‑Radio (WOR) with EXT0 wake on GPIO16 and sends outdoor BME280 readings via ESP‑NOW.

## Features

- SX1262 LoRa WOR RX (preamble‑only)
- EXT0 wake via DIO1 → GPIO16
- BME280 outdoor temperature/humidity/pressure
- ESP‑NOW transmission to Receiver Node
- Deep sleep between WOR cycles

## Files

- `BME280_WOR_Node.ino`  
- `boards.h`  
- `utilities.h`  
- `sx1262_commands.h`  
- `sx1262_commands.cpp`  
- `README.md`

## Year‑round operation

Runs continuously year‑round; each WOR wake produces outdoor data that is logged perpetually in Google Sheets.
