import logging
import json
import azure.functions as func
from datetime import datetime


def main(event: func.EventHubEvent, iotData: func.Out[func.SqlRow]):
    data = json.loads(event.get_body().decode('utf-8'))

    properties = event.metadata['Properties']
    systemProperties = event.metadata['SystemProperties']

    timestamp = systemProperties['iothub-enqueuedtime']
    deviceId = systemProperties['iothub-connection-device-id']
    deviceName = properties['deviceName']

    # Zero out seconds and milliseconds from timestamp
    timestamp = datetime.strptime(timestamp, '%Y-%m-%dT%H:%M:%S.%fZ')
    timestamp= timestamp.replace(second=0, microsecond=0)
    timestamp = timestamp.strftime('%Y-%m-%d %H:%M:%S') 

    if deviceName is None:
        deviceName = deviceId

    data.update({'deviceId': deviceName, 'timestamp': timestamp})

    row = func.SqlRow(data)

    iotData.set(row)

    logging.info('Python EventHub trigger processed an event: %s', data)
