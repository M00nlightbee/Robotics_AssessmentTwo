import cv2
import os
import numpy as np

dataset_path = "dataset"

faces = []
ids = []

for file in os.listdir(dataset_path):
    path = os.path.join(dataset_path, file)

    img = cv2.imread(path, cv2.IMREAD_GRAYSCALE)
    id = int(file.split('.')[1])

    faces.append(img)
    ids.append(id)

recognizer = cv2.face.LBPHFaceRecognizer_create()
recognizer.train(faces, np.array(ids))

recognizer.save("face_model.yml")

print("Training complete!")