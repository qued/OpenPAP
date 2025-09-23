# OpenPAP

A fork of OpenPAP (original [here](https://github.com/keredson/openpap)), an open hardware, open source CPAP / BiPAP solution than can be build for about **$120**.

![image](https://github.com/user-attachments/assets/87150fc5-7136-4654-b9ad-bc46e850a8fe)

## Changes from original project
I wanted to make this machine more end-user friendly. To that end, I've utilized a rotary encoder interface to allow the user to configure the machine via a menu system. Currently the menu system supports the following:
* Starting/stopping the therapy cycle (therapy cycle now shows a graph)
* Testing the motor with manual throttle control
* Testing the pressure sensor
* Calibrating the ESC
* Auto tuning the PID parameters system using IMC

I have removed the OTA firmware updates / PID parameter changes for the time being.

## Disclaimer

**WARNING: This project is not approved for medical use.**

This project is provided **as-is**, **without any guarantees** or **warranties of any kind**, express or implied. It is an **experimental, educational, and non-commercial** open-source project intended to explore the design and functionality of CPAP systems.

By using, modifying, or building this project, you acknowledge and agree to the following:

- This project is **not certified** by any regulatory authority (e.g. FDA, CE, ISO).
- It is **not intended to diagnose, treat, cure, or prevent any disease**.
- It **must not be used as a substitute** for a commercially approved CPAP or BiPAP device.
- **Use at your own risk**. The creators and contributors are **not responsible for any harm, injury, or loss** resulting from the use or misuse of this hardware, firmware, or software.
- This device has **not undergone clinical testing** or safety validation.
- No support or liability is implied or offered for any applications of this project in **real-world medical or therapeutic contexts**.

If you are in need of CPAP therapy or respiratory assistance, please consult a qualified medical professional and use approved commercial devices.

## Current Status

This is a WIP, although from my testing, the firmware is in a working state.

## Hardware
- Resmed motor ([example](https://www.amazon.com/gp/product/B0CHYMMM8G))
- 5A 12v to 24v power supply (higher voltages support higher pressures - [example 12v](https://www.amazon.com/gp/product/B08C594VNP) / [example 24v](https://www.amazon.com/ALITOVE-100-240V-Adapter-Converter-5-5x2-1mm/dp/B01GC6VS8I))
- 4S to 6S ESC (brushless motor driver, depending on voltage - [example](https://www.amazon.com/RC-Brushless-Electric-Controller-bullet/dp/B071GRSFBD))
- ESP32 microcontroller ([example](https://www.amazon.com/ESP-WROOM-32-Development-Microcontroller-Integrated-Compatible/dp/B08D5ZD528))
- Air pressure sensor ([example](https://www.amazon.com/gp/product/B09V5BDFF6))
- KY 040 Rotary encoder ([example](https://www.amazon.com/JTAREA-KY-040-Encoder-Encoders-Modules/dp/B0D2TTG858))
- SSD1306 OLED 128x64 Display ([example](https://www.amazon.com/Hosyond-Display-Self-Luminous-Compatible-Raspberry/dp/B09T6SJBV5))
