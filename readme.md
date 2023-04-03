Project for learning MQTT, NodeRed, influxDB etc. using Raspberry Pi 4, ESP32 and Arduino IDE.

Plan:

Raspberry Pi 4:
- MQTT-broker
- Node-RED
  - Upload data to IBM-cloud
  - Dashboard
    - Temperature-graph
    - Humidity-gauge
    - LED ON/OFF
    - Servo-motor slider
    - Camera feed from Raspi4 camera    
  - MQTT-publisher
  - MQTT-subscriber
  - Send alerts using WhatsApp
- influxDB
- Grafana

ESP32:
- DHT11-sensor
- Servomotor
- LED
- MQTT-publisher
- MQTT-subscriber
- Ultrasonic-sensor
- Light-sensor

Working now:

Raspi4: MQTT-broker, MQTT-publisher, MQTT-subscriber, Node-RED (IBM-Cloud, Dashboard (no camera feed), MQTT influxDB, Grafana

ESP32: DHT11, Servo, LED, MQTT -things, Ultrasonic

Work In Progress:

Tutorial, camera, light-sensor
