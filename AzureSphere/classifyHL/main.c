/* Copyright (c) Microsoft Corporation. All rights reserved.
 * Licensed under the MIT License.
 *
 * This example is built on the Azure Sphere DevX library.
 *   1. DevX is an Open Source community-maintained implementation of the Azure Sphere SDK samples.
 *   2. DevX is a modular library that simplifies common development scenarios.
 *        - You can focus on your solution, not the plumbing.
 *   3. DevX documentation is maintained at https://github.com/Azure-Sphere-DevX/AzureSphereDevX.Examples/wiki
 *
 * DEVELOPER BOARD SELECTION
 *
 * The following developer boards are supported.
 *
 *	 1. AVNET Azure Sphere Starter Kit.
 *   2. AVNET Azure Sphere Starter Kit Revision 2.
 *
 * ENABLE YOUR DEVELOPER BOARD
 *
 * Each Azure Sphere developer board manufacturer maps pins differently. You need to select the
 *    configuration that matches your board.
 *
 * Follow these steps:
 *
 *	 1. Open CMakeLists.txt.
 *	 2. Uncomment the set command that matches your developer board.
 *	 3. Click File, then Save to auto-generate the CMake Cache.
 *
 ************************************************************************************************/

#include "main.h"

/// <summary>
/// Handler to turn off LEDs
/// </summary>
/// <param name="eventLoopTimer"></param>
static DX_TIMER_HANDLER(azure_status_led_off_handler)
{
	dx_gpioOff(&gpio_network_led);
}
DX_TIMER_HANDLER_END

/// <summary>
/// Flash LEDs timer handler
/// </summary>
static DX_TIMER_HANDLER(azure_status_led_on_handler)
{
	static int init_sequence = 25;

	if (init_sequence-- > 0)
	{
		dx_gpioOn(&gpio_network_led);
		// on for 100ms off for 100ms = 200 ms in total
		dx_timerOneShotSet(&tmr_azure_status_led_on, &(struct timespec){0, 200 * ONE_MS});
		dx_timerOneShotSet(&tmr_azure_status_led_off, &(struct timespec){0, 100 * ONE_MS});
	}
	else if (azure_connected)
	{
		dx_gpioOff(&gpio_network_led);
		// dx_gpioOn(&gpio_network_led);
		// // off for 19990ms, on for 100ms = 4000ms in total
		// dx_timerOneShotSet(&tmr_azure_status_led_on, &(struct timespec){20, 0});
		// dx_timerOneShotSet(&tmr_azure_status_led_off, &(struct timespec){0, 10 * ONE_MS});
	}
	else
	{
		dx_gpioOn(&gpio_network_led);
		// on for 1000ms off for 1000ms = 2000 ms in total
		dx_timerOneShotSet(&tmr_azure_status_led_on, &(struct timespec){2, 0});
		dx_timerOneShotSet(&tmr_azure_status_led_off, &(struct timespec){1, 0});
	}
}
DX_TIMER_HANDLER_END

/// <summary>
/// Update the co2_alert_level from the device twin callback
/// </summary>
static DX_DEVICE_TWIN_HANDLER(set_co2_alert_level, deviceTwinBinding)
{
	if (IN_RANGE(*(int *)deviceTwinBinding->propertyValue, 0, 20000))
	{
		co2_alert_level = *(int *)deviceTwinBinding->propertyValue;
		update_co2_alert_status();
		dx_deviceTwinAckDesiredValue(deviceTwinBinding, deviceTwinBinding->propertyValue, DX_DEVICE_TWIN_RESPONSE_COMPLETED);
	}
	else
	{
		dx_deviceTwinAckDesiredValue(deviceTwinBinding, deviceTwinBinding->propertyValue, DX_DEVICE_TWIN_RESPONSE_ERROR);
	}
}
DX_DEVICE_TWIN_HANDLER_END

static DX_DEVICE_TWIN_HANDLER(set_device_altitude, deviceTwinBinding)
{
	if (IN_RANGE(*(int *)deviceTwinBinding->propertyValue, 0, 10000))
	{
		co2_set_altitude(*(int *)deviceTwinBinding->propertyValue);
		dx_deviceTwinAckDesiredValue(deviceTwinBinding, deviceTwinBinding->propertyValue, DX_DEVICE_TWIN_RESPONSE_COMPLETED);
	}
	else
	{
		dx_deviceTwinAckDesiredValue(deviceTwinBinding, deviceTwinBinding->propertyValue, DX_DEVICE_TWIN_RESPONSE_ERROR);
	}
}
DX_DEVICE_TWIN_HANDLER_END

