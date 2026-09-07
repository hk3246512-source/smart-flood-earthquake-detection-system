# smart-flood-earthquake-detection-system
A low-cost, real-time early warning system built on the ESP8266 (NodeMCU) that detects both flooding and earthquake-like vibrations, then sends SMS alerts — without needing internet connectivity. Designed with rural and disaster-prone areas in mind, where internet access during emergencies is often unreliable.

Problem it addresses

Most existing flood monitoring systems focus only on water level detection. This project combines flood detection and seismic/vibration detection in a single low-cost unit, with offline SMS alerting as the notification channel.

Hardware Components
ESP8266 (NodeMCU) — main microcontroller
Float sensor — detects rising water presence
JSN-SR04T ultrasonic sensor — measures water level distance
MPU-6050 accelerometer — detects earthquake-like vibrations
SIM900A GSM module — sends SMS alerts over the cellular network (no internet required)
How It Works

Flood Detection

The float sensor detects when water reaches a critical height (digital HIGH signal).
The ultrasonic sensor (JSN-SR04T) continuously measures the distance to the water surface; if the distance drops below a set threshold, it signals rising floodwater.
Either trigger sends an SMS alert to a pre-set alert number and a rescue-team number.

Earthquake Detection

On startup, the MPU-6050 is calibrated by averaging several accelerometer readings to establish a baseline (accounting for sensor offset/gravity).
During operation, the system continuously compares live accelerometer readings against the calibrated baseline.
If the deviation on any axis (X, Y, or Z) exceeds a set threshold, it's flagged as a vibration/earthquake event, and an SMS alert is triggered.

Offline SMS Alerting

The SIM900A GSM module is controlled via AT commands over a software serial connection.
Alerts are sent directly over the GSM cellular network, so the system works even with no WiFi or internet access — critical for disaster scenarios where internet infrastructure may be down.
Duplicate alerts are prevented using state flags, so the same alert isn't repeatedly sent while a condition persists.
Notes
Phone numbers in the code are placeholders (+8801XXXXXXXXX) — replace with actual numbers before deployment.
Threshold values (distance, vibration) can be tuned based on the specific deployment site and sensor calibration.
Project Context

Built as an academic project at North South University (Aug 2025 – Dec 2025), Electrical & Electronics Engineering.
