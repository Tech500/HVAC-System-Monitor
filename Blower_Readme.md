# Blower Node (MPU6050 Vibration Sensing + ESP‑NOW TX)

The Blower Node monitors HVAC blower state using an MPU6050 accelerometer attached to the blower housing. It detects ON/OFF cycles via vibration variance thresholds and sends state changes to the Receiver Node using ESP‑NOW.

## Features

- Safe, non‑electrical blower monitoring
- MPU6050 vibration sensing
- Variance‑based ON/OFF detection
- ESP‑NOW transmission to Receiver Node

## Files

- `Blower_Node.ino`  
- `boards.h`  
- `utilities.h`  
- `README.md`

## Year‑round tracking

Blower ON/OFF events are logged year‑round as part of the perpetual Google Sheets archive, enabling long‑term HVAC behavior analysis.
