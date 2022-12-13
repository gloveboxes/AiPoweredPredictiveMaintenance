# Sample Code in Python
########### Python 3.x #############
from dotenv import load_dotenv
from pathlib import Path
import http.client
import os

env_path = Path('..') / '.env'
load_dotenv(dotenv_path=env_path)

anomaly_detector_endpoint = os.environ.get('anomaly_detector_endpoint')
anomaly_detector_key = os.environ.get('anomaly_detector_key')

headers = {
    # Request headers
    'Content-Type': 'application/json',
    'Ocp-Apim-Subscription-Key': f'{anomaly_detector_key}',
}

try:
    conn = http.client.HTTPSConnection(anomaly_detector_endpoint.split('//')[1])
    conn.request("GET", "/anomalydetector/v1.1-preview/multivariate/models[?$skip][&$top]" % headers)
    response = conn.getresponse()
    data = response.read()
    print(data)
    conn.close()
except Exception as e:
    print("[Errno {0}] {1}".format(e.errno, e.strerror))

####################################