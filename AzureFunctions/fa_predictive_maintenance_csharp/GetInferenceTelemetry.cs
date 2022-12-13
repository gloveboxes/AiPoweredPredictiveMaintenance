using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Azure.WebJobs.Extensions.Http;
using Microsoft.Azure.WebJobs;
using System.Collections.Generic;

namespace Glovebox.Function
{
    public static class GetInferenceTelemetry
    {
        [FunctionName("GetInferenceTelemetry")]
        public static IActionResult Run(
            [HttpTrigger(AuthorizationLevel.Function, "get", "post", Route = "getinferencetelemetry/{deviceId}/{numberOfDays}")]
            HttpRequest req,
            [Sql("dbo.GetInferenceData",
                CommandType = System.Data.CommandType.StoredProcedure,
                Parameters = "@deviceId={deviceId},@numberOfDays={numberOfDays}",
                ConnectionStringSetting = "SqlConnectionString")]
            IEnumerable<InferenceItems> inferenceItems)
        {
            return new OkObjectResult(inferenceItems);
        }
    }
}
