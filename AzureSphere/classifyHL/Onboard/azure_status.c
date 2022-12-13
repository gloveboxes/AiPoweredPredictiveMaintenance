/* Copyright (c) Microsoft Corporation. All rights reserved.
   Licensed under the MIT License. */

#include "azure_status.h"

/// <summary>
/// Handler to turn off LEDs
/// </summary>
/// <param name="eventLoopTimer"></param>
DX_TIMER_HANDLER(azure_status_led_off_handler)
{
    dx_gpioOff(&gpio_network_led);
}
DX_TIMER_HANDLER_END

/// <summary>
/// Flash LEDs timer handler
/// </summary>
DX_TIMER_HANDLER(azure_status_led_on_handler)
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