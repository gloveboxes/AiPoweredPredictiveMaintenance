param anomalyDetectorName string
param location string = resourceGroup().location
param anomalyDetectorSku string

resource anomaly_detector_service 'Microsoft.CognitiveServices/accounts@2022-10-01' = {
  name: anomalyDetectorName
  location: location
  sku: {
    name: anomalyDetectorSku
  }
  kind: 'AnomalyDetector'
  identity: {
    type: 'SystemAssigned'
  }
}

#disable-next-line outputs-should-not-contain-secrets
output anomalyDetectorEndpointKey string = anomaly_detector_service.listKeys().key1
