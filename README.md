This repository contains the source code for an automated pet feeder powered by an ESP32 microcontroller. The system is designed to dispense pet food at scheduled times or manually, with control and monitoring capabilities provided through the Blynk mobile application.

## Features

*   **Automated Feeding Schedules:** Configure up to three unique feeding times directly from the Blynk app.
*   **Adjustable Portion Sizes:** Select from small, medium, or large portion sizes for each scheduled feeding.
*   **Remote & Manual Control:**
    *   Dispense food instantly using a button in the Blynk app.
    *   Trigger feeding with a physical push button.
*   **Real-Time Status Monitoring:**
    *   On-device status LEDs for power, WiFi connection, and feeding activity.
    *   Live status updates mirrored in the Blynk app.
    *   An I2C LCD screen displays current status like "CONNECTING...", "FEEDING...", and the current time.
*   **Emergency Stop:** Immediately halt all servo operations via a physical button or a switch in the app.
*   **Accurate Timekeeping:** Utilizes a DS1302 Real-Time Clock (RTC) module to maintain accurate time for scheduling, even after a power cycle.
*   **Feeding Log:** Automatically logs feeding events to a Google Sheet via a Google Apps Script web app for tracking.
*   **Audible Alerts:** A buzzer provides an audible signal when food is being dispensed.

## Hardware Requirements

*   **Microcontroller:** ESP32 Dev Board
*   **Actuator:** Servo Motor (e.g., SG90, MG996R)
*   **Display:** 16x2 I2C LCD Display
*   **Clock:** DS1302 RTC Module
*   **Input:** 2x Push Buttons (Start/Stop)
*   **Output:**
    *   3x LEDs (Power, WiFi, Feed Status)
    *   1x Buzzer
*   **Power Supply:** Appropriate power supply for the ESP32 and servo.

## Pinout

| Component          | ESP32 Pin      |
| ------------------ | -------------- |
| Onboard LED        | `GPIO 2`       |
| Status LED         | `GPIO 4`       |
| WiFi LED           | `GPIO 5`       |
| RTC Data (IO)      | `GPIO 12`      |
| RTC Clock (SCLK)   | `GPIO 27`      |
| RTC Chip Enable (CE) | `GPIO 13`      |
| Start Button       | `GPIO 18`      |
| Stop Button        | `GPIO 19`      |
| I2C SDA (LCD)      | `GPIO 21`      |
| I2C SCL (LCD)      | `GPIO 22`      |
| Buzzer             | `GPIO 23`      |
| Servo Signal       | `GPIO 26`      |

## Software & Libraries

This project is built using PlatformIO with the Arduino framework.

