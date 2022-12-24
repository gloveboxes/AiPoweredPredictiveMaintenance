using Newtonsoft.Json;
using WeatherNet;
using WeatherNet.Clients;
using System.Timers;


namespace dotnet.core.iot
{
    class Location
    {
        [JsonPropertyAttribute(PropertyName = "city")]
        public string City { get; set; } = String.Empty;

        [JsonPropertyAttribute(PropertyName = "latitude")]
        public double Latitude { get; set; } = 0;

        [JsonPropertyAttribute(PropertyName = "longitude")]
        public double Longitude { get; set; } = 0;
    }

    class Telemetry
    {
        [JsonPropertyAttribute(PropertyName = "temperature")]
        private int Temperature { get; set; } = 0;

        [JsonPropertyAttribute(PropertyName = "co2ppm")]
        private int Co2ppm { get; set; } = 0;

        [JsonPropertyAttribute(PropertyName = "humidity")]
        private int Humidity { get; set; } = 0;

        [JsonPropertyAttribute(PropertyName = "prediction")]
        public int Prediction { get; set; } = 0;

        [JsonPropertyAttribute(PropertyName = "peakUserMemoryKiB")]
        public int PeakUserMemoryKiB { get; set; } = 0;

        [JsonPropertyAttribute(PropertyName = "totalMemoryKiB")]
        public int TotalMemoryKiB { get; set; } = 0;

        [NonSerialized]
        public int current_temperature = 0;
        [NonSerialized]
        public int current_humidity = 0;
        [NonSerialized]
        public int add_to_humidity = 0;
        [NonSerialized]
        public int add_to_temperature = 0;
        [NonSerialized]
        bool weatherUpdated = false;
        [NonSerialized]
        double latitude = 51.477928; // default to London Greenwich
        [NonSerialized]
        double longitude = -0.001545; // default to London Greenwich

        [NonSerialized]
        string city = String.Empty;

        [NonSerialized]
        bool owmEnabled = false;

        [NonSerialized]
        Random rnd = new Random();
        static System.Timers.Timer generateAnomaly = new System.Timers.Timer();
        static System.Timers.Timer resetPredictionTimer = new System.Timers.Timer(60 * 2 * 1000);
        static System.Timers.Timer updateWeather = new System.Timers.Timer(60 * 1000 * 15); // 15 minute

        public Telemetry()
        {
            var gcMemoryInfo = GC.GetGCMemoryInfo();
            var installedMemory = gcMemoryInfo.TotalAvailableMemoryBytes;
            // it will give the size of memory in MB, convert to KiB
            TotalMemoryKiB = (int)(installedMemory / 1024);
        }

        private void OnResetPrediction(object? sender, ElapsedEventArgs e)
        {
            Console.WriteLine("Resetting anomaly prediction");
            ResetPrediction();

            generateAnomaly.Interval = (rnd.Next(60 * 8) + 1) * 60 * 1000; // 1 and 480 minutes () on avg 240 minutes
            Console.WriteLine($"Anomaly timer set to {generateAnomaly.Interval / 1000 / 60} minutes");
            generateAnomaly.Enabled = true;
        }

        private void OnGenerateAnomaly(object? sender, ElapsedEventArgs e)
        {
            SetPrediction(1);
        }

        private void OnUpdateWeather(object? source, ElapsedEventArgs e)
        {
            UpdateWeather();
        }

        public string ToJson()
        {
            if (!weatherUpdated && owmEnabled)
            {
                UpdateWeather();
            }

            if (weatherUpdated)
            {
                Temperature = current_temperature + add_to_temperature;
                Humidity = current_humidity + add_to_humidity;
                Humidity = Humidity > 100 ? 100 : Humidity;
                Co2ppm = 445 + rnd.Next(10);
            }
            else
            {
                Temperature = 20 + rnd.Next(4);
                Humidity = 44 + rnd.Next(2);
                Co2ppm = 445 + rnd.Next(10);
            }

            return JsonConvert.SerializeObject(this);
        }


        private async Task<bool> GetLocation()
        {
            string data = String.Empty;
            var client = new HttpClient();
            try
            {
                HttpResponseMessage response = await client.GetAsync("https://get.geojs.io/v1/ip/geo.json");
                response.EnsureSuccessStatusCode();
                data = await response.Content.ReadAsStringAsync();

                var location = JsonConvert.DeserializeObject<Location>(data);
                if (location != null)
                {
                    city = location.City;
                    latitude = location.Latitude;
                    longitude = location.Longitude;
                }

                Console.WriteLine($"Location - City: {city}, Latitude: {latitude}, longitude: {longitude}");
            }
            catch (Exception ex)
            {
                System.Console.WriteLine(ex.Message);
                return false;
            }
            return true;
        }

        public void UpdateWeather()
        {
            var data = CurrentWeather.GetByCoordinates(latitude, longitude, "en", "metric");

            if (data.Item != null)
            {
                current_humidity = (int)data.Item.Humidity;
                current_temperature = (int)data.Item.Temp;
                Co2ppm = 400;
                weatherUpdated = true;
            }
        }

        private void ResetPrediction()
        {
            Prediction = 0;
            add_to_temperature = 0;
            add_to_humidity = 0;
        }

        private void SetPrediction(int prediction)
        {
            Console.WriteLine("Setting anomaly prediction to 1 'rattle'");

            Prediction = prediction;
            add_to_temperature = 5 + rnd.Next(15);
            add_to_humidity = 5 + rnd.Next(15);

            resetPredictionTimer.Enabled = true;
        }

        internal async Task<bool> Init(string owmKey)
        {
            if (!String.IsNullOrEmpty(owmKey))
            {
                owmEnabled = true;
                ClientSettings.ApiUrl = "http://api.openweathermap.org/data/2.5";
                ClientSettings.ApiKey = owmKey;
                await GetLocation();

                updateWeather.Elapsed += new ElapsedEventHandler(OnUpdateWeather);
                updateWeather.Enabled = true;
            }

            // Start the timers

            generateAnomaly.Elapsed += new ElapsedEventHandler(OnGenerateAnomaly);
            generateAnomaly.AutoReset = false;
            generateAnomaly.Interval = 1000 * 60 * (rnd.Next(15) + 1); // between 1 and 15 minutes
            Console.WriteLine($"Anomaly timer set to {generateAnomaly.Interval / 1000 / 60} minutes");
            generateAnomaly.Enabled = true;

            resetPredictionTimer.Elapsed += new ElapsedEventHandler(OnResetPrediction);
            resetPredictionTimer.AutoReset = false;

            return true;
        }
    }
}