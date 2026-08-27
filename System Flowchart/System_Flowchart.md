# System Flowchart

1. Blower turns OFF
2. Blower Node detects vibration stop
3. Blower Node sends ESP-NOW flag + data to Inside Node
4. Inside Node receives packet
5. Inside Node sends WOR preamble
6. Outdoor SX1262 receives WOR and wakes ESP32-S3
7. Outdoor ESP32-S3 runs `setup()`
8. Reads wake reason EXT0 or Undefined
9. EXT0 Initializes SX1262 with no reset (Warm boot)
10. Undefined Initializes SX1262 with reset  (Cold boot)
11. Reads IRQ flags
12. Reads BME280
13. Sends ESP-NOW data to Inside Node
14. Arms RxDutyCycle
15. Returns to deep sleep
