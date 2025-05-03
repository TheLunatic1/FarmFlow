Physical Connection List
ESP32 DevKitV1 Power Connections:
3.3V: Used to power all sensors.
Connect ESP32 3.3V pin to the breadboard’s positive rail.
GND: Common ground for all sensors.
Connect ESP32 GND pin to the breadboard’s negative rail.


Farmer 1, Field 1 Sensors:

BH1750 (Light Intensity, Address 0x23):
VCC: To breadboard positive rail (ESP32 3.3V).
GND: To breadboard negative rail (ESP32 GND).
SDA: To ESP32 GPIO 21.
SCL: To ESP32 GPIO 22.
ADDR: To breadboard negative rail (GND, sets address to 0x23).

DHT11 (Temperature and Humidity):
VCC (Pin 1): To breadboard positive rail (ESP32 3.3V).
Data (Pin 2): To ESP32 GPIO 25.
GND (Pin 4): To breadboard negative rail (ESP32 GND).
Pull-up Resistor: 4.7kΩ resistor between Data (Pin 2) and VCC (Pin 1).

Soil Moisture Sensor:
VCC: To breadboard positive rail (ESP32 3.3V).
GND: To breadboard negative rail (ESP32 GND).
AO (Analog Output): To ESP32 GPIO 33 (ADC1_CH5).
Probe: Insert into soil for Farmer 1, Field 1.


Farmer 2, Field 1 Sensors:

BH1750 (Light Intensity, Address 0x5C):
VCC: To breadboard positive rail (ESP32 3.3V).
GND: To breadboard negative rail (ESP32 GND).
SDA: To ESP32 GPIO 21 (shared I2C bus).
SCL: To ESP32 GPIO 22 (shared I2C bus).
ADDR: To breadboard positive rail (3.3V, sets address to 0x5C).

DHT11 (Temperature and Humidity):
VCC (Pin 1): To breadboard positive rail (ESP32 3.3V).
Data (Pin 2): To ESP32 GPIO 26.
GND (Pin 4): To breadboard negative rail (ESP32 GND).
Pull-up Resistor: 4.7kΩ resistor between Data (Pin 2) and VCC (Pin 1).

Soil Moisture Sensor:
VCC: To breadboard positive rail (ESP32 3.3V).
GND: To breadboard negative rail (ESP32 GND).
AO (Analog Output): To ESP32 GPIO 36 (ADC1_CH0).
Probe: Insert into soil for Farmer 2, Field 1.


Breadboard Connections:

Power Rails:

Positive rail: Connected to ESP32 3.3V.
Negative rail: Connected to ESP32 GND.

Shared I2C Bus:

SDA (GPIO 21) and SCL (GPIO 22) are shared by both BH1750 sensors. Connect both sensors’ SDA pins to GPIO 21 and SCL pins to GPIO 22 via the breadboard.
