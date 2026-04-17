# AuraLink System (VibeAlert)

> **IoT-Based Multi-Modal Alert System for Hearing Impaired Individuals**

---

## Project Overview

The **AuraLink System** (also referred to as **VibeAlert**) is an assistive Internet of Things (IoT) technology designed to help hearing-impaired individuals detect and respond to important environmental sounds. By actively monitoring ambient noise and translating it into **visual**, **haptic (vibration)**, and **digital feedback**, this system ensures that users never miss critical environmental cues.

---

## The Problem

Deaf and hard-of-hearing individuals face daily challenges in detecting sounds that are vital for safety, security, and independence. Missing an emergency alarm, a doorbell ring, or even routine household sounds can lead to significant safety risks and reduced social interaction.

**Commercial solutions today are:**
- Prohibitively expensive — typically **$300–$500 per unit**
- Lacking in customization capabilities
- Without remote monitoring or modern mobile app integration

---

## The Solution

AuraLink bridges the accessibility gap by providing an **affordable**, **customizable**, and **highly integrated** alert system.

| Feature | Detail |
|---|---|
| **Cost-Effective** | Entire hardware implementation costs **under $100** |
| **Multi-Modal Feedback** | Simultaneously uses vibration patterns, colour-coded LEDs, LCD display, and mobile push notifications |
| **Highly Customizable** | Users can adjust sound sensitivity and vibration intensity via a mobile app |

---

## Who Is This For?

This system is built specifically for **Deaf and Hard of Hearing individuals**. However, its core technology is highly adaptable and serves as a blueprint for creating accessible smart-home devices that prioritise **user independence and safety**.

---

## Core Features & Alert Levels

The system continuously monitors ambient sound and categorises it into **three distinct alert levels**:

| Level | Sound Intensity | LED Color | LCD Message | Vibration Pulses |
|---|---|---|---|---|
| 🚨 **Emergency** | 60–100% | 🔴 Red | `EMERGENCY!` | 5 pulses |
| 🔔 **Doorbell** | 40–60% | 🔵 Blue | Doorbell alert | 2 pulses |
| 🔉 **Routine Sound** | 25–40% | 🟡 Yellow | Routine update | 1 pulse |

> **Manual Test Buttons** are also included, allowing users to verify each alert type without requiring actual sound input.

---

## Technical Architecture

### Hardware Components

| Component | Details |
|---|---|
| **Microcontroller** | ESP32 Development Board (Dual-core, WiFi/Bluetooth enabled) |
| **Sound Detection** | Analog Sound Sensor Module |
| **Haptic Feedback** | PWM-controlled 3V Vibration Motor |
| **Visual Indicators** | RGB LED |
| **Display** | 16×2 I2C LCD Display |
| **Audible Alert** | Active Buzzer |

### Software & Cloud Stack

| Layer | Technology |
|---|---|
| **IDE** | Arduino IDE |
| **Key Libraries** | `BlynkSimpleEsp32`, `WiFi.h`, `LiquidCrystal_I2C` |
| **Cloud Platform** | Blynk IoT Platform |
| **Communication Protocol** | WiFi + MQTT |
| **Simulation & Testing** | [Wokwi Simulator](https://wokwi.com) |

---

## Getting Started

### Prerequisites

- ESP32 Development Board
- Arduino IDE installed
- Blynk account and project token
- Required hardware components (see [Technical Architecture](#-technical-architecture))

### Installation

1. **Clone the repository**
   ```bash
   git clone https://github.com/your-username/auralink-system.git
   cd auralink-system
   ```

2. **Install Arduino Libraries**
   - Open Arduino IDE → *Sketch* → *Include Library* → *Manage Libraries*
   - Search and install: `Blynk`, `LiquidCrystal I2C`

3. **Configure credentials**
   ```cpp
   // In src/main.ino, update the following:
   #define BLYNK_AUTH_TOKEN "your_token_here"
   char ssid[] = "your_wifi_ssid";
   char pass[] = "your_wifi_password";
   ```

4. **Upload to ESP32**
   - Select board: *ESP32 Dev Module*
   - Upload the sketch

---

## Impact Summary

The AuraLink System is more than just a technical prototype — it is a **practical application of IoT technology** aimed at solving a real-world accessibility problem. By combining affordable hardware with cloud-based mobile technology, this project demonstrates how engineering can directly improve the **daily lives, safety, and autonomy** of hearing-impaired individuals.

---

