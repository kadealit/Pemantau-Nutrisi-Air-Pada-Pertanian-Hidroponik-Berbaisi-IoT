# Pemantau-Nutrisi-Air-Pada-Pertanian-Hidroponik-Berbaisi-IoT
Pemantau Nutrisi Air Pada Pertanian Hidroponik dengan nodemcu esp8266
Alat dan Bahan
Microcontroller:

ESP8266 (misalnya NodeMCU)
Sensors:

TDS Sensor
Relay Module:

2 Relay module untuk mengontrol perangkat (misalnya, pompa air)
Breadboard dan Kabel Jumper:

Untuk koneksi sementara selama pengembangan
Power Supply:

Adaptor atau kabel USB untuk memberi daya pada ESP8266
Komponen Lainnya:

LED (opsional) untuk indikasi visual
Resistor 220 ohm (untuk LED)
Diagram Pinout
Berikut adalah diagram pinout untuk menghubungkan komponen-komponen tersebut:

ESP8266 NodeMCU:
TDS Sensor:

VCC ke 3.3V
GND ke GND
AOUT ke A0 (TdsSensorPin)
Relay 1:

VCC ke 3.3V
GND ke GND
IN1 ke D1 (RelayPin1)
Relay 2:

VCC ke 3.3V
GND ke GND
IN2 ke D2 (RelayPin2)
