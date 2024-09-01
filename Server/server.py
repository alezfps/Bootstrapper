from flask import Flask, jsonify, send_from_directory
import os

app = Flask(__name__)

# Configuration
UPDATE_FOLDER = 'updates'
LATEST_VERSION = '1.0.0'
LATEST_FILE = 'Bootstrap.exe'

@app.route('/latest_version', methods=['GET'])
def get_latest_version():
    return jsonify({
        'version': LATEST_VERSION,
        'file_name': LATEST_FILE
    })

@app.route('/update/<filename>', methods=['GET'])
def get_update_file(filename):
    return send_from_directory(UPDATE_FOLDER, filename)

if __name__ == '__main__':
    if not os.path.exists(UPDATE_FOLDER):
        os.makedirs(UPDATE_FOLDER)
    app.run(host='0.0.0.0', port=5000)
