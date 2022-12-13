from azure.ai.anomalydetector import AnomalyDetectorClient
from azure.ai.anomalydetector.models import DetectionRequest, DetectionStatus
from azure.core.credentials import AzureKeyCredential
from azure.core.exceptions import HttpResponseError
from azure.storage.blob import BlobClient, BlobServiceClient, generate_blob_sas, BlobSasPermissions
from datetime import datetime, timedelta
import azure.functions as func
import json
import logging
import os
import pandas as pd
import tempfile
import time
import zipfile

class MultivariateSample:

    def __init__(self, anomaly_detector_endpoint=None, anomaly_detector_key=None, model_id=None, connection_string=None, container=None, blob_name=None):
        self.blob_name = blob_name
        self.container = container
        self.connection_string = connection_string
        self.model_id = model_id
        self.anomaly_detector_endpoint = anomaly_detector_endpoint
        self.anomaly_detector_key = anomaly_detector_key

        # Create an Anomaly Detector client

        # <client>
        self.ad_client = AnomalyDetectorClient(AzureKeyCredential(self.anomaly_detector_key), self.anomaly_detector_endpoint)
        # </client>        

    def upload_blob(self, filename):
        blob_client = BlobClient.from_connection_string(self.connection_string, container_name=self.container, blob_name=self.blob_name)
        with open(filename, "rb") as f:
            blob_client.upload_blob(f, overwrite=True)

    def detect(self, start_time, end_time):
        # Detect anomaly in the same data source (but a different interval)
        try:
            data_source = self.generate_data_source_sas(self.container, self.blob_name)
            detection_req = DetectionRequest(source=data_source, start_time=start_time, end_time=end_time)
            response_header = self.ad_client.detect_anomaly(self.model_id, detection_req,
                                                            cls=lambda *args: [args[i] for i in range(len(args))])[-1]
            result_id = response_header['Location'].split("/")[-1]

            # Get results (may need a few seconds)
            r = self.ad_client.get_detection_result(result_id)
            logging.info("Get detection result...(it may take a few seconds)")

            while r.summary.status != DetectionStatus.READY and r.summary.status != DetectionStatus.FAILED:
                r = self.ad_client.get_detection_result(result_id)
                logging.info("waiting for anomaly detection result...")
                time.sleep(1)

            if r.summary.status == DetectionStatus.FAILED:
                logging.info("Detection failed.")
                if r.summary.errors:
                    for error in r.summary.errors:
                        logging.info("Error code: {}. Message: {}".format(error.code, error.message))
                else:
                    logging.info("None")
                return None

        except HttpResponseError as e:
            logging.info('Error code: {}'.format(e.error.code), 'Error message: {}'.format(e.error.message))
            return None
        except Exception as e:
            raise e

        return r

    def generate_data_source_sas(self, container, blob_name):
        BLOB_SAS_TEMPLATE = "{blob_endpoint}{container_name}/{blob_name}?{sas_token}"
     
        self.connection_string = os.getenv('MyStorageConnectionAppSetting')
        blob_service_client = BlobServiceClient.from_connection_string(conn_str=self.connection_string)
        sas_token = generate_blob_sas(account_name=blob_service_client.account_name,
                                    container_name=container, blob_name=blob_name,
                                    account_key=blob_service_client.credential.account_key,
                                    permission=BlobSasPermissions(read=True),
                                    expiry=datetime.utcnow() + timedelta(days=1))
        blob_sas = BLOB_SAS_TEMPLATE.format(blob_endpoint=blob_service_client.primary_endpoint,
                                            container_name=container, blob_name=blob_name, sas_token=sas_token)
        return blob_sas


def main(req: func.HttpRequest, telemetry: func.SqlRowList, telemetryBlob: func.Out[bytes]) -> func.HttpResponse:
    logging.info('Python HTTP trigger function processed a request.')

    connection_string = os.getenv('MyStorageConnectionAppSetting')
    model_id = os.getenv('AnomalyDetectorModelId')
    anomaly_detector_endpoint = os.getenv('AnomalyDetectorEndpoint')
    anomaly_detector_key = os.getenv('AnomalyDetectorKey')
    
    temp_dir = tempfile.gettempdir()
    zip_filename = temp_dir + "/telemetry_mvad.zip"
    config = {}

    deviceId = req.params.get('deviceId')

    if deviceId:

        df = pd.DataFrame(telemetry)
        df.set_index('timestamp', inplace=True) 

        if not df.empty:

            zip_file = zipfile.ZipFile(zip_filename, "w", zipfile.ZIP_DEFLATED)

            for variable in df.columns:
                individual_df = pd.DataFrame(df[variable].values, index=df.index, columns=["value"])
                individual_df.to_csv(temp_dir + "/" + variable + ".csv", index=True)
                zip_file.write(temp_dir + "/" + variable + ".csv", arcname=variable + ".csv")

            zip_file.close()

            sample = MultivariateSample(anomaly_detector_endpoint, anomaly_detector_key, model_id, connection_string, 'data', "telemetry_mvad.zip")

            sample.upload_blob(zip_filename)

            start_time = df.index[0]
            end_time = df.index[-3]

            r = sample.detect(start_time=start_time, end_time=end_time)

            results = r.results

            # logging.info(result)

            

            start_time = datetime.strptime(end_time, '%Y-%m-%dT%H:%M:%SZ')
            start_time = start_time - timedelta(days=3)
            start_time = start_time.strftime('%Y-%m-%dT%H:%M:%SZ') 

            test_df = test_df = df.loc[start_time:end_time]

            is_anomalies = []
            sev = []
            scores = []
            sensitivity = 0.15

            for item in results:
                if item.value:
                    is_anomalies.append(item.value.is_anomaly)
                    sev.append(item.value.severity)
                    scores.append(item.value.score)


            anomalous_timestamps = []
            num_contributors = 3
            top_values = {f"top_{i}": [] for i in range(num_contributors)}
            for ts, item in zip(test_df.index, r.results):
                if item.value.is_anomaly and item.value.severity > 1 - sensitivity:
                    anomalous_timestamps.append(ts)
                    for i in range(num_contributors):
                        top_values[f"top_{i}"].append(test_df[item.value.interpretation[i].variable][ts])

            t = type(top_values)
            # json_data = json.dumps(top_values)

            config.update({"deviceId": deviceId})
            config.update({"is_anomalies": is_anomalies})
            config.update({"sev": sev})
            config.update({"scores": scores})
            config.update({"anomalous_timestamps": anomalous_timestamps})
            config.update({"dataframe": test_df.to_json()})
            # config.update({"top_values": json_data})

        return func.HttpResponse(json.dumps(config), status_code=200)
    else:
        return func.HttpResponse(
            "This HTTP triggered function executed successfully. Pass a name in the query string or in the request body for a personalized response.",
            status_code=200
        )