/// <summary>
/// Handler called every 20 seconds
/// The CO2 sensor and Avnet Onboard sensors are read
/// Then reload 20 second oneshot timer
/// </summary>
/// <param name="eventLoopTimer"></param>
static DX_TIMER_HANDLER(read_telemetry_handler)
{
#define UPDATE_LATEST(name) telemetry.latest.name = current.name > telemetry.latest.name ? current.name : telemetry.latest.name;

	SENSOR current;
	memset(&current, 0x00, sizeof(SENSOR));

	if (co2_read(&telemetry))
	{
		// clang-format off
        telemetry.valid =
            IN_RANGE(telemetry.latest.temperature, -20, 60) &&
            IN_RANGE(telemetry.latest.humidity, 0, 100) &&
            IN_RANGE(telemetry.latest.co2ppm, 0, 20000);
		// clang-format on
	}
	else
	{
		telemetry.valid = false;
	}

	if (telemetry.valid)
	{
		UPDATE_LATEST(temperature);
		UPDATE_LATEST(humidity);
		UPDATE_LATEST(co2ppm);

		update_co2_alert_status();
	}
}
DX_TIMER_HANDLER_END

/// <summary>
/// Publish HVAC telemetry
/// </summary>
/// <param name="eventLoopTimer"></param>
static DX_TIMER_HANDLER(publish_telemetry_handler)
{
	publish_telemetry();
}
DX_TIMER_HANDLER_END

DX_TIMER_HANDLER(status_rgb_off_handler)
{
	dx_pwmSetDutyCycle(&pwm_led_red, 1000, 100);
	dx_pwmSetDutyCycle(&pwm_led_green, 1000, 100);
}
DX_TIMER_HANDLER_END

DX_TIMER_HANDLER(read_accelerometer_handler)
{
#ifdef OEM_AVNET
	float xx, yy, zz;

	avnet_get_acceleration(&xx, &yy, &zz);

	x += xx;
	y += yy;
	z += zz;
	x /= 2;
	y /= 2;
	z /= 2;

	intercore_ml_classify_block.x = x;
	intercore_ml_classify_block.y = y;
	intercore_ml_classify_block.z = z;

	dx_intercorePublish(&intercore_ml_classify_ctx, &intercore_ml_classify_block, sizeof(intercore_ml_classify_block));

	dx_timerOneShotSet(&tmr_read_accelerometer, &(struct timespec){0, 10 * ONE_MS});
#endif // OEM_AVNET
}
DX_TIMER_HANDLER_END

DX_TIMER_HANDLER(reset_demo_handler)
{
	if (azure_connected)
	{
		dx_deviceTwinReportValue(&dt_prediction, "normal");
	}

	dx_gpioOn(&gpio_relay_ext);
	dx_gpioOff(&gpio_relay_2);
}
DX_TIMER_HANDLER_END

/// <summary>
/// Handler to check for Button Presses
/// </summary>
static DX_TIMER_HANDLER(control_button_handler)
{
	static GPIO_Value_Type buttonAState;
	static GPIO_Value_Type buttonBState;

	if (dx_gpioStateGet(&gpio_buttonA, &buttonAState))
	{
		reset_demo();
	}

	if (dx_gpioStateGet(&gpio_buttonB, &buttonBState))
	{
		if (azure_connected)
		{
			dx_deviceTwinReportValue(&dt_prediction, "normal");
		}

		reset_demo();
	}
}
DX_TIMER_HANDLER_END

DX_TIMER_HANDLER(reset_prediction_handler)
{
	latest_prediction = 0; // normal operation
	dx_gpioOff(&gpio_app_status_led);
	dx_gpioOn(&gpio_relay_ext);
}
DX_TIMER_HANDLER_END

