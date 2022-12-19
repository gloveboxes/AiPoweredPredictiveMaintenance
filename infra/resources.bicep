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


// output defaultHostKey string = function_app.outputs.defaultHostKey

