using System;

namespace Glovebox.Function
{
    public class InferenceItems
    {
        public DateTime timestamp { get; set; }
        public int temperature { get; set; }
        public int humidity { get; set; }
        public int prediction { get; set; }
    }

    public class Telemetry
    {
        public string deviceId { get; set; }
        public DateTime timestamp { get; set; }
        public int co2ppm { get; set; }
        public int humidity { get; set; }
        public int temperature { get; set; }
        public int prediction { get; set; }
        public int peakUserMemoryKiB { get; set; }
        public int totalMemoryKiB { get; set; }

    }

}