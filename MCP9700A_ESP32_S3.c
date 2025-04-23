#include <stdio.h>
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MCP9700A_OFFSET         500.0f          // 500 mV a 0°C
#define MCP9700A_TC             10.0f           // 10 mV/°C

adc_oneshot_unit_handle_t adc_handle = NULL;        //Para conffigurar el ADC en modo Oneshot
adc_cali_handle_t cali_handle = NULL;
bool calibrated = false;
int voltage_mv;

static void adc_calibration(void)
{
    adc_cali_scheme_ver_t scheme_mask;
    esp_err_t ret = adc_cali_check_scheme(&scheme_mask);

    if (ret == ESP_OK) 
    { 
        if (scheme_mask & ADC_CALI_SCHEME_VER_CURVE_FITTING)
        {   
            adc_cali_curve_fitting_config_t cali_cfg = {
                .unit_id = ADC_UNIT_1,
                .chan = ADC_CHANNEL_3,
                .atten = ADC_ATTEN_DB_12,
                .bitwidth = ADC_BITWIDTH_DEFAULT,
            };

            if (adc_cali_create_scheme_curve_fitting(&cali_cfg, &cali_handle) == ESP_OK)
                calibrated = true;             
        }       
    } 
}

static esp_err_t adc_oneshot_init(void)
{
    // Configuración del ADC
    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };

    esp_err_t ret = adc_oneshot_new_unit(&init_cfg, &adc_handle);
    if (ret != ESP_OK)
        return ret;     // Fallo al inicializar ADC
    
    // Configura el canal del ADC
    adc_oneshot_chan_cfg_t chan_cfg = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12
    };
    ret = adc_oneshot_config_channel(adc_handle, ADC_CHANNEL_3, &chan_cfg);
    
    if (ret != ESP_OK)
        adc_oneshot_del_unit(adc_handle);       //Liberar recursos del ADC si falla

    return ret;
}

float read_temperature()
{
    int adc_raw;
  
    esp_err_t ret = adc_oneshot_read(adc_handle, ADC_CHANNEL_3, &adc_raw);
    if (ret == ESP_OK)
    {
       if(calibrated)
       {
            if (adc_cali_raw_to_voltage(cali_handle, adc_raw, &voltage_mv) == ESP_OK)
            {
                float temperature = (voltage_mv - MCP9700A_OFFSET) / MCP9700A_TC; 
                return temperature;
            }
            
       }
    }
    return 0;
}

void app_main(void)
{
    adc_oneshot_init();
    adc_calibration();

    while (1)
    {
        float temp = read_temperature();
        printf("Voltaje %d Temperatura:  %.2f °C\n", voltage_mv, temp);
        vTaskDelay(pdMS_TO_TICKS(1000));
        
    }
    
    adc_oneshot_del_unit(adc_handle);
    if (calibrated)
        adc_cali_delete_scheme_curve_fitting(cali_handle);

}
