# STM32F4 Telemetry & Secure Bootloader Firmware

![Status](https://img.shields.io/badge/Status-Development-yellow)
![Platform](https://img.shields.io/badge/Hardware-STM32F407VG-blue)
![RTOS](https://img.shields.io/badge/OS-FreeRTOS-green)

## 📖 Overview
This repository contains a production-grade firmware solution for the **STM32F4 Discovery Board**. It integrates a **Fail-Safe Bootloader** with a **FreeRTOS-based Telemetry Application**.

The system is designed to acquire sensor data (BME280 & Quectel LG290P GPS), handle low-power states (STOP Mode), and support reliable firmware updates with automatic rollback capabilities.

---

## 🏗 System Architecture

### 1. Dual-Bank Bootloader
Located at `0x0800 0000`, the custom bootloader ensures system integrity before jumping to the main application.
* **Fail-Safe Logic:** Detects crashes via IWDG (Independent Watchdog) and performs an automatic rollback to the backup firmware.
* **Update Mechanism:** Atomic updates using a dedicated `UPDATE_FLAG` sector.
* **Backup System:** Automatically archives the current valid firmware to a backup sector before applying updates.

### 2. Main Application (Telemetry)
Located at `0x0802 0000`, the application runs on FreeRTOS with the following tasks:
* **`TelemetryTask`:** Manages sensor acquisition (I2C) and GPS parsing (UART DMA). Implements a State Machine (FSM) to handle `IDLE`, `READ`, `TRANSMIT`, and `STOP` modes.
* **`MonitoringTask`:** Periodically feeds the Watchdog (IWDG) only if all other tasks report healthy status.
* **`UpdateTask`:** Handles user-triggered firmware update requests.

---

## 🛠 Hardware Setup

| Component | Interface | STM32 Pin | Description | Datasheet|
| :--- | :--- | :--- | :--- | :--- |
| **STM32F4 Discovery** | - | - | MCU (STM32F407VGT6) | [Link](https://drive.google.com/file/d/1mhD7Uf_oiQP9GO_3iPqgNyHZHy8Q4iKb/view?usp=share_link)|
| **Quectel LG290P** | UART (DMA) | `PB6` (TX), `PB7` (RX) | GNSS Module (9600/115200 bps) | [Link](https://drive.google.com/file/d/12X6thVbqKbrnyJQV3zxjC0I-gmP59GG9/view?usp=sharing)|
| **PPS Signal** | GPIO EXTI | `PA0` | Pulse Per Second for Wakeup (Optional) | - |
| **BME280** | I2C | `PB8` (SCL), `PB9` (SDA) | Temp, Pressure, Humidity Sensor |[Link](https://drive.google.com/file/d/1_w-kBY3O8H9Q3vl5sqF5BOwwBP2pOsWy/view?usp=sharing) |
| **Debug Console** | UART/SWV | `PA9`/`PA10` | Log Output | -|

---

## 💾 Flash Memory Layout
The internal Flash (1MB) is partitioned to align with STM32F4 physical sectors to minimize erase overhead.

| Region | Address | Size | Function |
| :--- | :--- | :--- | :--- |
| **Bootloader** | `0x0800 0000` | 64 KB | Immutable startup code |
| **Update Flag** | `0x0801 0000` | 64 KB | Shared flag for update triggers |
| **Main App** | `0x0802 0000` | 256 KB | Active Firmware |
| **Backup App** | `0x0806 0000` | 256 KB | Rollback Image |
| *User Data* | `0x080A 0000` | 384 KB | Storage / Filesystem |

---

## 🚀 Key Technical Features

### Low Power Management
* Implements **STOP Mode** to reduce power consumption when the GPS is idle.
* **Smart Wakeup:** Uses EXTI interrupts (or PPS) to wake the system and re-initialize UART DMA buffers without data loss.
* **Clock Recovery:** Automatically re-configures system clocks (HSE/PLL) upon wakeup.

### Robust UART Handling
* Utilizes `HAL_UARTEx_ReceiveToIdle_DMA` for efficient variable-length NMEA sentence reception.
* Includes strict checksum validation (`XOR`) to discard corrupted GPS packets during wakeup latency.

---

## 🤖 AI Usage & Prompt Engineering
As per the assessment guidelines, AI tools were used to assist in architectural brainstorming, driver debugging (specifically Low Power UART issues), and documentation.

* Full Prompt Strategy Log: [docs/prompt.md](docs/prompt.md)

---

## ⚙️ How to Build & Flash

### Prerequisites
* STM32CubeIDE (v1.14+)
* ST-Link Driver

### Steps
1.  **Clone the Repository:**
    ```bash
    git clone "https://github.com/dnmaulana1907/firmware_assignment.git"
    ```
2.  **Import Project:** Open STM32CubeIDE -> File -> Import -> Existing Projects.
3.  **Build:**
    * Select **Debug** configuration for optimized code size.
    * Build Project (Ctrl+B).
4.  **Flash:**
    * Flash the `Bootloader\Debug\Bootloader.bin` first.
    * Flash the `Application\Debug\PB_Application` (ensure address offset is set to `0x08020000`).

## ⚙️ Automated Build Workflow
This project utilizes STM32CubeIDE's Post-Build functionality to automatically generate bootloader-compatible binaries.

Upon a successful build, the IDE triggers `${ProjDirPath}/PB_App.command`, which executes the `firmware-prepare.py` script. This automation ensures:

1. **CRC32 Calculation**: Every build is automatically checksummed.

2. **Header Injection**: Firmware version and size are embedded.

3. **Artifact Generation**: Produces a `PB_Application.bin` ready for OTA/UART updates.

Note: The post-build script wrapper (`PB_App.command`) ensures cross-platform compatibility and simplifies the CI/CD pipeline.

## 🧪 Testing & Simulation Utils
To facilitate consistent testing without relying on live GPS signals or manual flashing tools, a Python-based simulation suite is provided in the `TEST/` directory. 

The environment is managed via Pipenv to ensure reproducible dependencies.

### 1. Environment Setup
Prerequisites: Python 3.x and Pipenv installed.
```
# Navigate to the TEST folder (or root, depending where Pipfile is)
cd TEST

# Install dependencies
pipenv install

# Activate the virtual environment
pipenv shell
```

### 2. GPS Simulation (`gps_simulation.py`)

This Python script is designed to emulate a live GNSS module (Quectel LG290P) for Hardware-in-the-Loop (HIL) testing. Unlike static string replay, this script dynamically generates NMEA sentences with randomized movement data to validate the firmware's parsing and checksum logic.

**Key Capabilities:**
1. **Auto-Discovery:** Automatically detects USB-to-Serial devices (`/dev/ttyUSB*` or `COM*`), eliminating the need to hardcode ports.

2. **Dynamic Geofencing:** Generates random coordinates specifically within the Jakarta region (Lat: -6.1 to -6.3, Lon: 106.7 to 106.9) to simulate realistic local tracking.

3. **Protocol Compliance:** Calculates valid NMEA XOR Checksums in real-time. This is critical for testing the firmware's `lg290p_validatechecksum()` function.

4. **Format:** Outputs standard `$GPRMC` sentences including UTC time, Status (A), Speed, and Course.

**Simulation Flow**
The script operates in a continuous loop, pushing data to the STM32 via UART every delay in seconds.
```
graph TD
    A([Start Simulation]) --> B{Auto-Detect Serial Port}
    B -- Found (e.g., ttyUSB0) --> C[Open Serial @ 115200 baud]
    B -- Not Found --> Z([Exit Error])
    
    C --> D[Start Loop]
    D --> E[Generate Random Lat/Lon (Jakarta Area)]
    E --> F[Generate Random Speed & Course]
    F --> G[Get System UTC Time]
    
    G --> H[Construct Raw NMEA String]
    H --> I[Calculate XOR Checksum]
    I --> J[Append Checksum (*XX) & CRLF]
    
    J --> K[Send via Serial]
    K --> L[Print to Console Log]
    L --> M[Sleep Several Seconds]
    M --> D
```
**Usage**
```
# Ensure pyserial is installed
pip install pyserial

# Run the simulation (Auto-connects to first USB serial device)
python TEST/gps_simulation.py
```

### 3. Automated Flashing (flash_firmware.py)
Instead of relying on standard DFU tools, this project implements a **custom UART Flash Protocol** to demonstrate secure and reliable firmware transfer.

The script `TEST/flash_firmware.py`handles the handshake, packetization, and CRC validation on the host side.

#### 1. Communication Protocol

The host communicates with the Bootloader using a packetized `Stop-and-Wait` ARQ protocol.

**A. Handshake** Before sending data, the Host verifies the Bootloader is listening.

Host Sends: `0xAA 0xAA 0xAA 0xAA 0xAA`

Target Acks: `0x09`

**B. Data Packet Structure (520 Bytes)** The firmware is split into 512-byte chunks and wrapped in a custom frame with a 32-bit CRC.

| Byte Offset | Field | Value | Description |
| :--- | :--- | :--- | :--- |
| **0** | **CMD** | `0x01` | Write Command |
| **1 - 512** | **Payload** | `Data...` | Raw Firmware Chunk (Padded with `0xFF`) |
| **513 - 514** | **Padding** | `0xFF` | Alignment Padding |
| **515 - 518** | **CRC32** | `0xXXXXXXXX` | Checksum of Bytes 0-514 (Big Endian) |
| **519** | **Footer** | `0x04` | End of Packet (EOT) |

#### 2. Workflow Logic
1. **Auto-Discovery**: The script scans for USB-Serial ports containing `"usbserial"` (configurable for Windows/Linux).

2. **Hex Parsing**: Loads the `PB_Application.hex` artifact and converts it to a raw byte stream.

3. Transmission:

    - Sends Handshake.

    - Loops through data chunks.

    - Calculates Python-side CRC32 (`binascii.crc32`).

    - Waits for `ACK `(`0x09`) from STM32 after every packet.

#### 3. Usage
```
# Run the flasher (Ensure the device is in Bootloader mode)
python TEST/flash_firmware.py
```
**Note**: The script defaults to 115200 baud and targets the `PB_Application.hex` build artifact.

## 🧠 Application Logic & Task Architecture

The firmware implements a cooperative multitasking architecture using FreeRTOS. The system is divided into three primary tasks, each with specific responsibilities and inter-task communication mechanisms.

### **1. Telemetry Task** (`telemetry_task`)
**Priority:** HIGH | **Period:** Event-Driven This is the core task responsible for data acquisition and power management. It operates as a Finite State Machine (FSM) to handle asynchronous events from GPS (UART), Sensors (I2C), and Timers.

- **FSM States:**
    - `STATE_INIT`: Initializes BME280 sensor, starts the 5-second sampling timer, and configures DMA for GPS reception.
    - `STATE_IDLE`: Waits for `osSignalWait` events:
         - `UART_SIGNAL`: Triggered when a full NMEA sentence is received (Idle Line Detection).
         - `WAKEUP_SIGNAL`: Triggered every 5s by `SensorTimer` to force a sensor reading.
         - *Timeout Logic*: If no activity is detected for 30s (`IDLE_TIME_OUT_MS`), transitions to `STATE_STOP`.

    - `STATE_READ`: Parses NMEA GPS data (Checksum validation included), reads BME280 (Temp, Pressure, Humidity) via I2C, and samples Fuel Level via ADC.

    - `STATE_TRANSMIT`: Formats and logs the aggregated telemetry data to the debug console (simulating RF transmission).

    - `STATE_STOP`: Enters Low-Power STOP mode.
        - Configures RTC Wakeup Timer.

        - Re-routes UART RX pin to EXTI to allow "Wake-on-RX".

        - Suspends OS scheduler (`osThreadSuspendAll`).

    - `STATE_ERROR:` Handles parsing failures or sensor timeouts by resetting buffers and restarting reception.

### **2. Update Task** (`update_task`)

**Priority:** Normal | **Period:** 3000ms (Blocked) Handles firmware update requests triggered by external events (e.g., button press or specific UART command).

- Waits for `UPDATE_SIGNAL`.

- Writes the` 0x01` flag to the shared Flash sector (`UPDATE_FLAG_ADDRESS`).

- Performs a System Reset (`HAL_NVIC_SystemReset`) to handover control to the Bootloader.

### **3. Monitoring Task** (`monitoring_task`)
**Priority:** Low | **Period:** 1000ms Acts as a "Software Watchdog" aggregator to ensure system health.

- **Mechanism:** It waits for "Alive Signals" (`WDT_TASK_TELE` & `WDT_TASK_UPDATE`) from other critical tasks.

- **Action:** Only refreshes the hardware Independent Watchdog (`HAL_IWDG_Refresh)` if ALL monitored tasks have reported in within the timeout window.

- **Visual Feedback:** Toggles the Green LED to indicate a healthy heartbeat.

### 💡 Key Design Patterns Used

- **Event-Driven Communication:** Instead of polling, tasks block on `osSignalWait()`. This maximizes CPU efficiency and allows the Idle Task to sleep the core when no events are pending.

- **Robust UART Handling:**

    - Uses `HAL_UARTEx_ReceiveToIdle_DMA` to handle variable-length NMEA sentences efficiently.

    - Implements `__HAL_UART_CLEAR_OREFLAG` during initialization to prevent "Overrun Error" lockups during wake-up transitions.

- **Fail-Safe State Recovery:** The FSM includes a default `STATE_ERROR` handler that resets communication buffers (`memset` & restart DMA) to recover from noise or data corruption automatically.

## ⚠️ Known Limitations & Hardware Constraints

This project was developed under specific hardware constraints. The following limitations should be noted during review:

**1. ADC Simulation (Fuel Level)**

- **Constraint:** Lacking a potentiometer to simulate continuous analog input.

- **Impact:** The Fuel Level monitoring (ADC) was tested using only fixed voltage reference points available on the Discovery board (GND, 3.3V, and 5V). A continuous range test was not performed.

**2. I2C Sensor Integration (BME280)**
- **Constraint:** The BME280 sensor module available for testing lacked on-board pull-up resistors, and external 4.7kΩ resistors were not available at the time of development.

- **Impact:** While the I2C driver and parsing logic are fully implemented in `task_telemetry_process`, the physical communication with the sensor could not be verified. The system may report I2C timeouts in the current hardware setup.

**3. UART Wake-up Latency (STOP Mode)**
- **Issue:** When waking from STOP mode using the RX pin as an EXTI source, the first incoming NMEA packet is often corrupted or missed.

- **Technical Root Cause:** The time required for the MCU to wake up (HSI/PLL startup) and re-initialize the UART peripheral (`HAL_UART_Init`) exceeds the bit-width of the incoming data stream at **115200 bps**.

- **Software Mitigation:** The firmware relies on the Checksum Validation mechanism. If the first packet is incomplete (framing error), it is safely discarded, and the system processes the subsequent valid packet without crashing.