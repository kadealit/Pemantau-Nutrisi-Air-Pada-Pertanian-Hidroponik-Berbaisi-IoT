#define BLYNK_TEMPLATE_ID "TMPL6icpoQPr5"  // Ganti dengan Template ID Anda
#define BLYNK_TEMPLATE_NAME "tds"          // Ganti dengan Template Name Anda
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#define TdsSensorPin A0
#define RelayPin1 D1            // Relay 1 control pin
#define RelayPin2 D2            // Relay 2 control pin
#define VREF 3.3               // analog reference voltage(Volt) of the ADC
#define SCOUNT  30             // sum of sample point

char auth[] = "-rb4-sXQqCBlJCnL9xcSepJDbLekJ4te"; // Ganti dengan token Blynk Anda
char ssid[] = "cipuk";                   // Ganti dengan SSID WiFi Anda
char pass[] = "12345678";               // Ganti dengan kata sandi WiFi Anda

int analogBuffer[SCOUNT];     // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;

float averageVoltage = 0;
float tdsValue = 0;
float temperature = 23;       // current temperature for compensation

int relayState1 = HIGH;         // initial state of relay 1 (relay off)
bool manualControl1 = false;   // flag to indicate manual control from Blynk button
int relayState2 = HIGH;         // initial state of relay 2 (relay off)
bool manualControl2 = false;   // flag to indicate manual control from Blynk button
unsigned long relay2Interval = 1000; // interval in milliseconds
unsigned long previousMillis = 0;   // to store the last time relay2 was updated

WidgetTerminal terminal(V4); // Widget Terminal pada pin V4
WidgetLED led1(V5); // Widget LED untuk Relay 1 pada pin V5
WidgetLED led2(V6); // Widget LED untuk Relay 2 pada pin V6

// median filtering algorithm
int getMedianNum(int bArray[], int iFilterLen) {
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++)
    bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0) {
    bTemp = bTab[(iFilterLen - 1) / 2];
  } else {
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  }
  return bTemp;
}

BLYNK_WRITE(V1) {
  int buttonState = param.asInt(); // Get button value from Blynk app
  manualControl1 = true; // Set manual control flag

  if (buttonState == 1) {
    relayState1 = LOW;  // Relay ON (active LOW)
    Serial.println("Relay 1 ON (from button)");
    terminal.println("Relay 1 ON (from button)");
    led1.on();
  } else {
    relayState1 = HIGH; // Relay OFF (inactive HIGH)
    Serial.println("Relay 1 OFF (from button)");
    terminal.println("Relay 1 OFF (from button)");
    led1.off();
  }
  digitalWrite(RelayPin1, relayState1); // Set relay state
  terminal.flush(); // Send data to Terminal
}

BLYNK_WRITE(V2) {
  int buttonState = param.asInt(); // Get button value from Blynk app
  manualControl2 = true; // Set manual control flag

  if (buttonState == 1) {
    relayState2 = LOW;  // Relay ON (active LOW)
    Serial.println("Relay 2 ON (from button)");
    terminal.println("Relay 2 ON (from button)");
    led2.on();
  } else {
    relayState2 = HIGH; // Relay OFF (inactive HIGH)
    Serial.println("Relay 2 OFF (from button)");
    terminal.println("Relay 2 OFF (from button)");
    led2.off();
  }
  digitalWrite(RelayPin2, relayState2); // Set relay state
  terminal.flush(); // Send data to Terminal
}

BLYNK_WRITE(V3) {
  relay2Interval = param.asInt() * 1000; // Get interval value from Blynk app and convert to milliseconds
  Serial.print("Relay 2 interval set to ");
  Serial.print(relay2Interval);
  Serial.println(" milliseconds");
  terminal.print("Relay 2 interval set to ");
  terminal.print(relay2Interval);
  terminal.println(" milliseconds");
  terminal.flush(); // Send data to Terminal
}

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);
  pinMode(TdsSensorPin, INPUT);
  pinMode(RelayPin1, OUTPUT);  // Set relay 1 pin as output
  pinMode(RelayPin2, OUTPUT);  // Set relay 2 pin as output
  digitalWrite(RelayPin1, relayState1); // Ensure relay 1 is off at start
  digitalWrite(RelayPin2, relayState2); // Ensure relay 2 is off at start

  terminal.println("System started");
  terminal.flush();
}

