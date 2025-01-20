#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Define our Servo
Servo servoMain;

// Define ultrasonic sensor pins
int trigpin = 12;
int echopin = 13;
int distance;
float duration;
float cm;

// Initialize the LCD with I2C address 0x27 (commonly used address)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Parking slot management
int parkingSlots = 3;

unsigned long previousMillis = 0;
const long interval = 2000; // 2 seconds delay

enum ServoState {
  SCAN,
  DETECTED,
  DELAY
};

ServoState servoState = SCAN;

void setup() {
    // Attach servo to pin 6
    servoMain.attach(6);

    // Set ultrasonic sensor pins
    pinMode(trigpin, OUTPUT);
    pinMode(echopin, INPUT);

    // Initialize the LCD and print the initial number of slots
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Parking Slots:");
    lcd.setCursor(0, 1);
    lcd.print(parkingSlots);
}

void loop() {
    switch (servoState) {
        case SCAN:
            scanAndMoveServo();
            break;
        case DETECTED:
            if (distance >= 10) {
                servoState = SCAN; // Reset state if object is out of range
            } else {
                unsigned long currentMillis = millis();
                if (currentMillis - previousMillis >= interval) {
                    servoState = DELAY; // Move to delay state after 2 seconds
                    previousMillis = currentMillis; // Record the time
                }
            }
            break;
        case DELAY:
            if (millis() - previousMillis >= interval) {
                servoMain.write(0); // Return servo to 0 degrees
                servoState = SCAN; // Return to scanning state
            }
            break;
    }
}

void scanAndMoveServo() {
    digitalWrite(trigpin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigpin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigpin, LOW);
    duration = pulseIn(echopin, HIGH);
    cm = (duration / 29.1) / 2; // Convert duration to cm

    if (cm != 0 && cm < 400) { // Check for valid readings
        distance = cm;
        if (distance < 10) {
            if (parkingSlots > 0) {
                servoMain.write(90); // Turn servo to 90 degrees if object is below 10 cm
                servoState = DETECTED; // Change state
                previousMillis = millis(); // Record the time
                
                // Update parking slots and display
                parkingSlots--;
                updateLCD();
            } else {
                // Display "Slots Filled" message
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Parking Slots:");
                lcd.setCursor(0, 1);
                lcd.print("Slots Filled");
            }
        }
    }
}

void updateLCD() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Parking Slots:");
    lcd.setCursor(0, 1);
    lcd.print(parkingSlots);
}
