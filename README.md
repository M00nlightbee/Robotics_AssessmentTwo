# Robotics

## Assignment Title: Solving real world problems through robotics

AI is used creatively and critically by the human. The human uses AI a co-creator with a critical thinking approach to generating novel AI activities and outputs 
This submission aligns with AITS 5 of the Artificial Intelligence Transparency Scale (AITS).

---

## Project Inspiration
In this task a robot system is built to perform a repetitive, complex, task automatically and without the need for intervention or control from an operator.
As this project is about solving real world problems, i decided to build a security system with moveable parts for a bicycle as there are thousands of bicycle being stolen with no way of tracking them.

There has been an experiment to see if one could use the apple airtags to keep track of stolen and it was found that tags were unfortunately not effiecent.[**Video Documentation of the experiment**](https://www.youtube.com/watch?v=S0NRo0uoehw)  

## Project Overview
This an event driven robotic system, it integrates radar sensing with computer vision to autonomously detect and respond to human presence.

A robotic system that detects motion using a radar sensor, activates a live camera stream, performs face recognition, and dynamically adapts its behaviour based on whether a detected individual is authorized or unknown

## Hardware Components

- 3× ESP32 boards
- ESP32-CAM with ESP32-CAM-MB
- LD2410 radar sensor
- Servo motor
- Buzzer
- LCD display
- Jumper Wires
- Breadboards

## Environment

- **Arduino IDE**: Write c++ code for ESP32 Boards
- **Visual Studio Code**: Write code for computer vision

> [!NOTE]
> Install dependencies to run python codes with no errors
```
pip install opencv-python
```
```
pip install pyserial
```
---
### Architecture

The program is divided into three layers:

#### 1. Sensor and Vision Layer

- **Radar Sensor**:
  - Detects presence, motion, and stationary targets
  - Operates in real-time using LD2410
- **Camera**:
  - Used for Live Camera Streaming
    - Activated only when motion is detected
    - Reduces unnecessary processing
  - Used for Face Recognition (OpenCV)
    - Distinguishes between known and unknown individuals
    - Uses LBPH algorithm for efficient recognition

#### 2. Robot Brain

- **State Machine Control**:
  - IDLE → SCANNING → TRACKING → ALERT
- **ESP-NOW Communication**:
  - Low-latency wireless communication between nodes
  - No router required

#### 3. Control Layer

The system performs it robotic capabilities using components such as:
- Servo Controller (Camera Sweep)  
- Buzzer (Audio Feedback) 
- LCD Display (System Status, e.g: Traget: Idle)

---

### Key Functions and Scripts

In the Arduino IDE to get the mac address for each ESP32 board
#### MAC Address code
```c++
#include "WiFi.h"

void setup() {
  Serial.begin(115200);

  // WIFI_STA mode to initialize the WiFi hardware
  WiFi.mode(WIFI_STA); 
  
  // Optional: You may not need a delay for just to be save you can leave uncommented
  while (WiFi.status() == WL_STOPPED) { 
    delay(10); 
    }

  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
}

void loop() {}
```

#### Test Script for Video
```python

import cv2

# Connect to ESP32 camera stream
video = cv2.VideoCapture("http://<camera-ip>:81/stream")

# Check if connection was successful
if not video.isOpened():
    print("Error: Could not open video stream from ESP32")
    exit()

print("Camera connected successfully!")

try:
    while True:
        # Read frame from the stream
        ret, frame = video.read()
        
        if not ret:
            print("Error: Failed to read frame")
            break
        
        # Display the frame
        cv2.imshow("ESP32 Camera Stream", frame)
        
        # Press 'q' to exit
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

except KeyboardInterrupt:
    print("Stream interrupted by user")

finally:
    # Release resources
    video.release()
    cv2.destroyAllWindows()
    print("Camera stream closed")

```

> [!CAUTION]
> ESP32 CAM only connects to WiFi 2.4GHz

### Connect ESP32 CAM WiFi 2.4GHz
If your WiFi is not on 2.4GHz your ESP32 CAM will not connect to WiFi.

Here is a step by step walkaround:

1. Go to settings and click on Network & Internet
2. Click on Mobile hotspot, turn it on
3. Click on edit and give a name and password. (MAKE SURE NOT TO USE A SPECIAL FOR THE NAME AND NO SPACES)
```c++
# Replace with the name and password you have created
# You will find this in your CameraWebServer code in the Arduino IDE
const char *ssid = "**************";
const char *password = "**************";
```
4. While in edit mode , change the Network band to 2.4GHz
5. Your ESP32 CAM should show up with your IP address , your CAM is streaming to

> [!NOTE]
> You may sometimes need to press the reset button on your CAM 

----

### Project Features 

| Feature                        | Status | Notes          |
| ------------------------------ | ------ | ---------------
| Radar Motion Detection         | ✅     | Implemented     
| Live Camera Streaming          | ✅     | Implemented    
| Face Detection (OpenCV)        | ✅     | Implemented                 
| Face Recognition (OpenCV)      | ⚠️     | Partially Implemented                   
| State Machine Control          | ✅     | Implemented                
| ESP-NOW Communication          | ✅     | Implemented              
| Multi-Modal Feedback           | ✅     | Implemented             

---

### Future Improvements

- Train face with different lighting or downgrade from current python version to use a [**Better Model**](https://github.com/ageitgey/face_recognition) 
- Upgrade to ESP32-S3 to compute computer vision directly using the board
- Lock onto face when detected and track movement of face with servo instead(maybe add another servo for a close to 360degree vision)
- Fine tune the radar for specific distance 

---

### Setup

1. Downlaod the [**Arduino IDE**](https://docs.arduino.cc/software/ide/)
2. Open the Arduino IDE after download
3. Click on Tools → Board → Boards Manager
4. In the search bar type ESP32 and install the one by Espressif Systems
5. Click on the Library manager (The middle icon on the left looks like a bookshelf)
6. Use the search bar to downlad all 3 libraries:
    1. LiquidCrystal
    2. ESP32Servo
    3. MyLD2410
7. Click on Tools → Board → esp32 → find your board name
8. Upload the MAC Address code to each ESP32 board and make note of them and which board they belong to
9. To Get the CAM working [**Follow Steps**](https://randomnerdtutorials.com/upload-code-esp32-cam-mb-usb/)
10. Run the python scripts: 
    1. capture.py (collects your Face Dataset)
    2. train_model.py (Train Face Recognition Model)
    3. main.py
---

### References

- [**ESP32io Tutorials**](https://esp32io.com/tutorials/esp32-software-installation)
- [**Interfacing 16X2 LCD Module with ESP32 with and without I2C**](https://www.circuitschools.com/interfacing-16x2-lcd-module-with-esp32-with-and-without-i2c/)
- [**LD2410C Human Presence Detector**](https://dronebotworkshop.com/ld2410c-human-sensor/)
- [**Effective Ways To Detect People With Common Sensors**](https://www.youtube.com/watch?v=LNeSz5hfTLI)
- [**How to Interface LCD With ESP32**](https://www.instructables.com/ESP32-How-to-Interface-LCD-With-ESP32-Microcontrol/)
- [**Create Custom Animations on 16x2 LCD Displays**](https://projecthub.arduino.cc/tusindfryd/create-custom-animations-on-16x2-lcd-displays-57d776)
- [**Using the ESP32-CAM-MB**](https://www.espboards.dev/blog/flash-any-esp32-with-esp32-cam-mb/)
- [**Interfacing ESP32 And 16×2 LCD Parallel Data (Without I2C)**](https://www.makerguides.com/interfacing-esp32-and-16x2-lcd-parallel-data-without-i2c/)
- [**ESP32-CAM**](https://randomnerdtutorials.com/esp32-cam-video-streaming-web-server-camera-home-assistant/)
- [**ESP32-CAM AI-Thinker using ESP32-CAM-MB USB Programmer**](https://randomnerdtutorials.com/upload-code-esp32-cam-mb-usb/)

---
