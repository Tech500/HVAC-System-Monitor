# SX1262 SetRxDutyCycle — Nordic PPK2 rxDutyCycle tick Observations

Power-profiling notes for `SetRxDutyCycle` (rxPeriod / sleepPeriod, in 15.625 µs ticks) on the SX1262, captured with a Nordic PPK2. 
Three experiment Images below, one per tick configuration tested.

---

### Nordic PPK II Observation Image 01

![Nordic PPK2 Observaton --OneDutyCycle 01](./HVAC%20rxDutyCycle/01%20One%20RxDutyCycle.png)

### Measurements

#define RXDC_RX_TICKS       512UL  
#define RXDC_SLEEP_TICKS  63488UL  


| Metric | Value |
|---|---|
| Sleep average current | 22.40 µA |
| Duty-cycle average current | 94.01 µA |
| Peak current (RX window) | 13.29 mA |
| Wake event duration | 8.0 ms (per programmed rxPeriod) |
| Estimated battery life (3000 mAh) | ~43.7 months |

### Observations

- Selection window 997.6 ms, matching the programmed 1.000 s full cycle (8.0 ms RX + 992.0 ms sleep)
- By far the most efficient config measured — ~27× lower average current than Image 02, at the cost of a longer worst-case wake latency

---


### Nordic PPK II Observation Image 02

![Nordic PPK2 Observaton --OneDutyCycle 02](./HVAC%20rxDutyCycle/02%20One%20RxDutyCycle.png)

### Measurements

#define RXDC_RX_TICKS       2048UL  
#define RXDC_SLEEP_TICKS   64000UL  

| Metric | Value |
|---|---|
| Sleep average current | 21.94 µA  |
| Duty-cycle average current | 284.39 µA |
| Peak current (RX window) | 9.13 mA |
| Wake event duration | 32.0 ms (per programmed rxPeriod) |
| Estimated battery life (3000 mAh) | ~14.4 months |

### Observations

- Selection window 1.025 s, close to the programmed 1.032 s full cycle (32.0 ms RX + 1000.0 ms sleep)

---

### Nordic PPK II Observation Image 03

![Nordic PPK2 Observaton --OneDutyCycle 03](./HVAC%20rxDutyCycle/03%20One%20RxDutyCycle.png)

### Measurements

#define RXDC_RX_TICKS       1050UL  
#define RXDC_SLEEP_TICKS    5120UL  

| Metric | Value |
|---|---|
| Sleep floor current | ____ µA (not resolvable at this zoom level) |
| Duty-cycle average current | 2.52 mA |
| Peak current (RX window) | 13.20 mA |
| Wake event duration | 32.0 ms (per programmed rxPeriod) |
| Estimated battery life (3000 mAh) | ~1.6 months |

### Observations

- Selection window 111.8 ms, closely matching the programmed 112.0 ms full cycle (32.0 ms RX + 80.0 ms sleep)
- High average current (2.52 mA) driven by the 28.6% RX duty cycle — as expected, this is the least efficient config of the sweep

---

## Summary Comparison

| Image | Ticks (rxPeriod / sleepPeriod) | Duty-cycle avg (µA) | Est. battery life |
|---|---|---|---|
| 01 | 512 / 63488 | 94.01 | ~43.7 months |
| 02 | 2048 / 64000 | 284.39 | ~14.4 months |
| 03 |2048 / 5120 | 2520 | ~1.6 months |

### Conclusion

- Sleep period is the dominant lever, not RX period: Image 03 keeps the same 32 ms RX window as Image 02 but stretches sleep from 80 ms to 1000 ms, cutting average current ~9× (2520 µA → 284 µA)
- Shrinking the RX window matters too: Image 01 cuts RX from 32 ms to 8 ms on top of a comparable ~1 s sleep period, taking average current down another ~3× versus Image 03 (284 µA → 94 µA)
- Combined effect across the sweep so far: ~27× reduction in average current (2520 µA → 94 µA) and a projected battery-life improvement from ~1.6 months to ~43.7 months on a 3000 mAh cell
- Peak RX current stays roughly flat (9–13 mA) across all three configs — the gains come entirely from duty-cycle shaping, not from reducing per-window draw
