/*
Nicla Sense ME BLE trainer code for Edge Impulse

@Author: Christopher Mendez
@Date: 02/02/2023 (mm/dd/yy)

@Brief:

This code let you upload data wirelessly from the Nicla Sense ME accelerometer to your Edge Impulse project
using the "Data forwarder"

@Note:
To successfully upload data to your project, you will need an ESP32 board with the "ESP32_Uploader_EI" code on it.
*/

#include "Nicla_System.h"  // Nicla library to be able to enable battery charging and LED control
#include "Arduino_BHY2.h"  // Arduino library to read data from every built-in sensor of the Nicla Sense ME
#include <ArduinoBLE.h>    // Bluetooth® Low Energy library for the Nicla Sense Me

// Bluetooth® Low Energy Service
BLEService myService("2d0a0000-e0f7-5fc8-b50f-05e267afeb67");

// Bluetooth® Low Energy Characteristic
BLEStringCharacteristic myChar("2d0a0001-e0f7-5fc8-b50f-05e267afeb67",  // standard 16-bit characteristic UUID
                               BLERead | BLENotify, 56);                // remote clients will be able to get notifications if this characteristic changes


long previousMillis = 0;  // last time the accelerometer data was sent, in ms

#define CONVERT_G_TO_MS2 9.80665f

SensorXYZ accel(SENSOR_ID_ACC);  // sensor class and ID declaration

void setup() {

  Serial.begin(115200);  // initialize serial communication

  pinMode(LED_BUILTIN, OUTPUT);  // initialize the built-in LED pin to indicate when a central is connected (BUILTIN = White LED)

  /* Init Nicla system and enable battery charging (100mA) */
  nicla::begin();
  nicla::enableCharge(100);

  /* Init & start sensors */
  BHY2.begin(NICLA_I2C);
  accel.begin();


  // Begin initialization
  if (!BLE.begin()) {  //BLE initialization
    Serial.println("starting BLE failed!");
    while (1)
      ;
  }

  /* Set a local name for the Bluetooth® Low Energy device
     This name will appear in advertising packets
     and can be used by remote devices to identify this Bluetooth® Low Energy device
     The name can be changed but maybe be truncated based on space left in advertisement packet
  */
  BLE.setLocalName("The_Protector");
  BLE.setAdvertisedService(myService);  // add the service UUID
  myService.addCharacteristic(myChar);  // add the custom characteristic
  BLE.addService(myService);            // Add the custom service

  /* Start advertising Bluetooth® Low Energy.  It will start continuously transmitting Bluetooth® Low Energy
     advertising packets and will be visible to remote Bluetooth® Low Energy central devices
     until it receives a new connection */

  // Start advertising
  BLE.advertise();

  Serial.println("Bluetooth® device active, waiting for connections...");
}

void loop() {
  // Wait for a Bluetooth® Low Energy central
  BLEDevice central = BLE.central();

  // If a central is connected to the peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // Print the central's BT address:
    Serial.println(central.address());
    // Turn on the LED to indicate the connection:
    digitalWrite(LED_BUILTIN, HIGH);

    // Send accelerometer data every 100ms
    // while the central is connected:
    while (central.connected()) {
      long currentMillis = millis();
      // If 100ms have passed, sample the sensor and send through BLE:
      if (currentMillis - previousMillis >= 100) {
        previousMillis = currentMillis;

        updateSensors();  // sending sensor data function
      }
    }
    // When the central disconnects, turn off the LED:
    digitalWrite(LED_BUILTIN, LOW);

    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}

void updateSensors() {

  BHY2.update();  // update the sensors

  /* Formatting the sensor data to be sent */
  String msg = String((accel.x() * 8.0 / 32768.0) * CONVERT_G_TO_MS2, 2) + "," + String((accel.y() * 8.0 / 32768.0) * CONVERT_G_TO_MS2, 2) + "," + String((accel.z() * 8.0 / 32768.0) * CONVERT_G_TO_MS2, 2);

  myChar.writeValue(msg);  // this is the sending command (msg = "acceX,acceY,acceZ")
}