static void publish_telemetry(void)
{
	int temperature, humidity;

	if (telemetry.valid && azure_connected)
	{
		telemetry.latest.prediction = latest_prediction;

		switch (latest_prediction)
		{
			case 1: // Rattle
				temperature = telemetry.latest.temperature + 15;
				humidity    = telemetry.latest.humidity + 15;
				break;
			case 3: // bearing
				humidity    = telemetry.latest.humidity + 25;
				temperature = telemetry.latest.temperature + 20;
				break;
			default: // normal
				humidity    = telemetry.latest.humidity;
				temperature = telemetry.latest.temperature;
				break;
		}

		humidity = humidity > 100 ? 100 : humidity;

		// clang-format off
        // Serialize telemetry as JSON
        if (dx_jsonSerialize(msgBuffer, sizeof(msgBuffer), 6,
            DX_JSON_INT, "co2ppm", telemetry.latest.co2ppm,
            DX_JSON_INT, "humidity", humidity,
            DX_JSON_INT, "temperature", temperature,
            DX_JSON_INT, "prediction", telemetry.latest.prediction,
            DX_JSON_INT, "peakUserMemoryKiB", (int)Applications_GetPeakUserModeMemoryUsageInKB(),
            DX_JSON_INT, "totalMemoryKiB", (int)Applications_GetTotalMemoryUsageInKB()))
		// clang-format on
		{
			dx_Log_Debug("%s\n", msgBuffer);

			// Publish telemetry message to IoT Hub/Central
			dx_azurePublish(msgBuffer, strlen(msgBuffer), messageProperties, NELEMS(messageProperties), &contentProperties);
		}
		else
		{
			dx_Log_Debug("JSON Serialization failed: Buffer too small\n");
			dx_terminate(APP_ExitCode_Telemetry_Buffer_Too_Small);
		}
		// reset latest sensor telemetry
		// memset(&telemetry.latest, 0x00, sizeof(SENSOR));
	}
}

static void update_co2_alert_status(void)
{
	dx_pwmSetDutyCycle(&pwm_led_red, 1000, 100);
	dx_pwmSetDutyCycle(&pwm_led_green, 1000, 100);

	if (!telemetry.valid)
	{
		Log_Debug("Invalid telemetry\n");
		return;
	}

	// Light level is a percentage. 0% is dark, 100% very bright
	// Calculate brightness. LED brightness is inverted, 100% duty cycle is off, 0% duty cycle is full on
	int light_level     = avnet_get_light_level();
	uint32_t brightness = 100 - (uint32_t)(light_level > 100 ? 100 : light_level);
	brightness          = brightness == 100 ? 99 : brightness;

	if (telemetry.latest.co2ppm > co2_alert_level)
	{
		dx_pwmSetDutyCycle(&pwm_led_red, 1000, brightness);
	}
	else
	{
		dx_pwmSetDutyCycle(&pwm_led_green, 1000, brightness);
	}
	dx_timerOneShotSet(&tmr_status_rgb_off, &(struct timespec){0, 100 * ONE_MS});
}

/// <summary>
/// Callback handler for Asynchronous Inter-Core Messaging Pattern
/// </summary>
void intercore_classify_response_handler(void *data_block, ssize_t message_length)
{
	INTERCORE_PREDICTION_BLOCK_T *ic_message_block = (INTERCORE_PREDICTION_BLOCK_T *)data_block;
	char *predictions[]                            = {"normal", "rattle", "updown", "bearings", "anomaly"};

	switch (ic_message_block->cmd)
	{
		case IC_PREDICTION:
			memcpy(prediction, ic_message_block->PREDICTION, sizeof(prediction));

			if (latest_prediction != 0 || !strncmp(prediction, "anomaly", sizeof(prediction)))
			{
				return;
			}

			Log_Debug("Prediction %s\n", ic_message_block->PREDICTION);

			// If prediction not "normal"
			if (strncmp(prediction, predictions[0], sizeof(prediction)))
			{
				if (azure_connected)
				{
					dx_deviceTwinReportValue(&dt_prediction, prediction);
				}

				for (size_t i = 0; i < sizeof(predictions) / sizeof(char *); i++)
				{
					if (!strncmp(prediction, predictions[i], sizeof(prediction)))
					{
						latest_prediction = (int)i;
						dx_gpioOn(&gpio_app_status_led);
						dx_gpioOff(&gpio_relay_ext);
                        publish_telemetry();
						dx_timerOneShotSet(&tmr_reset_prediction, &(struct timespec){(4 * 60), 0});
						break;
					}
				}
			}

			break;
		default:
			break;
	}
}

