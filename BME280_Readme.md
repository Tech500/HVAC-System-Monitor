# Ultra-Low-Power LoRa WOR BME280 Node

### SX1262 RxDutyCycle + EoRa-S3-900TB (ESP32-S3)

### *LoRa's only job is to wake the MCU.*

This open-source project demonstrates how to build an **ultra-low-power LoRa wake-on-radio receiver** using:

- The **built-in SX1262** LoRa transceiver on the EoRa-S3-900TB
- **RxDutyCycle** (Semtech Wake-On-Radio mode)
- **EXT0 deep-sleep wake**
- **One jumper wire**

The SX1262 periodically listens for a **LoRa preamble**.
If a preamble is detected, the radio asserts **DIO1**, waking the EoRa-S3-900TB from deep sleep.

**In this project, LoRa's only function is to wake the MCU.**
The SX1262 does not decode packets during RxDutyCycle — it simply detects the preamble and wakes the microcontroller.

This design is ideal for battery-powered sensors, remote monitors, and long-range low-power systems.

---

## What You Need

### Hardware

- **EoRa-S3-900TB** (ESP32-S3 low-power board with built-in SX1262)<br>
[EoRa-S3-900TB Development Board](https://ebyteiot.com/products/ebyte-oem-odm-eora-s3-900tb-22dbm-7km-mini-low-power-and-long-distance-sx1262-rf-module-lora-module-915mhz)
- **Two BME280 GY-BME280-3.3 Breakouts
- **One jumper wire** (GPIO33 → GPIO16)

### Required Files (must be in the sketch folder)

Place these **three files** in the same folder as your `.ino` or `main.cpp`:

| File | Purpose |
|---|---|
| `boards.h` | Ebyte board initBoard, board setup |
| `utilities.h` |Ebyte board, hardware mapping (DIO1 = GPIO33) |
| `SX1262_Commands.h` | Semtech low-level commands set (SetRxDutyCycle, IRQ ops, etc.) |

### Sketch Folder Placement

The entire sketch folder can be copied anywhere:

- Arduino sketchbook
- A custom project directory
- A local library folder

As long as these three files remain in the same folder as the sketch, the project will compile and run correctly.

### Mandatory Requirement

**ESP32 Arduino Core 3.3.10 is required!!!**

This project must be compiled using ESP32 Arduino Core 3.3.10. Earlier or later versions may:

- break EXT0 wake behavior
- Break ESP-NOW
- change RTC GPIO handling
- alter deep-sleep current
- modify SX1262 SPI timing
- cause WOR wake failures

Core 3.3.10 is the validated, stable version for the EoRa-S3-900TB + SX1262 WOR architecture.

---

## Wiring Overview

SX1262 is built into the EoRa-S3-900TB. All SX1262 connections (SPI, RESET, BUSY, DIO1) are already routed internally on the board. No external SX1262 wiring is required.

### Wake-On-Radio Jumper (Important)

The SX1262's DIO1 pin is internally routed to GPIO33 on the EoRa-S3-900TB.

But GPIO33 cannot wake the ESP32-S3 from deep sleep.

So we add one jumper wire:

**GPIO33 → GPIO16**

GPIO16 is RTC-capable and is used for EXT0 wake. This jumper is required for WOR wake.

---

## How RxDutyCycle Works

RxDutyCycle is a low-power receive mode built into the Semtech SX126x LoRa transceivers. Instead of staying in continuous receive, the radio periodically wakes up, opens a short listening window, and checks only for the presence of a LoRa preamble.

Semtech describes it as:

> "The receiver is activated periodically to detect the presence of a valid LoRa preamble."

### What the SX1262 does

1. Sleep
2. Wake periodically
3. Open RX window
4. Look for LoRa preamble
5. If detected → assert DIO1
6. MCU wakes via EXT0
7. MCU reads and sends BME280 Readings to Inside Node via ESP-NOW
8. MCU re-arms RxDutyCycle and returns to sleep

### The Only Function of LoRa Here

In this project, LoRa is used only to wake the EoRa-S3-900TB. The SX1262 does not decode packets during RxDutyCycle — it simply detects the preamble and wakes the MCU.

---

## Firmware Wake Setup (EXT0 on GPIO16)

Wake pin is defined separately:

```cpp
#define WAKEUP_PIN GPIO_NUM_16
```

Before entering deep sleep, GPIO16 must be placed into the RTC domain:

```cpp
rtc_gpio_init(GPIO_NUM_16);
rtc_gpio_set_direction(GPIO_NUM_16, RTC_GPIO_MODE_INPUT_ONLY);
rtc_gpio_pulldown_en(GPIO_NUM_16);     
esp_sleep_enable_ext0_wakeup(GPIO_NUM_16, 1);   // or 0 depending on DIO1 polarity
```

If `rtc_gpio_init()` is missing, EXT0 will never wake the MCU.

---

## Power Profile (Three-Segment Model)
These values describe radio behavior only during the RxDutyCycle WOR pattern, measured using the Nordic PPK2 grey-selection box averages.

### Sleep Segment — WOR Sleep Portion

This is the radio's sleep portion inside the WOR cycle, not the SX1262 standalone deep-sleep mode.

- **Average: ~22.40 µA**
- This is the correct value from the Nordic PPK2 grey-selection box
- Any lower instantaneous readings (including single-digit µA) are not representative averages and must not be used for documentation

### Listening Segment — WOR RX Window

The radio opens its RX window and listens for a LoRa preamble.

- **Average: ~7.79 mA**
- Short duration (8.58 ms), autonomous listening on the SX1262

### One Complete RxDutyCycle **

This is the average of one RxDutyCycle:

- One complete RxDutyCycle (~94.01 µA Average, 997.8 ms) 
- Sleep portion (~22.40 µA, 987.9 ms)
- Listening window (~7.79 mA Average, 8.58 ms)
- RxDutyCycle pattern repeats continuously;
  accounting for the higher ~94.01 µA current.
- Wake current during each event-driven wake; during 24
  hours must also be added in the power profile.

[Nordic Power Profiler Kit 2 Observations](https://github.com/Tech500/HVAC-System-Monitor/blob/main/NPPK2_RxDutyCycle_Observations.md)

---

## EoRa-S3-900TB Wake Behavior (Event-Driven)

When WOR asserts DIO1:

1. The MCU wakes
2. Performs housekeeping
3. Reads SX1262 IRQ flags
4. Reads and sends BME280 reading
5. Re-arms WOR
6. Returns to deep sleep

Wake current is ~45.27 mA,  but only during the short wake window.

Typical deployments see ~20 wake events per day, so this cost is small and intermittent.

---

## Example Log Output

```
[WAKE] EXT0 Triggered by GPIO 16 (RTC Level: 1)
[WOR] Warm Boot active. SX1262 IRQ: 0x0004
BME280 -> Temp: 80.88 F  Hum: 54.18 %  Pres: 1017.4484 hPa
[ESP-NOW] Send to hub: OK
[ESP-NOW] Sent alertFlag: TRUE
Entering Deep Sleep with RxDutyCycle armed...

---

## Project Structure

```
/src
  * Node.ino
  boards.h
  utilities.h
  sx1262_commands.h
/docs
  images/
  timing/
```

---

## Contributions Welcome

This project is open-source and beginner-friendly. Contributions are welcome in:

- documentation
- timing diagrams
- SX1262 improvements
- deep-sleep optimization
- transmitter examples

---

## Summary

- SX1262 is built into the EoRa-S3-900TB
- SX1262 runs RxDutyCycle autonomously
- LoRa's only job is to wake the MCU
- DIO1 → GPIO33 → jumper → GPIO16 → EXT0 wake
- Three support files must be in the sketch folder
- Sketch folder can be placed anywhere
- ESP32 Core 3.3.10 is mandatory!!!
- WOR sleep segment averages ~22.40 µA
- Wake events are rare and efficient
