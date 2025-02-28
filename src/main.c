#include "cJSON.h"
#include "driver/gpio.h"
#include "freertos/idf_additions.h"
#include "mqtt_client_wrapper.h"
#include "network_wrapper.h"
#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define BUILT_IN_LED GPIO_NUM_2

void app_main() {
  gpio_set_direction(BUILT_IN_LED, GPIO_MODE_OUTPUT);
  gpio_set_level(BUILT_IN_LED, 0);

  static mqtt_topic_t mqtt_topic = {
      .sub_topic = "topic/sub_test",
      .pub_topic = "topic/pub_test",
      .interval = 10, // interval for 5s
  };

  cJSON *json_data = cJSON_CreateObject();
  cJSON_AddStringToObject(json_data, "MCU", "esp32");
  cJSON_AddNumberToObject(json_data, "node", 1);
  cJSON_AddNumberToObject(json_data, "temperature", 30);

  mqtt_topic.msg = cJSON_Print(json_data);

  wifi_start();
  vTaskDelay(2000 / portTICK_PERIOD_MS);

  if (is_sta_connected()) {
    gpio_set_level(BUILT_IN_LED, 1);
    mqtt_start(&mqtt_topic);
    xTaskCreate(mqtt_pusblish_task, "publish task", 1024 * 2, &mqtt_topic, 2,
                NULL);
  } else {
    cJSON_Delete(json_data);
    cJSON_free(mqtt_topic.msg);
    gpio_set_level(BUILT_IN_LED, 0);
  }
}
