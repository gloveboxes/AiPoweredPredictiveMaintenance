@description('Name of the solution which is used to generate a short unique hash used in all resources.')
@minLength(6)
@maxLength(50)
param AppName string = newGuid()

@description('The administrator username of the SQL logical server.')
param SqlAdministratorLogin string

@description('The administrator password of the SQL logical server.')
@secure()
param SqlAdministratorLoginPassword string

// @description('Location for all resources.')
// param location string = resourceGroup().location

@description('Azure IoT Hub SKU. F1=free, S1=standard1. You can only have one free IoT Hub in a subscription. If you already have a free one, you must use S1.') // https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-devguide-pricing#pricing-tiers
@allowed([
  'F1'
  'S1'
])
param iotHubSku string

@description('Azure Anomaly Detector Service SKU. F0=free, S0=standard. You can only have one free Anomaly Detector Service in a subscription. If you already have a free one, you must use S0.')
@allowed([
  'F0'
  'S0'
])
param anomalyDetectorSku string

var app_name = toLower(AppName)
var hash = uniqueString(app_name, resourceGroup().id)

// @description('Location for all resources.')
#disable-next-line no-loc-expr-outside-params
var location = resourceGroup().location

// storage accounts must be between 3 and 24 characters in length and use numbers and lower-case letters only
var storageAccountName = 'stgpm${hash}'
var hostingPlanName = 'app-plan-pm-${hash}'
var appInsightsName = 'app-insights-pm-${hash}'
var functionAppName = 'func-pm-${hash}'
var sqlServerName = 'sql-pm-${hash}'
var iothubName = 'iot-hub-pm-${hash}'
var dpsName = 'dps-pm-${hash}'
var sqlDBName = 'predictive-maintenance'
var anomalyDetectorName = 'anomaly-detector-pm-${hash}'

module resources './resources.bicep' = {
  name: 'resources'
  params: {
    sqlServerName: sqlServerName
    sqlDBName: sqlDBName
    location: location
    sqlAdministratorLogin: SqlAdministratorLogin
    sqlAdministratorLoginPassword: SqlAdministratorLoginPassword
    storageAccountName: storageAccountName
    appInsightsName: appInsightsName
    hostingPlanName: hostingPlanName
    functionAppName: functionAppName
    iothubName: iothubName
    dpsName: dpsName
    iotHubSku: iotHubSku
    anomalyDetectorName: anomalyDetectorName
    anomalyDetectorSku: anomalyDetectorSku
  }
}

output telemetry_function_app_name string = 'Azure Function App Name: ${functionAppName}'
output inference_telemetry_endpoint_url string = 'Inference Telemetry Endpoint URL: https://${functionAppName}.azurewebsites.net'
output inference_telemetry_endpoint_key string = 'Inference Telemetry Endpoint Key: ${resources.outputs.defaultHostKey}'
output idScope string = 'Azure DPS ID Scope: ${resources.outputs.idScope}'
output anomaly_detector_key string = 'Anomaly Detector Key: ${resources.outputs.anomalyDetectorEndpointKey}'
output storage_connection_string string = 'Storage Connection String: ${resources.outputs.storageConnectionString}'
