/*
 * VibeAlert - Smart Alert System for Deaf/Hard of Hearing
 * Simulation Version with Blynk Integration
 * 
 * IMPORTANT: In Wokwi, save this file as "sketch.ino" NOT "main.c"
 * 
 * Features:
 * - 3 Sound Classification (Emergency, Doorbell, Routine)
 * - Haptic Feedback via Vibration Motor
 * - Visual Alerts via LCD Display
 * - Blynk Mobile App Integration
 * - LED Indicators
 * 
 * Hardware Components (Simulated):
 * - ESP32 Dev Board
 * - Sound Sensor (simulated with potentiometer)
 * - Vibration Motor
 * - 16x2 LCD Display (I2C)
 * - RGB LED
 * - Push Buttons (for testing different sounds)
 */

// ==================== BLYNK CREDENTIALS (MUST BE FIRST) ====================
#define BLYNK_TEMPLATE_ID "TMPL3wgj2bqze"
#define BLYNK_TEMPLATE_NAME "VibeAlert"
#define BLYNK_AUTH_TOKEN "ImyGZ-WplRAWottFBLFsIOeIYpU2D3Nr"

// ==================== LIBRARIES ====================
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// WiFi credentials
char ssid[] = "Wokwi-GUEST";     // For Wokwi simulation
char pass[] = "";                 // No password for Wokwi

// ==================== PIN DEFINITIONS ====================
// Sound Sensor (simulated with potentiometer)
#define SOUND_SENSOR_PIN 34       // ADC1 pin for analog input

// Vibration Motor
#define VIBRATION_MOTOR_PIN 25    // PWM capable pin

// RGB LED Pins
#define LED_RED_PIN 26
#define LED_GREEN_PIN 27
#define LED_BLUE_PIN 14

// Button Inputs (for simulating different sounds)
#define BTN_EMERGENCY 32          // Simulate emergency sound
#define BTN_DOORBELL 33           // Simulate doorbell
#define BTN_ROUTINE 35            // Simulate routine alert

// Buzzer (optional for testing)
#define BUZZER_PIN 15

// ==================== LCD CONFIGURATION ====================
LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C address 0x27, 16x2 display

// ==================== BLYNK VIRTUAL PINS ====================
#define VPIN_ALERT_STATUS V0      // Alert status display
#define VPIN_SOUND_LEVEL V1       // Sound level gauge
#define VPIN_LAST_ALERT V2        // Last alert type
#define VPIN_EMERGENCY_LED V3     // Emergency indicator LED
#define VPIN_SYSTEM_STATUS V4     // System status
#define VPIN_SENSITIVITY V5       // Sensitivity slider
#define VPIN_VIBRATION_STRENGTH V6 // Vibration strength slider
#define VPIN_MANUAL_TEST V7       // Manual test button
#define VPIN_ALERT_COUNT V8

// ==================== SOUND CLASSIFICATION ====================
enum SoundClass {
  NONE = 0,
  EMERGENCY = 1,      // Smoke alarm, CO detector, glass breaking
  DOORBELL = 2,       // Doorbell, phone ring
  ROUTINE = 3         // Microwave, timer, water boiling
};

// Threshold values for classification
#define THRESHOLD_LOW 1500
#define THRESHOLD_MED 2500
#define THRESHOLD_HIGH 3500

// ==================== GLOBAL VARIABLES ====================
int soundLevel = 0;
SoundClass currentSound = NONE;
String lastAlertType = "None";
int alertCount = 0;
unsigned long lastAlertTime = 0;
bool isAlerting = false;

// User adjustable settings
int sensitivity = 50;           // 0-100%
int vibrationStrength = 75;     // 0-100%

// Haptic patterns (duration in ms)
struct HapticPattern {
  int onTime;
  int offTime;
  int repeats;
};

HapticPattern emergencyPattern = {100, 100, 5};   // Fast SOS
HapticPattern doorbellPattern = {300, 200, 2};    // Friendly double
HapticPattern routinePattern = {500, 0, 1};       // Single long

// ==================== TIMER FOR BLYNK ====================
BlynkTimer timer;