*   [Blynk](https://github.com/blynkkk/blynk-library)
*   [ESP32Servo](https://github.com/madhephaestus/ESP32Servo)
*   [LiquidCrystal_I2C](https://github.com/johnrickman/LiquidCrystal_I2C)
*   [Rtc by Makuna](https://github.com/Makuna/Rtc) (for DS1302)
*   `WiFi.h`
*   `HTTPClient.h`

## Setup & Configuration

1.  **Clone the Repository:**
    ```bash
    git clone https://github.com/yourshoji/myESPFeeder.git
    cd myESPFeeder
    ```

2.  **Install PlatformIO:** Ensure you have the [PlatformIO IDE extension](https://platformio.org/platformio-ide) for VSCode installed.

3.  **Set up Event Logger:** Create a new Google Sheet, set up Apps Script, and put `Code.gs` (sheet-log folder) it. Hit `Deploy` and use the given URL in the following step.

4.  **Configure Credentials:** Open `src/PetFeeder_Blynk.ino` and update the following values with your own credentials:

    ```cpp
    // Blynk Credentials
    #define BLYNK_TEMPLATE_ID "YOUR_TEMPLATE_ID"
    #define BLYNK_TEMPLATE_NAME "YOUR_TEMPLATE_NAME"
    #define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN"

    // WiFi Credentials
    const char* ssid = "YOUR_WIFI_SSID";
    const char* pass = "YOUR_WIFI_PASSWORD";

    // Google Apps Script URL for logging
    const char* webApp = "https://script.google.com/macros/s/YOUR_SCRIPT_ID/exec";
    ```
    
5.  **Set RTC Time (First Use):** For the initial upload, you need to set the time on the DS1302 module. In `src/PetFeeder_Blynk.ino`, find the `setup()` function and uncomment these two lines:
    ```cpp
    // RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    // Rtc.SetDateTime(compiled);
    ```
    Upload the code to your ESP32. After the first successful upload, **comment these lines out again** and re-upload to prevent the RTC from being reset on every boot.

6.  **Build and Upload:** Use the PlatformIO interface to build and upload the project to your ESP32.

## Blynk App Configuration

Set up your Blynk project dashboard with the following widgets:

| Virtual Pin | Widget                   | Function                                           |
| ----------- | ------------------------ | -------------------------------------------------- |
| `V0`        | LED                      | Power Status Indicator                             |
| `V1`        | LED                      | Feeding Status Indicator                           |
| `V2`        | LED                      | WiFi Connection Status                             |
| `V3`        | Button                   | Manual Feed Trigger                                |
| `V4`        | Time Input               | Set time for Schedule 1                            |
| `V5`        | Time Input               | Set time for Schedule 2                            |
| `V6`        | Time Input               | Set time for Schedule 3                            |
| `V7`-`V9`   | LED                      | Indicates if Schedule 1, 2, or 3 has been dispensed|
| `V10`       | Button                   | Reset all schedules in the device's memory         |
| `V11`       | Button                   | Display current schedule details in the V12 terminal|
| `V12`       | Terminal                 | Displays schedule information                      |
| `V13`       | Segmented Switch         | Select portion size (S/M/L) for Schedule 1         |
| `V14`       | Segmented Switch         | Select portion size (S/M/L) for Schedule 2         |
| `V15`       | Segmented Switch         | Select portion size (S/M/L) for Schedule 3         |
| `V16`       | Button / Switch          | Emergency Stop                                     |

## Screenshots
<table width="100%">
  <tr>
    <td align="center" width="33%"><strong>Prototype</strong></td>
    <td align="center" width="33%"><strong>Blynk</strong></td>
    <td align="center" width="33%"><strong>Google Sheet Logging</strong></td>
  </tr>
  <tr>
    <td><img src="https://github.com/yourshoji/myESPFeeder/blob/main/img/model.jpg" alt="Prototype" width="100%"></td>
    <td><img src="https://github.com/yourshoji/myESPFeeder/blob/main/img/blynk.jpg" alt="Blynk" width="100%"></td>
    <td><img src="https://github.com/yourshoji/myESPFeeder/blob/main/img/logger.png" alt="Logger" width="100%"></td>
  </tr>
  
  <tr>
    <td align="center"><strong>Angle 1</strong></td>
    <td align="center"><strong>Angle 2</strong></td>
    <td align="center"><strong>Angle 3</strong></td>
  </tr>
  <tr>
    <td><img src="https://github.com/yourshoji/myESPFeeder/blob/main/img/compo3.jpg" alt="Angle1" width="100%"></td>
    <td><img src="https://github.com/yourshoji/myESPFeeder/blob/main/img/compo2.jpg" alt="Angle2" width="100%"></td>
    <td><img src="https://github.com/yourshoji/myESPFeeder/blob/main/img/compo1.jpg" alt="Angle3" width="100%"></td>
  </tr>

  <tr>
    <td align="center" colspan="3"><strong>Drawing</strong></td>
  </tr>
  <tr>
    <td align="center" colspan="3">
      <img src="https://github.com/yourshoji/myESPFeeder/blob/main/img/feeder.png" alt="Scheme" width="33%">
    </td>
  </tr>
</table>
