@description('Name of the solution which is used to generate a short unique hash used in all resources.')
@minLength(6)
@maxLength(50)
param AppName string

@description('The administrator username of the SQL logical server.')
param SqlAdministratorLogin string

@description('The administrator password of the SQL logical server.')
@secure()
param SqlAdministratorLoginPassword string

@description('Location for all resources.')
param location string = resourceGroup().location

@description('Azure SQL GitHub metrics database name.')
param sqlDBName string = 'predictive-maintenance'

@description('Azure IoT Hub SKU. F1=free, S1=standard1. You can only have one free IoT Hub in a subscription. If you already have one, you must use S1.') // https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-devguide-pricing#pricing-tiers
@allowed([
  'F1'
  'S1'
])
param iotHubSku string

var app_name = toLower(AppName)
var hash = uniqueString(app_name, resourceGroup().id)

// storage accounts must be between 3 and 24 characters in length and use numbers and lower-case letters only
var storageAccountName = 'stg${hash}'
var hostingPlanName = 'app-plan-${hash}'
var appInsightsName = 'app-insights-${hash}'
var functionAppName = 'function-${hash}'
var sqlServerName = 'azure-sql-${hash}'
var iothubName = 'iot-hub-${hash}'
var dpsName = 'dps-${hash}'

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
  }
}

// output reporting_endpoint_url string = 'Azure Function App URL: https://${functionAppName}.azurewebsites.net'
// output reporting_endpoint_key string = 'Azure Function Host Key: ${resources.outputs.defaultHostKey}'


