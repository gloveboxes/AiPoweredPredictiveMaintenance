param location string
param iotHubName string
param dpsName string
param iotHubSku string

var consumerGroupName = '${iotHubName}/events/cg1'
var iotHubKey = 'iothubowner'

resource iotHub 'Microsoft.Devices/IotHubs@2021-07-02' = {
  location: location
  name: iotHubName
  sku: {
    name: 'S1'
    capacity: 1
  }
  identity: {
    type: 'None'
  }
  properties: {
    routing: {
      enrichments: [
        {
          key: 'deviceName'
          value: '$twin.tags.deviceName'
          endpointNames: [
            'events'
          ]
        }
      ]
    }
    eventHubEndpoints: {
      events: {
        partitionCount: 4
        retentionTimeInDays: 1
      }
    }
  }
}

resource consumerGroup 'Microsoft.Devices/IotHubs/eventHubEndpoints/ConsumerGroups@2021-07-02' = {
  name: consumerGroupName
  properties: {
    name: 'telemetry'
  }
  dependsOn: [
    iotHub
  ]
}

resource provisioningServices_dps_ahjgdhjgjh_name_resource 'Microsoft.Devices/provisioningServices@2022-02-05' = {
  name: dpsName
  location: location
  sku: {
    name: iotHubSku
    capacity: 1
  }
  properties: {
    state: 'Active'
    provisioningState: 'Succeeded'
    iotHubs: [
      {
        connectionString: 'HostName=${iotHub.properties.hostName};SharedAccessKeyName=${iotHubKey};SharedAccessKey=${iotHub.listkeys().value[0].primaryKey}'
        location: 'australiaeast'
      }
    ]
    allocationPolicy: 'Hashed'
    enableDataResidency: false
  }
}

#disable-next-line outputs-should-not-contain-secrets
output eventHubConnectionString string = 'Endpoint=sb://${iotHub.properties.eventHubEndpoints.events.endpoint};SharedAccessKeyName=${iotHubKey};SharedAccessKey=${iotHub.listkeys().value[0].primaryKey};EntityPath=${iotHub.properties.eventHubEndpoints.events.path}'

// output defaultHostKey string = listkeys('${functionApp.id}/host/default', '2016-08-01').functionKeys.default
