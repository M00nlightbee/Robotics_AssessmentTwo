import serial
import cv2

# ===== CONFIG =====
SERIAL_PORT = 'COM3'
STREAM_URL = "http://<camera-ip>:81/stream"
MODEL_PATH = "face_model.yml"

ser = serial.Serial(SERIAL_PORT, 115200)

recognizer = cv2.face.LBPHFaceRecognizer_create()
recognizer.read(MODEL_PATH)

face_cascade = cv2.CascadeClassifier(
    cv2.data.haarcascades + 'haarcascade_frontalface_default.xml'
)

video = None
vision_active = False
last_state = None


# ===== SEND COMMAND =====
def send(cmd):
    global last_state
    if cmd != last_state:
        ser.write((cmd + "\n").encode())
        last_state = cmd


print("System started...")

while True:
    msg = ser.readline().decode(errors="ignore").strip()

    # ===== RADAR TRIGGER =====
    if msg == "CAMERA_ON":
        if not vision_active:
            video = cv2.VideoCapture(STREAM_URL)
            vision_active = True

    elif msg == "CAMERA_OFF":
        if vision_active:
            video.release()
            cv2.destroyAllWindows()
            vision_active = False

        send("RADAR_ON")
        continue

    # ===== VISION SYSTEM =====
    if vision_active and video is not None:

        ret, frame = video.read()
        if not ret:
            continue

        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        faces = face_cascade.detectMultiScale(gray, 1.3, 5)

        authorized_face_found = False
        unknown_face_found = False

        for (x, y, w, h) in faces:
            id, confidence = recognizer.predict(gray[y:y+h, x:x+w])

            authorized = confidence < 70

            if authorized:
                authorized_face_found = True
            else:
                unknown_face_found = True

            cv2.rectangle(frame, (x,y), (x+w,y+h), (0,255,0), 2)

        # ===== DECISION LOGIC =====
        if authorized_face_found:
            print("Authorized user detected")
            send("RADAR_OFF")

        elif unknown_face_found:
            print("Unknown face detected!")
            send("RADAR_ON")

        else:
            send("RADAR_ON")

        cv2.imshow("Vision System", frame)

        if cv2.waitKey(1) == ord('q'):
            break