from flask import Flask, jsonify, send_from_directory
import os
import hashlib
from waitress import serve

app = Flask(__name__)

# Configuration
UPDATE_FOLDER = 'updates'
LATEST_VERSION = '1.0.1'
LATEST_FILE = 'Bootstrapper.exe'

def calculate_sha256(file_path):
    sha256_hash = hashlib.sha256()
    with open(file_path, 'rb') as f:
        for byte_block in iter(lambda: f.read(4096), b""):
            sha256_hash.update(byte_block)
    return sha256_hash.hexdigest()

@app.route('/latest_version', methods=['GET'])
def get_latest_version():
    file_path = os.path.join(UPDATE_FOLDER, LATEST_FILE)
    file_hash = calculate_sha256(file_path) if os.path.exists(file_path) else None

    return jsonify({
        'version': LATEST_VERSION,
        'file_name': LATEST_FILE,
        'file_hash': file_hash
    })

@app.route('/update/<filename>', methods=['GET'])
def get_update_file(filename):
    return send_from_directory(UPDATE_FOLDER, filename)

if __name__ == '__main__':
    if not os.path.exists(UPDATE_FOLDER):
        os.makedirs(UPDATE_FOLDER)
    serve(app, host='0.0.0.0', port=5000)
