# AI Assistance Log & Prompt Strategy

## Overview
This document outlines the strategy used to leverage AI (Gemini/ChatGPT) during the development of this firmware. The AI was utilized primarily for:
1.  **Architectural Brainstorming:** Validating FreeRTOS task partitioning.
2.  **Driver Implementation Patterns:** generating boilerplate for DMA-based UART and I2C transactions.
3.  **Debugging & Hardware Constraints:** Analyzing specific STM32F4 low-power mode behaviors (Wakeup latency).
4.  **Unit Testing:** Generating edge-case NMEA data for parser validation.

---

## 1. System Architecture & FreeRTOS Design
**Objective:** To design a non-blocking, event-driven architecture.

* **Prompt:** "Propose a FreeRTOS task architecture for an embedded telemetry system on STM32F4. The system needs to read sensors (I2C), handle GPS streams (UART), and manage low-power STOP mode. Focus on inter-task communication using Signals vs Queues for synchronization."
* **Outcome:** Adopted a design using `osSignalWait` for the Telemetry Task to remain in a blocked state until triggered by Timer or UART Idle events, minimizing CPU usage.

## 2. Peripherals & Driver Implementation
**Objective:** To implement efficient DMA-based communication handling variable-length data.

* **Prompt:** "Provide a C implementation example using STM32 HAL for `HAL_UARTEx_ReceiveToIdle_DMA`. Explain how to handle the 'IDLE' interrupt to process variable-length NMEA sentences efficiently without polling."
* **Prompt:** "How to correctly implement BME280 compensation formulas using integer arithmetic to avoid excessive floating-point overhead on an MCU without double-precision FPU."
* **Outcome:** Implemented `start_receive_gps()` with Idle Line detection, ensuring the task only wakes up when a full data packet is received.

## 3. Algorithm & Logic (NMEA Parsing)
**Objective:** To ensure robust data parsing and validation.

* **Prompt:** "Generate a C function to validate NMEA checksums (XOR method). Also, provide a set of 'edge-case' NMEA strings (e.g., empty fields, max length, invalid checksums) to unit test the parser."
* **Outcome:** Integrated a robust `lg290p_validatechecksum` function and used the generated test vectors to verify parser stability against corrupted GPS data.

## 4. Debugging Low Power & Wakeup Latency
**Objective:** To solve data loss issues during transition from STOP mode to RUN mode.

* **Prompt:** "Analyze a scenario where the first byte of UART data is lost/corrupted when waking an STM32F4 from STOP mode using an EXTI interrupt on the RX pin. Explain 'Wakeup Latency' and suggest hardware (PPS) vs software workarounds."
* **Prompt:** "Why does `HAL_UART_ErrorCallback` trigger instead of `RxEventCallback` immediately after wakeup? Explain the state of the UART Overrun (ORE) flag during clock startup sequence."
* **Outcome:** Identified that the start-bit was being missed due to oscillator startup time. Implemented a robust error-clearing mechanism (`__HAL_UART_CLEAR_OREFLAG`) and state recovery logic to ignore the first partial packet after wakeup.

## 5. Code Quality & Refactoring
**Objective:** To clean up logic and ensure "Clean Code" standards.

* **Prompt:** "Review this C state machine implementation for the Telemetry Task. Identify potential race conditions between the DMA ISR and the main loop. Suggest improvements for readability using `switch-case` best practices."
* **Outcome:** Refactored the `if-else` logic into a structured `switch(currentState)` FSM, improving maintainability and debugging clarity.

---
