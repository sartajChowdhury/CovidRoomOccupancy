Contains subfolders:
Server
Client

The Server subfolder contains just the modified lab code to run the server and show the count. It has one file:
server_final.ino

The server can be run on its own, requiring just the Arduino libraries "WiFi.h" and "esp_task_wdt.h"

The Client subfolder contains the main code used by the system. It has 4 files:
final_prototype.ino
sharedVariable.h
WirelessCommunication.cpp
WirelessCommunication.h

The client runs from "final_prototype.ino" and uses the supporting files "sharedVariable.h", "WirelessCommunication.cpp", and "WirelessCommunication.h" from lab. It also requires the Arduino library "Adafruit_VL53L0X.h".