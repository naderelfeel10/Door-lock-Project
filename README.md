# Door Locker Security System  
**CSE211 / CSE322 – Introduction to Embedded Systems**  
**Fall 2025**

---

## Project Overview
The **Door Locker Security System** is an embedded systems project implemented using **two TM4C123 (Tiva C) microcontroller-based ECUs**.  
The system provides **password-based secure access control**, motorized door locking/unlocking, alarm handling, and persistent configuration storage using EEPROM.

The project is designed to apply professional embedded software practices, including layered architecture, inter-ECU communication, systematic testing, and resource analysis.

---

## Project Objectives
This project aims to provide hands-on experience in:
- Layered embedded software architecture (MCAL, HAL, Application)
- Inter-microcontroller communication using UART
- Persistent data storage using EEPROM
- Timer-based control (SysTick, GPTM)
- Security feature implementation (password, lockout)
- Systematic software testing and documentation
- Professional engineering practices (coding standards, resource measurement)

---

## System Architecture
The system is divided into **two ECUs**:

### 1. HMI_ECU (Human-Machine Interface ECU)
Responsible for user interaction.

**Hardware Components**
- 16x2 LCD
- 4x4 Keypad
- Potentiometer (ADC-based)
- RGB LED
- UART interface
- SysTick / Timers

**Responsibilities**
- Display system messages and menus
- Capture user input
- Configure auto-lock timeout
- Communicate user commands to Control_ECU

---

### 2. Control_ECU
Responsible for system logic and actuation.

**Hardware Components**
- EEPROM
- DC Motor (door lock)
- Buzzer (alarm)
- UART interface
- GPTM Timers

**Responsibilities**
- Password verification and storage
- Door lock/unlock motor control
- Alarm handling and lockout logic
- Persistent configuration management

---

## Key Features
- Password-based access control
- EEPROM-based persistent storage
- UART-based inter-ECU communication
- Motorized door locking/unlocking
- Alarm system with buzzer
- Auto-lock timeout configuration via potentiometer
- Visual system status via RGB LEDs

---

## Functional Requirements

### 1. Initial Password Setup
- LCD prompts user to enter a 5-digit password
- Password is masked with `*`
- Confirmation required
- Password stored in EEPROM

### 2. Main Menu
Displayed on LCD:
- `+` → Open Door  
- `-` → Change Password  
- `*` → Set Auto-Lock Timeout  

### 3. Open Door
- Password verification required
- Correct password:
  - Door unlocks
  - System waits for configured timeout (5–30 seconds)
  - Door locks automatically
- Incorrect password:
  - Maximum of 3 attempts
  - Buzzer alarm activated
  - Temporary lockout enforced

### 4. Change Password
- Old password verification required
- New password setup with confirmation
- Lockout enforced after 3 failed attempts

### 5. Auto-Lock Timeout Configuration
- Timeout adjusted using potentiometer (5–30 seconds)
- Live value shown on LCD
- Password verification required to save
- Stored persistently in EEPROM

---

## Software Architecture
The software follows a **layered architecture**:

Application Layer
│
HAL (Hardware Abstraction Layer)
│
MCAL (Microcontroller Abstraction Layer)

yaml
Copy code

- **MCAL**: GPIO, UART, ADC, Timers, SysTick
- **HAL**: LCD, Keypad, Motor, Buzzer, Potentiometer
- **Application**: Door logic, password handling, menus, system control

---

## Non-Functional Requirements

### Code Quality
- Coding standard followed (MISRA C / CERT C)
- At least **5 documented violations**
- Each violation includes:
  - Description
  - Before fix
  - After fix

### Resource Analysis
- Measurement of:
  - ROM usage
  - RAM usage
  - Stack usage
- Methodology explained using:
  - Map files
  - Debugger analysis
  - Runtime tools

### Testing
- Independent testing code developed by testing team
- Includes:
  - Unit tests (drivers)
  - Integration tests
  - Full system functional tests
- Automatic logging in **PASS / FAIL** format

---
