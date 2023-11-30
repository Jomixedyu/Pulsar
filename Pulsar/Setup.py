import zipfile
import os

files = [
    "Source/PulsarEd/third/fbxsdk/lib.zip"
]





for zip_path in files:
    zip_file = zipfile.ZipFile(zip_path)
    dir_path = os.path.dirname(zip_path)
    zip_file.extractall(dir_path)