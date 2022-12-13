#pragma once

#include "app_exit_codes.h"
#include "hw/predictive_maintenance.h"

// #include "co2_sensor.h"
#include "co2_sensor.h"
#include "dx_azure_iot.h"
#include "dx_config.h"
#include "dx_i2c.h"
#include "dx_intercore.h"
#include "dx_json_serializer.h"
#include "dx_pwm.h"
#include "dx_terminate.h"
#include "dx_utilities.h"
#include "dx_version.h"
#include "imu_temp_pressure.h"
#include "intercore_contract.h"
#include "light_sensor.h"
#include <applibs/applications.h>
#include <applibs/log.h>

#define CORE_ML_CLASSIFY_COMPONENT_ID "af8b26db-355e-405c-bbde-3b851668ee23"

typedef enum
{
	MOTOR_STOP,
	MOTOR_FORWARD,
	MOTOR_REVERSE,
	MOTOR_SLEEP
} MOTOR_ACTION;

MOTOR_ACTION motor_action;

// https://docs.microsoft.com/en-us/azure/iot-pnp/overview-iot-plug-and-play
#define IOT_PLUG_AND_PLAY_MODEL_ID "dtmi:AzureSphere:PredictiveMaintenance;1"
#define NETWORK_INTERFACE          "wlan0"
#define SAMPLE_VERSION_NUMBER      "1.1"

#define Log_Debug(f_, ...) dx_Log_Debug((f_), ##__VA_ARGS__)
#define JSON_MESSAGE_BYTES 256

// Variables
static char Log_Debug_Time_buffer[128];
static char msgBuffer[JSON_MESSAGE_BYTES] = {0};
static DX_USER_CONFIG dx_config;
static bool azure_connected;
static float x, y, z;
ENVIRONMENT telemetry;
static int32_t co2_alert_level = 1000;
static int latest_prediction = 0;

static char prediction[20] = {'n', 'o', 'r', 'm', 'a', 'l'};
static INTERCORE_ML_CLASSIFY_BLOCK_T intercore_ml_classify_block;

// Forward declarations
static DX_DECLARE_DEVICE_TWIN_HANDLER(set_co2_alert_level);
static DX_DECLARE_DEVICE_TWIN_HANDLER(set_device_altitude);
static DX_DECLARE_TIMER_HANDLER(azure_status_led_off_handler);
static DX_DECLARE_TIMER_HANDLER(azure_status_led_on_handler);
static DX_DECLARE_TIMER_HANDLER(control_button_handler);
static DX_DECLARE_TIMER_HANDLER(publish_telemetry_handler);
static DX_DECLARE_TIMER_HANDLER(read_accelerometer_handler);
static DX_DECLARE_TIMER_HANDLER(read_telemetry_handler);
static DX_DECLARE_TIMER_HANDLER(reset_demo_handler);
static DX_DECLARE_TIMER_HANDLER(reset_prediction_handler);
static DX_DECLARE_TIMER_HANDLER(status_rgb_off_handler);
static void intercore_classify_response_handler(void *data_block, ssize_t message_length);
static void reset_demo(void);
static void update_co2_alert_status(void);
static void publish_telemetry(void);

/***********************************************************************************************************
 * Common content properties for publish messages to IoT Hub/Central
 **********************************************************************************************************/

/// <summary>
/// Publish sensor telemetry using the following properties for efficient IoT Hub routing
/// https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-devguide-messages-d2c
/// </summary>
static DX_MESSAGE_PROPERTY *messageProperties[] = {&(DX_MESSAGE_PROPERTY){.key = "appid", .value = "co2monitor"},
	&(DX_MESSAGE_PROPERTY){.key = "type", .value = "telemetry"}, &(DX_MESSAGE_PROPERTY){.key = "schema", .value = "1"}};

static DX_MESSAGE_CONTENT_PROPERTIES contentProperties = {.contentEncoding = "utf-8", .contentType = "application/json"};

// Bindings

DX_INTERCORE_BINDING intercore_ml_classify_ctx = {.sockFd = -1,
	.nonblocking_io                                       = true,
	.rtAppComponentId                                     = CORE_ML_CLASSIFY_COMPONENT_ID,
	.interCoreCallback                                    = intercore_classify_response_handler,
	.intercore_recv_block                                 = &intercore_ml_classify_block,
	.intercore_recv_block_length                          = sizeof(intercore_ml_classify_block)};

// clang-format on

static DX_DEVICE_TWIN_BINDING dt_hvac_sw_version     = {.propertyName = "SoftwareVersion", .twinType = DX_DEVICE_TWIN_STRING};
static DX_DEVICE_TWIN_BINDING dt_utc_startup         = {.propertyName = "StartupUtc", .twinType = DX_DEVICE_TWIN_STRING};
static DX_DEVICE_TWIN_BINDING dt_prediction          = {.propertyName = "FaultPrediction", .twinType = DX_DEVICE_TWIN_STRING};
static DX_DEVICE_TWIN_BINDING dt_altitude_in_meters  = {.propertyName = "AltitudeInMeters", .twinType = DX_DEVICE_TWIN_INT, .handler = set_device_altitude};
static DX_DEVICE_TWIN_BINDING dt_co2_ppm_alert_level = {.propertyName = "AlertLevel", .twinType = DX_DEVICE_TWIN_INT, .handler = set_co2_alert_level};

static DX_GPIO_BINDING gpio_network_led = {
	.pin = NETWORK_CONNECTED_LED, .name = "network_led", .direction = DX_OUTPUT, .initialState = GPIO_Value_Low, .invertPin = true};
