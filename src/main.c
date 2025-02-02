#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mq2_module.h"
#include "pir_module.h"

void app_main()
{
    bool use_mq2 = true;
    if (use_mq2)
    {
        xTaskCreate(mq2_task, "mq2_task", 2048, NULL, 5, NULL);
        //tst
    }
    else
    {
        xTaskCreate(pir_task, "pir_task", 2048, NULL, 5, NULL);
    }
}