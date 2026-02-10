# Firmware Architecture & Bootloader Design

## 1. System Overview
The system runs on an **STM32F407VGT6** (1MB Flash, 192KB RAM) and features a robust custom bootloader. The architecture is designed for high availability, supporting:
* **Over-The-Air (OTA) / UART Updates.**
* **Fail-Safe Rollback:** Automatic restoration of previous firmware if the main application crashes (Watchdog trigger).
* **Backup Management:** Automatic archiving of working firmware.
* **Atomic Updates:** Utilizes a dedicated flag sector to manage update states.

---

## 2. Flash Memory Layout
The 1024KB Flash memory is partitioned to align with the physical sectors of the STM32F407 to minimize erase overhead.

| Region Name | Start Address | Size | STM32 Sector(s) | Description |
| :--- | :--- | :--- | :--- | :--- |
| **Bootloader** | `0x0800 0000` | 64 KB | Sectors 0-3 | The immutable code responsible for system health checks and jumping to the application. |
| **Update Flag** | `0x0801 0000` | 64 KB | Sector 4 | Stores the `0x01` flag to trigger firmware updates. Also acts as a buffer between Bootloader and App. |
| **Main Application** | `0x0802 0000` | 256 KB | Sectors 5-6 | The active running firmware. |
| **Backup Application** | `0x0806 0000` | 256 KB | Sectors 7-8 | A compressed or raw copy of the last known working firmware. |
| *Unused / Storage* | `0x080A 0000` | ~384 KB | Sectors 9-11 | Available for file system or configuration storage. |

### Memory Definition (`flash_layout.h`)
```c
#define BOOTLOADER_START_ADDRESS    0x08000000
#define UPDATE_FLAG_ADDRESS         0x08010000
#define APPLICATION_START_ADDRESS   0x08020000
#define APPLICATION_BACKUP_ADDRESS  (APPLICATION_START_ADDRESS + 0x40000) // 0x08060000
#define APP_MAX_SIZE                (256 * 1024)
#define VEC_OFFSET                  0x200
```

## 3. Boot Process Flow

The bootloader executes a strict state machine ``(UpdateStatus)`` based on the Reset Cause and the Update Flag.

### 3.1. Reset Cause Analysis
On startup, the system reads the RCC_CSR register to determine why the MCU reset:
- **IWDG Reset (Watchdog)**: Indicates the Main App froze or crashed. -> `Triggers Rollback`.

- **Soft Reset**: Indicates a commanded reset (likely after an update or config change). -> `Triggers Backup Check`.

- **BOR/PIN/Power Reset**: Normal startup. -> Triggers Verification.

### 3.2. Boot States

| State | Condition | Action |
| :--- | :--- | :--- |
| **UPDATE_FIRMWARE** | `UPDATE_FLAG == 0x01` | 1. Erase Flag.<br>2. Download new FW.<br>3. Validate.<br>4. System Reset. |
| **BACKUP_MAIN_APP** | `Soft Reset` AND No Backup exists | 1. Copy Main App Flash -> Backup Flash.<br>2. Validate Backup.<br>3. Proceed to boot. |
| **MAIN_APP_ERR** | `IWDG Reset` (Crash detected) | 1. **Rollback:** Copy Backup Flash -> Main App Flash.<br>2. Validate restored image.<br>3. Proceed to boot. |
| **CHECK_MAIN_APP** | Normal Power On | 1. Calculate CRC/Hash of Main App.<br>2. If valid, Boot.<br>3. If invalid, check Backup integrity. |

## 4. Application Jump Mechanism

To ensure a clean transition from Bootloader to Application, the following sequence is performed in `start_application()`:

1. **Disable Interrupts**: `__disable_irq()` prevents ISRs from firing during the jump.

2. **Stop System Ticks**: `SysTick` is disabled and cleared to prevent OS context switching immediately after jump.

3. **Stack Pointer Update**: The Main Stack Pointer (MSP) is updated to the Application's stack address.

4. **Vector Forwarding**: The Program Counter (PC) is loaded with the Reset Handler address located at `APPLICATION_START_ADDRESS + VEC_OFFSET + 4`.

## 5. Security & Safety Features

### 5.1. Firmware Validation
Before any boot or update, the firmware undergoes validation (referenced in `authentication.h`). If validation fails:

    1. The system attempts to locate a valid Backup.

    2. If the Backup is valid, it restores it to the Main region.

    3. If both Main and Backup are corrupt, the system halts (Fatal Error) to prevent undefined behavior.

### 5.2. Independent Watchdog (IWDG)
The Bootloader refreshes the IWDG during long operations (like Flashing or Backup copying) to prevent false resets. The IWDG is the primary mechanism for detecting application lockups.

### 5.3. Linker Protection
The Linker Script (`STM32F407VGTX_FLASH.ld`) provided is for the Bootloader. It restricts the Bootloader's code space to the first 64KB (`0x8000000` to `0x8010000)`, ensuring the compiler triggers an error if the Bootloader grows into the Update Flag or Application space.