static DX_GPIO_BINDING gpio_app_status_led = {
	.pin = APP_STATUS_LED, .name = "app_status_led", .direction = DX_OUTPUT, .initialState = GPIO_Value_Low, .invertPin = true};
static DX_GPIO_BINDING gpio_buttonA = {.pin = BUTTON_A, .name = "buttonA", .direction = DX_INPUT, .detect = DX_GPIO_DETECT_LOW};
static DX_GPIO_BINDING gpio_buttonB = {.pin = BUTTON_B, .name = "buttonB", .direction = DX_INPUT, .detect = DX_GPIO_DETECT_LOW};
static DX_GPIO_BINDING gpio_relay_ext = {.pin = RELAY_EXT, .name = "gpio_relay_1", .direction = DX_OUTPUT, .initialState = GPIO_Value_High, .invertPin = false};
static DX_GPIO_BINDING gpio_relay_2 = {.pin = RELAY_2, .name = "gpio_relay_2", .direction = DX_OUTPUT, .initialState = GPIO_Value_Low, .invertPin = false};

// MikroE Click H-BRIDGE Pins
static DX_GPIO_BINDING gpio_en  = {.pin = EN, .name = "EN", .direction = DX_OUTPUT, .initialState = GPIO_Value_Low, .invertPin = false};
static DX_GPIO_BINDING gpio_nsl = {.pin = NSL, .name = "NSL", .direction = DX_OUTPUT, .initialState = GPIO_Value_Low, .invertPin = false};
static DX_GPIO_BINDING gpio_ph  = {.pin = PH, .name = "PH", .direction = DX_OUTPUT, .initialState = GPIO_Value_Low, .invertPin = false};

static DX_TIMER_BINDING tmr_azure_status_led_off = {.name = "tmr_azure_status_led_off", .handler = azure_status_led_off_handler};
static DX_TIMER_BINDING tmr_azure_status_led_on  = {.repeat = &(struct timespec){0, 500 * ONE_MS}, .name = "tmr_azure_status_led_on", .handler = azure_status_led_on_handler};
static DX_TIMER_BINDING tmr_control_button = {.repeat = &(struct timespec){0, 100 * ONE_MS}, .name = "tmr_control_button", .handler = control_button_handler};
static DX_TIMER_BINDING tmr_publish_telemetry  = {.repeat = &(struct timespec){10, 0}, .name = "tmr_publish_telemetry", .handler = publish_telemetry_handler};
static DX_TIMER_BINDING tmr_read_accelerometer = {.delay = &(struct timespec){1, 0}, .name = "tmr_read_accelerometer", .handler = read_accelerometer_handler};
static DX_TIMER_BINDING tmr_read_telemetry     = {.repeat = &(struct timespec){15, 0}, .name = "tmr_read_telemetry", .handler = read_telemetry_handler};
static DX_TIMER_BINDING tmr_reset_demo         = {.name = "tmr_reset_demo", .handler = reset_demo_handler};
static DX_TIMER_BINDING tmr_reset_prediction     = {.name = "tmr_reset_prediction", .handler = reset_prediction_handler};
static DX_TIMER_BINDING tmr_status_rgb_off     = {.name = "tmr_status_rgb_off", .handler = status_rgb_off_handler};

/***********************************************************************************************************
 * declare pwm bindings
 **********************************************************************************************************/

static DX_PWM_CONTROLLER pwm_rgb_controller = {.controllerId = PWM_RGB_CONTROLLER, .name = "PWM RBG Controller"};

static DX_PWM_BINDING pwm_led_red   = {.pwmController = &pwm_rgb_controller, .channelId = 0, .name = "pwm red led"};
static DX_PWM_BINDING pwm_led_green = {.pwmController = &pwm_rgb_controller, .channelId = 1, .name = "pwm green led"};
static DX_PWM_BINDING pwm_led_blue  = {.pwmController = &pwm_rgb_controller, .channelId = 2, .name = "pwm blue led"};

// clang-format on

/***********************************************************************************************************
 * declare i2c bindings
 **********************************************************************************************************/

static DX_I2C_BINDING isu2_i2c = {.interfaceId = ISU2_I2C, .speedInHz = I2C_BUS_SPEED_STANDARD, .name = "CO2 and Avnet onboard sensors"};

static DX_PWM_BINDING *pwm_bindings[] = {&pwm_led_green, &pwm_led_red, &pwm_led_blue};

// All bindings referenced are initialised in the InitPeripheralsAndHandlers function
DX_I2C_BINDING *i2c_bindings[] = {
	&isu2_i2c,
};

DX_DEVICE_TWIN_BINDING *device_twin_bindings[] = {
	&dt_altitude_in_meters,
	&dt_co2_ppm_alert_level,
	&dt_hvac_sw_version,
	&dt_prediction,
	&dt_utc_startup,

};

DX_DIRECT_METHOD_BINDING *direct_method_binding_sets[] = {};

DX_GPIO_BINDING *gpio_binding_sets[] = {
	&gpio_app_status_led,
	&gpio_buttonA,
	&gpio_buttonB,
	&gpio_en,
	&gpio_network_led,
	&gpio_nsl,
	&gpio_ph,
	&gpio_relay_ext,
	&gpio_relay_2,
};
DX_TIMER_BINDING *timer_binding_sets[] = {
	&tmr_azure_status_led_off,
	&tmr_azure_status_led_on,
	&tmr_control_button,
	&tmr_publish_telemetry,
	&tmr_read_accelerometer,
	&tmr_read_telemetry,
	&tmr_reset_demo,
	&tmr_status_rgb_off,
	&tmr_reset_prediction,
};