static void reset_demo(void)
{
	dx_timerOneShotSet(&tmr_reset_demo, &(struct timespec){0, 0});
	dx_gpioOff(&gpio_app_status_led);
	dx_gpioOn(&gpio_relay_ext);
	latest_prediction = 0;
    dx_deviceTwinReportValue(&dt_prediction, "normal");
}

static void ConnectionStatus(bool connected)
{
	azure_connected = connected;
}

static void report_startup(bool connected)
{
	dx_deviceTwinReportValue(&dt_utc_startup, dx_getCurrentUtc(msgBuffer, sizeof(msgBuffer)));
	snprintf(msgBuffer, sizeof(msgBuffer), "Sample version: %s, DevX version: %s", SAMPLE_VERSION_NUMBER, AZURE_SPHERE_DEVX_VERSION);
	dx_deviceTwinReportValue(&dt_hvac_sw_version, msgBuffer);
	// now unregister this callback as we've reported startup time and sw version
	dx_azureUnregisterConnectionChangedNotification(report_startup);
}

/// <summary>
///  Initialize peripherals, device twins, direct methods, timer_binding_sets.
/// </summary>
static void InitPeripheralsAndHandlers(void)
{
	int intercore_retry = 0;

	dx_Log_Debug_Init(Log_Debug_Time_buffer, sizeof(Log_Debug_Time_buffer));
	dx_azureConnect(&dx_config, NETWORK_INTERFACE, NULL);
	dx_gpioSetOpen(gpio_binding_sets, NELEMS(gpio_binding_sets));
	dx_pwmSetOpen(pwm_bindings, NELEMS(pwm_bindings));
	dx_i2cSetOpen(i2c_bindings, NELEMS(i2c_bindings));
	dx_deviceTwinSubscribe(device_twin_bindings, NELEMS(device_twin_bindings));

	// Onboard LEDs are wired such that high voltage is off, low is on
	// Slightly unintuitive, but a 100% duration cycle turns the LED off
	dx_pwmSetDutyCycle(&pwm_led_red, 1000, 100);
	dx_pwmSetDutyCycle(&pwm_led_green, 1000, 100);
	dx_pwmSetDutyCycle(&pwm_led_blue, 1000, 100);

	co2_initialize(isu2_i2c.fd);
	avnet_imu_initialize(isu2_i2c.fd);
	avnet_get_temperature_lps22h(); // This is a hack to initialize the accelerometer :)

	avnet_open_adc(ADC_CHANNEL);

	while (++intercore_retry < 10)
	{
		if (dx_intercoreConnect(&intercore_ml_classify_ctx))
		{
			break;
		}
		else
		{
			nanosleep(&(struct timespec){0, 500 * ONE_MS}, NULL);
		}
	}
	
	if (intercore_retry == 10)
	{
		dx_terminate(APP_EXITCode_Intercore_connection_failed);
		return;
	}

	dx_azureRegisterConnectionChangedNotification(report_startup);
	dx_azureRegisterConnectionChangedNotification(ConnectionStatus);

	dx_timerSetStart(timer_binding_sets, NELEMS(timer_binding_sets));

	telemetry.previous.temperature = telemetry.previous.prediction = telemetry.previous.humidity = telemetry.previous.co2ppm = INT32_MAX;
}

/// <summary>
///     Close peripherals and handlers.
/// </summary>
static void ClosePeripheralsAndHandlers(void)
{
	dx_deviceTwinUnsubscribe();
	dx_gpioSetClose(gpio_binding_sets, NELEMS(gpio_binding_sets));
	dx_azureToDeviceStop();

	dx_gpioSetClose(gpio_binding_sets, NELEMS(gpio_binding_sets));
	dx_i2cSetClose(i2c_bindings, NELEMS(i2c_bindings));
	dx_pwmSetClose(pwm_bindings, NELEMS(pwm_bindings));
}

int main(int argc, char *argv[])
{
	dx_registerTerminationHandler();

	if (!dx_configParseCmdLineArguments(argc, argv, &dx_config))
	{
		return dx_getTerminationExitCode();
	}

	InitPeripheralsAndHandlers();

	// Main loop
	dx_eventLoopRun();

	ClosePeripheralsAndHandlers();
	Log_Debug("Application exiting.\n");
	return dx_getTerminationExitCode();
}