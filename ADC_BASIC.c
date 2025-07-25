#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"

void app_main(void) {

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_DB_11);
    while (1) {
        int raw_value = adc1_get_raw(ADC1_CHANNEL_3);
        float voltage = (raw_value * 3.3)/4095.0;
        printf("Valor de ADC: %d, Voltage: %.2fV\n", raw_value, voltage);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}