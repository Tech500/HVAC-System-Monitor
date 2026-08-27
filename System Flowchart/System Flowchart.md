flowchart TD

    %% Blower Node
    BN0([Blower turns OFF])
    BN1[Blower Node detects vibration stop]
    BN2[Send ESP-NOW: blowerFlag + data]

    %% Inside Node
    IN1[Inside Node receives blower packet]
    IN2[Inside Node sends WOR preamble]

    %% Outdoor WOR Node
    WS0[Outdoor SX1262 receives WOR → wakes ESP32-S3]
    WS1[setup() runs]
    WS2[Read wake reason]
    WS3[Initialize SX1262]
    WS4[Read IRQ flags]
    WS5[Read BME280]
    WS6[Send ESP-NOW to Inside Node]
    WS7[Arm RxDutyCycle]
    WS8[Return to deep sleep]

    %% Connections
    BN0 --> BN1 --> BN2 --> IN1 --> IN2 --> WS0
    WS0 --> WS1 --> WS2 --> WS3 --> WS4 --> WS5 --> WS6 --> WS7 --> WS8

