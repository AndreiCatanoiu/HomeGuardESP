#ifndef __MQ2_MODULE_H__
#define __MQ2_MODULE_H__

#define MQ2_SENSOR_DIGITAL_GPIO GPIO_NUM_18
#define MQ2_SENSOR_ANALOG_GPIO GPIO_NUM_34
#define SAFE_LEVEL 1000
#define CAUTION_LEVEL 2000
#define WARNING_LEVEL 3000

void mq2_config();
void mq2_process_data();

#endif