## Project Overview

This project aims to develop an embedded system application for the MSP432 microcontroller. The application utilizes various sensors and peripherals to monitor and interact with environmental conditions.

## Project Goal

The primary functions of this system include:

1. **Menu Navigation**: Implement a user interface that allows navigation through a menu of predefined options using physical buttons. Each menu option corresponds to a specific task with an associated duration.
2. **Environmental Monitoring**: Utilize light and temperature sensors to monitor environmental conditions. Define thresholds for light intensity and temperature changes to trigger specific actions.
3. **Timed Events and Alerts**: Implement timers to manage the duration of selected tasks and to provide visual feedback through LEDs. Trigger alerts when timers expire or when significant changes in temperature are detected.
4. **User Feedback**: Display messages and information on an LCD screen to guide the user through the menu and provide real-time updates on system status. Use LEDs and potentially a buzzer to signal important events or alerts.

## Project Structure

The project consists of the following main components:

1. **Source Files**:
   - `main.c`: Contains the main logic of the project.
   - `timer.c` and `timer.h`: Functions for initializing and managing timers.
   - `gpio.c` and `gpio.h`: Functions for managing GPIO (General Purpose Input/Output).

2. **Configuration Files**:
   - `.ccsproject`, `.cproject`, `.project`: Project configuration files for Code Composer Studio.

3. **System Files**:
   - `system_msp432p401r.c`: System configuration for the MSP432 microcontroller.
   - `startup_msp432p401r_ccs.c`: Startup configuration file.

4. **Others**:
   - `msp432p401r.cmd`: Command file for memory configuration of the microcontroller.
   - `Drivers`: Directory containing library and driver files.
   - `Debug`: Directory containing compiled output files.

## How to Use

1. **Setup the Development Environment**:
   - Install [Code Composer Studio](https://www.ti.com/tool/CCSTUDIO).
   - Import the project using the `.ccsproject` file.

2. **Compile and Upload**:
   - Compile the project in Code Composer Studio.
   - Upload the compiled binary to the MSP432 microcontroller.

3. **Operation**:
   - Use the physical buttons to navigate through the menu and select tasks.
   - Monitor the LCD screen for real-time updates and feedback.
   - Observe the LEDs for alerts and system states.

## Link utili

  - [Presentazione progetto](https://docs.google.com/presentation/d/1akwyEOqc3FRNULJlvh5p67DcEXYNSdqN/edit?usp=sharing&ouid=109986732659211978447&rtpof=true&sd=true)

  - [Video progetto](https://youtu.be/KnO2sZ6FoGI)
