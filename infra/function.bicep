param storageAccountName string
param location string
param appInsightsName string
param hostingPlanName string
param functionAppName string
param sqlConnectionString string
param eventHubConnectionString string

resource storageAccount 'Microsoft.Storage/storageAccounts@2022-05-01' = {
  name: storageAccountName
  location: location
  kind: 'StorageV2'
  sku: {
    name: 'Standard_LRS'
  }
  properties: {
    supportsHttpsTrafficOnly: true
    minimumTlsVersion: 'TLS1_2'
  }
}

resource appInsights 'Microsoft.Insights/components@2020-02-02' = {
  name: appInsightsName
  location: location
  kind: 'web'
  properties: {
    Application_Type: 'web'
    publicNetworkAccessForIngestion: 'Enabled'
    publicNetworkAccessForQuery: 'Enabled'
  }
  tags: {
    // circular dependency means we can't reference functionApp directly  /subscriptions/<subscriptionId>/resourceGroups/<rg-name>/providers/Microsoft.Web/sites/<appName>"
    'hidden-link:/subscriptions/${subscription().id}/resourceGroups/${resourceGroup().name}/providers/Microsoft.Web/sites/${functionAppName}': 'Resource'
  }
}

resource hostingPlan 'Microsoft.Web/serverfarms@2022-03-01' = {
  name: hostingPlanName
  location: location
  sku: {
    name: 'Y1'
    tier: 'Dynamic'
    // https://stackoverflow.com/questions/47522539/server-farm-service-plan-skus
  }
}

// resource deployment 'Microsoft.Web/sites/sourcecontrols@2022-03-01' = {
//   name: 'web'
//   parent: functionApp
//   properties: {
//     isManualIntegration: false
//     deploymentRollbackEnabled: false
//     // isGitHubAction: true
//     // repoUrl: 'https://github.com/gloveboxes/AiPoweredPredictiveMaintenanceFunctions'
//     // branch: 'master'
    
//     // gitHubActionConfiguration: {
//     //   codeConfiguration: {
//     //     runtimeStack: 'dotnet'
//     //     runtimeVersion: '6.0.x'
//     //   }
//     //   generateWorkflowFile: true
//     //   // workflowSettings: {
//     //   //   appType: 'functionapp'
//     //   //   publishType: 'code'
//     //   //   os: 'linux'
//     //   //   runtimeStack: 'dotnet'
//     //   //   workflowApiVersion: '2020-12-01'
//     //   //   slotName: 'production'
//     //   //   variables: {
//     //   //     runtimeVersion: '6.0.x'
//     //   //   }
//     //   // }
//     // }
//   }
// }

resource functionApp 'Microsoft.Web/sites@2022-03-01' = {
  name: functionAppName
  location: location
  kind: 'functionapp,linux'
  properties: {
    httpsOnly: true
    serverFarmId: hostingPlan.id
    clientAffinityEnabled: true
    siteConfig: {
      appSettings: [
        {
          name: 'APPINSIGHTS_INSTRUMENTATIONKEY'
          value: appInsights.properties.InstrumentationKey
        }
        {
          name: 'AzureWebJobsStorage'
          value: 'DefaultEndpointsProtocol=https;AccountName=${storageAccount.name};EndpointSuffix=${environment().suffixes.storage};AccountKey=${listKeys(storageAccount.id, storageAccount.apiVersion).keys[0].value}'
        }
        {
          name: 'FUNCTIONS_EXTENSION_VERSION'
          value: '~4'
        }
        {
          name: 'FUNCTIONS_WORKER_RUNTIME'
          value: 'dotnet'
        }
        {
          name: 'WEBSITE_CONTENTAZUREFILECONNECTIONSTRING'
          value: 'DefaultEndpointsProtocol=https;AccountName=${storageAccount.name};EndpointSuffix=${environment().suffixes.storage};AccountKey=${listKeys(storageAccount.id, storageAccount.apiVersion).keys[0].value}'
        }
        {
          name: 'PredictiveMaintenanceIotHub'
          value: eventHubConnectionString
        }
      ]
      connectionStrings: [
        {
          connectionString: sqlConnectionString
          name: 'SqlConnectionString'
          type: 'SQLAzure'
        }
      ]
      cors: {
        allowedOrigins: [
          'https://ms.portal.azure.com'
        ]
      }
    }
  }

  // dependsOn: [
  //   // appInsights
  //   // hostingPlan
  //   // storageAccount
  //   // sqlServer
  // ]
}

#disable-next-line outputs-should-not-contain-secrets
output defaultHostKey string = listkeys('${functionApp.id}/host/default', '2016-08-01').functionKeys.default
#disable-next-line outputs-should-not-contain-secrets
output storageConnectionString string = 'DefaultEndpointsProtocol=https;AccountName=${storageAccount.name};EndpointSuffix=${environment().suffixes.storage};AccountKey=${listKeys(storageAccount.id, storageAccount.apiVersion).keys[0].value}'
