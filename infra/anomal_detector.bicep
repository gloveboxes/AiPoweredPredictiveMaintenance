resource accounts_anomaly_predictive_maintenance_name_resource 'Microsoft.CognitiveServices/accounts@2022-10-01' = {
  name: accounts_anomaly_predictive_maintenance_name
  location: 'australiaeast'
  sku: {
    name: 'F0'
  }
  kind: 'AnomalyDetector'
  identity: {
    type: 'SystemAssigned'
  }
  properties: {
    customSubDomainName: accounts_anomaly_predictive_maintenance_name
    networkAcls: {
      defaultAction: 'Allow'
      virtualNetworkRules: []
      ipRules: []
    }
    publicNetworkAccess: 'Enabled'
  }
}