// ==================== SETUP FUNCTION ====================
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== VibeAlert System Starting ===");
  
  // Initialize pins
  pinMode(SOUND_SENSOR_PIN, INPUT);
  pinMode(VIBRATION_MOTOR_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);
  pinMode(BTN_EMERGENCY, INPUT_PULLUP);
  pinMode(BTN_DOORBELL, INPUT_PULLUP);
  pinMode(BTN_ROUTINE, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("VibeAlert v1.0");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  
  // Set initial LED state (green = ready)
  setRGBColor(0, 255, 0);
  
  delay(2000);
  
  // Connect to WiFi
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");
  Serial.println("Connecting to WiFi...");
  
  WiFi.begin(ssid, pass);
  int wifiAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && wifiAttempts < 20) {
    delay(500);
    Serial.print(".");
    lcd.setCursor(wifiAttempts % 16, 1);
    lcd.print(".");
    wifiAttempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    
    // Connect to Blynk
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connecting Blynk");
    
    Blynk.config(BLYNK_AUTH_TOKEN);
    Blynk.connect();
    
    delay(1000);
  }
  
  // Setup timers
  timer.setInterval(1000L, sendSensorData);    // Send data every 1 sec
  timer.setInterval(100L, checkSoundInput);     // Check sound every 100ms
  
  // Final setup
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready!");
  setRGBColor(0, 255, 0);  // Green = ready
  delay(1000);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Monitoring...");
  
  Serial.println("=== System Ready ===");
}

// ==================== MAIN LOOP ====================
void loop() {
  Blynk.run();
  timer.run();
  
  // Check manual button inputs
  checkButtonInputs();
  
  // Update display if not alerting
  if (!isAlerting) {
    updateIdleDisplay();
  }
}

// ==================== SOUND INPUT CHECKING ====================
void checkSoundInput() {
  // Read analog sound level
  soundLevel = analogRead(SOUND_SENSOR_PIN);
  
  // Apply sensitivity adjustment
  int adjustedThreshold = map(sensitivity, 0, 100, THRESHOLD_HIGH, THRESHOLD_LOW);
  
  // Classify sound based on level
  if (soundLevel > adjustedThreshold) {
    if (soundLevel > THRESHOLD_HIGH) {
      currentSound = EMERGENCY;
    } else if (soundLevel > THRESHOLD_MED) {
      currentSound = DOORBELL;
    } else if (soundLevel > THRESHOLD_LOW) {
      currentSound = ROUTINE;
    }
    
    // Trigger alert if new sound detected
    if (currentSound != NONE && !isAlerting) {
      triggerAlert(currentSound);
    }
  }
}

// ==================== BUTTON INPUT CHECKING ====================
void checkButtonInputs() {
  // Emergency button
  if (digitalRead(BTN_EMERGENCY) == LOW) {
    delay(50); // Debounce
    if (digitalRead(BTN_EMERGENCY) == LOW) {
      Serial.println("Emergency button pressed!");
      triggerAlert(EMERGENCY);
      while(digitalRead(BTN_EMERGENCY) == LOW); // Wait for release
    }
  }
  
  // Doorbell button
  if (digitalRead(BTN_DOORBELL) == LOW) {
    delay(50);
    if (digitalRead(BTN_DOORBELL) == LOW) {
      Serial.println("Doorbell button pressed!");
      triggerAlert(DOORBELL);
      while(digitalRead(BTN_DOORBELL) == LOW);
    }
  }
  
  // Routine button
  if (digitalRead(BTN_ROUTINE) == LOW) {
    delay(50);
    if (digitalRead(BTN_ROUTINE) == LOW) {
      Serial.println("Routine button pressed!");
      triggerAlert(ROUTINE);
      while(digitalRead(BTN_ROUTINE) == LOW);
    }
  }
}

// ==================== ALERT TRIGGERING ====================
void triggerAlert(SoundClass sound) {
  isAlerting = true;
  alertCount++;
  lastAlertTime = millis();
  
  String alertType;
  HapticPattern pattern;
  
  // Set alert parameters based on sound class
  switch(sound) {
    case EMERGENCY:
      alertType = "EMERGENCY!";
      pattern = emergencyPattern;
      setRGBColor(255, 0, 0);  // Red
      lastAlertType = "Emergency";
      Blynk.virtualWrite(VPIN_EMERGENCY_LED, 255);
      Blynk.logEvent("emergency_alert", "Emergency sound detected!");
      break;
      
    case DOORBELL:
      alertType = "Doorbell";
      pattern = doorbellPattern;
      setRGBColor(0, 0, 255);  // Blue
      lastAlertType = "Doorbell";
      Blynk.virtualWrite(VPIN_EMERGENCY_LED, 0);
      Blynk.logEvent("doorbell_alert", "Someone at the door!");
      break;
      
    case ROUTINE:
      alertType = "Routine Alert";
      pattern = routinePattern;
      setRGBColor(255, 255, 0);  // Yellow
      lastAlertType = "Routine";
      Blynk.virtualWrite(VPIN_EMERGENCY_LED, 0);
      break;
      
    default:
      isAlerting = false;
      return;
  }
  
  // Update LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ALERT!");
  lcd.setCursor(0, 1);
  lcd.print(alertType);
  
  // Send to Blynk
  Blynk.virtualWrite(VPIN_ALERT_STATUS, alertType);
  Blynk.virtualWrite(VPIN_LAST_ALERT, alertType);
  Blynk.virtualWrite(VPIN_ALERT_COUNT, alertCount);
  
  // Execute haptic pattern
  executeHapticPattern(pattern);
  
  // Play buzzer pattern (optional)
  playBuzzerPattern(sound);
  
  Serial.println("Alert triggered: " + alertType);
  Serial.println("Alert count: " + String(alertCount));
  
  // Keep alert display for 3 seconds
  delay(3000);
  
  // Return to normal state
  setRGBColor(0, 255, 0);  // Green
  Blynk.virtualWrite(VPIN_EMERGENCY_LED, 0);
  isAlerting = false;
  currentSound = NONE;
}

