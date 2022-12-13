using IoTHubTrigger = Microsoft.Azure.WebJobs.EventHubTriggerAttribute;
using Microsoft.Azure.EventHubs;
using Microsoft.Azure.WebJobs;
using Microsoft.Extensions.Logging;
using System.Text;
using System;

namespace Glovebox.Function
{
    public class PredictiveMaintenanceIotHub
    {
        static object deviceId = string.Empty;
        static object deviceName = string.Empty;
        static object enqueuedTime = string.Empty;

        [FunctionName("PredictiveMaintenanceIotHub")]
        public void Run([IoTHubTrigger("messages/events", Connection = "PredictiveMaintenanceIotHub", ConsumerGroup = "telemetry")] EventData message, ILogger log,
        [Sql("dbo.Telemetry", ConnectionStringSetting = "SqlConnectionString")] IAsyncCollector<Telemetry> newItems)
        {
            try
            {
                string payload = Encoding.UTF8.GetString(message.Body.Array);
                var telemetry = Newtonsoft.Json.JsonConvert.DeserializeObject<Telemetry>(payload);

                message.Properties.TryGetValue("deviceName", out var deviceName);
                message.SystemProperties.TryGetValue("iothub-enqueuedtime", out var enqueuedTime);
                message.SystemProperties.TryGetValue("iothub-connection-device-id", out var deviceId);

                telemetry.deviceId = deviceName != null && (string)deviceName != "$twin.tags.deviceName" ? deviceName.ToString() : deviceId.ToString();
                var timestamp = DateTime.Parse(enqueuedTime.ToString());

                telemetry.timestamp = new DateTime(timestamp.Year, timestamp.Month, timestamp.Day, timestamp.Hour, timestamp.Minute, 0);

                newItems.AddAsync(telemetry);
                newItems.FlushAsync();
            }
            catch (Exception ex)
            {
                log.LogError(ex, "Error in TelemetryWebhook");
            }
        }
    }
}