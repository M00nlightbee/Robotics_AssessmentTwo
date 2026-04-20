import cv2
import os

# ===== CONFIG =====
STREAM_URL = "http://<camera-ip>:81/stream"
DATASET_PATH = "dataset"
PERSON_NAME = "Replace with person to be authorized name"
MAX_IMAGES = 30   # for auto mode

os.makedirs(DATASET_PATH, exist_ok=True)

video = cv2.VideoCapture(STREAM_URL)

face_cascade = cv2.CascadeClassifier(
    cv2.data.haarcascades + 'haarcascade_frontalface_default.xml'
)

count = 0

print("Press 's' to save manually OR auto-capture will run...")

while True:
    ret, frame = video.read()
    if not ret:
        break

    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    faces = face_cascade.detectMultiScale(gray, 1.3, 5)

    for (x, y, w, h) in faces:
        face_gray = gray[y:y+h, x:x+w]

        cv2.rectangle(frame, (x,y), (x+w,y+h), (0,255,0), 2)

        key = cv2.waitKey(1) & 0xFF

        # ===== MANUAL SAVE =====
        if key == ord('s'):
            filename = f"{DATASET_PATH}/{PERSON_NAME}.{count}.jpg"
            cv2.imwrite(filename, face_gray)
            print(f"Saved {filename}")
            count += 1

        # ===== AUTO SAVE =====
        elif count < MAX_IMAGES:
            filename = f"{DATASET_PATH}/{PERSON_NAME}.{count}.jpg"
            cv2.imwrite(filename, face_gray)
            count += 1

    cv2.imshow("Capture Faces", frame)

    if cv2.waitKey(1) & 0xFF == ord('q') or count >= MAX_IMAGES:
        break

video.release()
cv2.destroyAllWindows()