void loop() {
  Blynk.run();

  static unsigned long analogSampleTimepoint = millis();
  if (millis() - analogSampleTimepoint > 40U) {  // every 40 milliseconds, read the analog value from the ADC
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);  // read the analog value and store into the buffer
    analogBufferIndex++;
    if (analogBufferIndex == SCOUNT) {
      analogBufferIndex = 0;
    }
  }

  static unsigned long printTimepoint = millis();
  if (millis() - printTimepoint > 800U) {
    printTimepoint = millis();
    for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++) {
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
    }

    // read the analog value more stable by the median filtering algorithm, and convert to voltage value
    averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 1024.0;

    // temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
    float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
    // temperature compensation
    float compensationVoltage = averageVoltage / compensationCoefficient;

    // convert voltage value to TDS value
    tdsValue = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage 
               - 255.86 * compensationVoltage * compensationVoltage 
               + 857.39 * compensationVoltage) * 0.5;

    // Print voltage and TDS value
    Serial.print("Voltage: ");
    Serial.print(averageVoltage, 2);
    Serial.print("V   ");
    Serial.print("TDS Value: ");
    Serial.print(tdsValue, 0);
    Serial.println("ppm");

    terminal.print("Voltage: ");
    terminal.print(averageVoltage, 2);
    terminal.print("V   ");
    terminal.print("TDS Value: ");
    terminal.print(tdsValue, 0);
    terminal.println("ppm");

    // Send TDS value to Blynk
    Blynk.virtualWrite(V0, tdsValue);

    // Determine water clarity based on TDS value and update relay if not in manual control
    if (!manualControl1) {
      if (tdsValue < 50) {
        Serial.println("Water clarity: Very Clear");
        terminal.println("Water clarity: Very Clear");
        if (relayState1 != HIGH) {
          relayState1 = HIGH;
          digitalWrite(RelayPin1, relayState1);  // Turn off relay 1
          Serial.println("Relay 1 OFF (due to water clarity)");
          terminal.println("Relay 1 OFF (due to water clarity)");
          led1.off();
        }
      } else if (tdsValue >= 50 && tdsValue < 200) {
        Serial.println("Water clarity: Clear");
        terminal.println("Water clarity: Clear");
        if (relayState1 != HIGH) {
          relayState1 = HIGH;
          digitalWrite(RelayPin1, relayState1);  // Turn off relay 1
          Serial.println("Relay 1 OFF (due to water clarity)");
          terminal.println("Relay 1 OFF (due to water clarity)");
          led1.off();
        }
      } else if (tdsValue >= 200 && tdsValue < 500) {
        Serial.println("Water clarity: Slightly Turbid");
        terminal.println("Water clarity: Slightly Turbid");
        if (relayState1 != LOW) {
          relayState1 = LOW;
          digitalWrite(RelayPin1, relayState1);  // Turn on relay 1
          Serial.println("Relay 1 ON (due to water clarity)");
          terminal.println("Relay 1 ON (due to water clarity)");
          led1.on();
        }
      } else {
        Serial.println("Water clarity: Turbid");
        terminal.println("Water clarity: Turbid");
        if (relayState1 != LOW) {
          relayState1 = LOW;
          digitalWrite(RelayPin1, relayState1);  // Turn on relay 1
          Serial.println("Relay 1 ON (due to water clarity)");
          terminal.println("Relay 1 ON (due to water clarity)");
          led1.on();
        }
      }
    }
    terminal.flush(); // Send data to Terminal
  }

  if (!manualControl2) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= relay2Interval) {
      previousMillis = currentMillis;
      if (relayState2 == HIGH) {
        relayState2 = LOW;  // Relay ON (active LOW)
        led2.on();
      } else {
        relayState2 = HIGH; // Relay OFF (inactive HIGH)
        led2.off();
      }
      digitalWrite(RelayPin2, relayState2); // Update relay state
      Serial.print("Relay 2 state toggled to ");
      Serial.println(relayState2 == LOW ? "ON" : "OFF");
      terminal.print("Relay 2 state toggled to ");
      terminal.println(relayState2 == LOW ? "ON" : "OFF");
    }
  }
  terminal.flush(); // Send data to Terminal
}