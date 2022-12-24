using DotNetEnv;
using Microsoft.Azure.Devices.Client;
using Microsoft.Azure.Devices.Provisioning.Client.Transport;
using Microsoft.Azure.Devices.Provisioning.Client;
using Microsoft.Azure.Devices.Shared;
using System.Text;
using System.Timers;

// Sample based on Azure IoT C# SDK Samples. 
// https://github.com/Azure-Samples/azure-iot-samples-csharp

namespace dotnet.core.iot
{
    class Program
    {
        private static string idScope = String.Empty;
        private static string deviceId = String.Empty;
        private static string derivedKey = String.Empty;
        private static string owmKey = String.Empty;
        private const string GlobalDeviceEndpoint = "global.azure-devices-provisioning.net";
        static Telemetry telemetry = new Telemetry();
        static DeviceClient? iotClient;
        static int sendCount = 0;
        static System.Timers.Timer publishTelemetry = new System.Timers.Timer();
        const string appVersion = "1.0.9";


        static void load_config(string[] args)
        {
            try
            {
                if (args.Length > 0)
                {
                    Console.WriteLine($"Loading config from {args[0]}");
                    Env.Load(args[0]);
                }
                else
                {
                    Env.Load();
                }

                idScope = Env.GetString("ID_SCOPE", String.Empty);
                deviceId = Env.GetString("DEVICE_ID", String.Empty);
                derivedKey = Env.GetString("DERIVED_KEY", String.Empty);
                owmKey = Env.GetString("OWM_KEY", String.Empty);

                if (String.IsNullOrEmpty(idScope) || String.IsNullOrEmpty(deviceId) || String.IsNullOrEmpty(derivedKey))
                {
                    Console.WriteLine("Missing config keys. Please check your .env file and try again.");
                    System.Environment.Exit(1);
                }
            }
            catch (Exception e)
            {
                Console.WriteLine($"Error loading config: {e.Message}");
                System.Environment.Exit(1);
            }
        }

        static async Task Main(string[] args)
        {
            load_config(args);

            Console.WriteLine($"App version: {appVersion}, Device ID: {deviceId}. Press Ctrl+C to exit.");

            await telemetry.Init(owmKey);

            publishTelemetry.Interval = 1000;
            publishTelemetry.Elapsed += PublishTelemetry_Elapsed;
            publishTelemetry.AutoReset = false;
            publishTelemetry.Enabled = false;

            // Handle Ctrl+C
            var exitEvent = new ManualResetEvent(false);
            Console.CancelKeyPress += (sender, eventArgs) =>
            {
                eventArgs.Cancel = true;
                exitEvent.Set();
            };

            // Connect to IoT Hub
            using (var security = new SecurityProviderSymmetricKey(deviceId, derivedKey, derivedKey))
            using (var transport = new ProvisioningTransportHandlerMqtt())
            {
                ProvisioningDeviceClient provClient = ProvisioningDeviceClient.Create(GlobalDeviceEndpoint, idScope, security, transport);
                DeviceRegistrationResult result = await provClient.RegisterAsync();
                IAuthenticationMethod auth = new DeviceAuthenticationWithRegistrySymmetricKey(result.DeviceId, (security as SecurityProviderSymmetricKey).GetPrimaryKey());

                using (iotClient = DeviceClient.Create(result.AssignedHub, auth, TransportType.Mqtt))
                {
                    publishTelemetry.Enabled = true;

                    exitEvent.WaitOne();
                    System.Console.WriteLine("ctrl+c received. Quitting...");
                    System.Environment.Exit(0);
                }
            }
        }

        private static void PublishTelemetry_Elapsed(object? sender, ElapsedEventArgs e)
        {
            if (iotClient != null)
            {
                var json = telemetry.ToJson();

                System.Console.WriteLine($"Sending {++sendCount}: {json}");

                Message eventMessage = new Message(Encoding.UTF8.GetBytes(json));
                try
                {
                    iotClient.SendEventAsync(eventMessage).ConfigureAwait(false);
                }
                catch (Exception ex)
                {
                    System.Console.WriteLine(ex.Message);
                }
            }
            else
            {
                Console.WriteLine("Invalid IoT Hub Client. Check device ID Scope, Device ID, and Derived Device Key.");
            }

            publishTelemetry.Interval = 20 * 1000; // 20 seconds
            publishTelemetry.Enabled = true;
        }
    }
}