// ==================== HAPTIC PATTERN EXECUTION ====================
void executeHapticPattern(HapticPattern pattern) {
  int strength = map(vibrationStrength, 0, 100, 0, 255);
  
  for (int i = 0; i < pattern.repeats; i++) {
    // Motor ON
    analogWrite(VIBRATION_MOTOR_PIN, strength);
    delay(pattern.onTime);
    
    // Motor OFF
    analogWrite(VIBRATION_MOTOR_PIN, 0);
    if (pattern.offTime > 0) {
      delay(pattern.offTime);
    }
  }
  
  // Ensure motor is off
  analogWrite(VIBRATION_MOTOR_PIN, 0);
}

// ==================== BUZZER PATTERN ====================
void playBuzzerPattern(SoundClass sound) {
  switch(sound) {
    case EMERGENCY:
      for (int i = 0; i < 3; i++) {
        tone(BUZZER_PIN, 1000, 200);
        delay(300);
      }
      break;
      
    case DOORBELL:
      tone(BUZZER_PIN, 800, 200);
      delay(300);
      tone(BUZZER_PIN, 800, 200);
      break;
      
    case ROUTINE:
      tone(BUZZER_PIN, 600, 500);
      break;
  }
  noTone(BUZZER_PIN);
}

// ==================== RGB LED CONTROL ====================
void setRGBColor(int red, int green, int blue) {
  analogWrite(LED_RED_PIN, red);
  analogWrite(LED_GREEN_PIN, green);
  analogWrite(LED_BLUE_PIN, blue);
}

// ==================== IDLE DISPLAY UPDATE ====================
void updateIdleDisplay() {
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 2000) {  // Update every 2 seconds
    lcd.setCursor(0, 0);
    lcd.print("Monitoring...   ");
    lcd.setCursor(0, 1);
    lcd.print("Alerts: ");
    lcd.print(alertCount);
    lcd.print("       ");
    lastUpdate = millis();
  }
}

// ==================== SEND SENSOR DATA TO BLYNK ====================
void sendSensorData() {
  Blynk.virtualWrite(VPIN_SOUND_LEVEL, map(soundLevel, 0, 4095, 0, 100));
  Blynk.virtualWrite(VPIN_SYSTEM_STATUS, "Online");
  
  if (WiFi.status() == WL_CONNECTED) {
    Blynk.virtualWrite(VPIN_SYSTEM_STATUS, "Connected");
  }
}

// ==================== BLYNK INPUT HANDLERS ====================

// Sensitivity slider
BLYNK_WRITE(VPIN_SENSITIVITY) {
  sensitivity = param.asInt();
  Serial.println("Sensitivity set to: " + String(sensitivity) + "%");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sensitivity:");
  lcd.setCursor(0, 1);
  lcd.print(String(sensitivity) + "%");
  delay(1500);
}

// Vibration strength slider
BLYNK_WRITE(VPIN_VIBRATION_STRENGTH) {
  vibrationStrength = param.asInt();
  Serial.println("Vibration strength set to: " + String(vibrationStrength) + "%");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Vibration:");
  lcd.setCursor(0, 1);
  lcd.print(String(vibrationStrength) + "%");
  delay(1500);
}

// Manual test button
BLYNK_WRITE(VPIN_MANUAL_TEST) {
  int buttonState = param.asInt();
  if (buttonState == 1) {
    Serial.println("Manual test triggered from Blynk");
    triggerAlert(DOORBELL);
  }
}

// ==================== BLYNK CONNECTED ====================
BLYNK_CONNECTED() {
  Serial.println("Blynk connected!");
  Blynk.syncAll();  // Sync all virtual pins
  Blynk.virtualWrite(VPIN_SYSTEM_STATUS, "Connected");
  Blynk.virtualWrite(VPIN_ALERT_COUNT, alertCount);
}
