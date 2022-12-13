/* Copyright (c) Microsoft Corporation. All rights reserved.
   Licensed under the MIT License. */

#pragma once

#include <stdbool.h>
#include "dx_i2c.h"
// #include "Onboard/onboard_sensors.h"

#ifdef SCD30
#include "Drivers/EmbeddedScd30/scd30/scd30.h"
#include <math.h> 
#else
#include "scd4x_i2c.h"
#include "sensirion_i2c_hal.h"
#include "sensirion_common.h"
#endif

enum PREDICTION {
   NORMAL,
   RATTLE,
   UPDOWN,
   BEARINGS,
   ANOMALY
};


typedef struct {
    int temperature;
    int humidity;
    int co2ppm;
    int light;
    enum PREDICTION prediction;
} SENSOR;

typedef struct {
    SENSOR latest;
    SENSOR previous;
    bool updated;
    bool valid;
} ENVIRONMENT;

/// <summary>
/// Initialize the CO2 sensor
/// </summary>
/// <param name=""></param>
/// <returns></returns>
bool co2_initialize(int fd);

/// <summary>
/// Read CO2 sensor telemetry
/// </summary>
/// <param name="telemetry"></param>
/// <returns></returns>
bool co2_read(ENVIRONMENT *telemetry);

/// <summary>
/// Set the CO2 sensor altitude
/// </summary>
/// <param name="altitude_in_meters"></param>
/// <returns></returns>
bool co2_set_altitude(int altitude_in_meters);
