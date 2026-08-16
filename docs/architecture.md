# Architecture

The ESP32 is the controller. A GY-521 (MPU-6050) measures angular velocity
and acceleration over I²C. A NEO-6M GPS module provides ground speed and
position over UART. An ADA254 breakout writes the log to a microSD card over
SPI. FreeRTOS schedules the work, running each subsystem as an independent
task so that a slow peripheral can't stall a fast one.

## Why an RTOS

SD cards have a tendency to stall unpredictably during internal wear-levelling:
this can range anywhere from tens to hundreds of milliseconds with no warning. In a
superloop, the IMU isn't sampled during a stall, so every stall punches a hole
in the gyro integration. That error accumulates and is unrecoverable.

FreeRTOS separates the sampler from the writer. The IMU task runs on a fixed 100 Hz
schedule regardless of what the SD card is doing; the logger drains a buffer whenever
the card is willing to accept data. Stalls become latency instead of lost samples.

## What starvation looked like

A busy-waiting task at priority 4, between a producer at 5 and a consumer at 3. The
result was a clean cut at the hog's priority level:

- Producer (5) ran normally (it preempted the hog every 100ms).
- Consumer (3) never ran again.

Blocking calls ('vTaskDelay', 'xQueueRecieve') yield the CPU voluntarily. A busy-wait
doesn't, so anything below it is permanently starved.

## Task table

| Task | Priority | Rate | Why |
|---|---|---|---|
| IMU | 5 | 100 Hz | Fixed cadence; a missed sample is unrecoverable |
| Fusion | 4 | 100 Hz | Consumes IMU output, must keep pace |
| GPS | 3 | 5 Hz | Slow correction term; latency is tolerable |
| Logger | 2 | ~2 Hz | **Deliberately low.** SD stalls must never block sampling |
| Status | 1 | 10 Hz | LEDs and button (no timing requirement) |

The logger's priority is the most deliberate choice here. Place it above the
IMU task and an SD stall blocks sampling which is exactly the kind of failure
this architecture is designed to prevent. Keeping it low means stalls delay
the write instead, and the ring buffer absorbs the gap.