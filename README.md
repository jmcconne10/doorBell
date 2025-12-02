# ESP32 Gate Bell Alert System – Quick Wiring & Setup Guide

## 📘 Project Overview

This ESP32 project listens for **remote alerts** sent from a Firebase Realtime Database and triggers visual and audio notifications inside a robotics lab.

When the ESP32 detects an alert (`ALERT = "true"`), it:
- Turns on a large **NeoPixel LED strip** (300 LEDs) in bright red  
- Plays a **custom jingle** based on the user who triggered the alert  
- Falls back to a **buzzer alarm** if no matching jingle exists  
- Blinks the ESP32 built-in LED **rapidly** as a status indicator  

The device automatically:
- Detects whether it is at **school** or **home** and selects the correct WiFi network  
- Shows connection status via the built-in LED (**slow blink while searching**, **solid when connected**)  
- Monitors Firebase once per second and reacts instantly when a new alert is posted  

This creates a simple but reliable “gate bell” system where team members can post `!gate` in Discord and trigger lights + sounds inside the robotics lab.


## 📌 Pin Map (Matches Code)
| Feature        | Wire Connects To | ESP32 GPIO |
|----------------|------------------|------------|
| LED Strip Data | Green wire       | **GPIO 13 (D13)** |
| Buzzer +       | Brown wir        | **GPIO 12 (D12)** |
| Buzzer –       | White wire       | **GND** |
| LED Strip GND  | White wire       | **GND** |
| LED Strip +5V  | Red wire         | **5V Power Supply** |
| ESP32 Built-In LED | — | GPIO 2 |

> **IMPORTANT:** ESP32 GND and LED strip power supply GND **must** be connected together.

---

## ⚡ LED Strip Power Notes
- Power the LED strip directly from a **5V 2A+ supply**
- Do **NOT** power the strip from the ESP32 5V pin
- Optional but recommended: **330Ω resistor** in data line

---

## 🚦 Built-In LED Status
| LED Behavior | Meaning |
|--------------|---------|
| Slow blink | WiFi not connected |
| Solid ON | WiFi connected |
| Fast blink | **Alert active** |

---

## ▶️ Startup Instructions
1. Wire per diagram above  
2. Connect ESP32 via USB  
3. Load the provided `.ino` sketch  
4. Serial Monitor @ **115200 baud**  
5. When Firebase sets `ALERT = "true"`:
   - LED strip turns red  
   - Song or buzzer plays  
   - ESP32 built-in LED fast blinks  

---

## 🧪 Quick Test Checklist
- LED strip shows **green pixel #0** when connected to WiFi  
- Alerts make strip turn **red**  
- Buzzer sounds if no song found  
- Built-in LED blinks based on status  