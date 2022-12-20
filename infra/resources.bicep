param appInsightsName string
param functionAppName string
param hostingPlanName string
param location string
param sqlAdministratorLogin string
param sqlDBName string
param sqlServerName string
param storageAccountName string
param iothubName string
param dpsName string
param iotHubSku string
param anomalyDetectorName string
param anomalyDetectorSku string

@secure()
param sqlAdministratorLoginPassword string

module azure_sql 'sql.bicep' = {
  name: 'azure_sql'
  params: {
    sqlServerName: sqlServerName
    sqlDBName: sqlDBName
    location: location
    sqlAdministratorLogin: sqlAdministratorLogin
    sqlAdministratorLoginPassword: sqlAdministratorLoginPassword
  }
}

module function_app 'function.bicep' = {
  name: 'function_app'
  params: {
    storageAccountName: storageAccountName
    appInsightsName: appInsightsName
    hostingPlanName: hostingPlanName
    functionAppName: functionAppName
    location: location
    sqlConnectionString: azure_sql.outputs.SQL_CONNECTION_STRING
    eventHubConnectionString: iothub.outputs.eventHubConnectionString
  }
}

module iothub 'iot_hub.bicep' = {
  name: 'iothub'
  params: {
    location: location
    iotHubName: iothubName
    dpsName: dpsName
    iotHubSku: iotHubSku
  }
}

module anomaly_detector 'anomaly_detector.bicep' = {
  name: 'anomaly_detector'
  params: {
    location: location
    anomalyDetectorName: anomalyDetectorName
    anomalyDetectorSku: anomalyDetectorSku
  }
}


output defaultHostKey string = function_app.outputs.defaultHostKey
output idScope string = iothub.outputs.idScope
output anomalyDetectorEndpointKey string = anomaly_detector.outputs.anomalyDetectorEndpointKey
output storageConnectionString string = function_app.outputs.storageConnectionString
