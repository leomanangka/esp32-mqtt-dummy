#include "cJSON.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "mqtt_client_wrapper.h"
#include "network_wrapper.h"
#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"

void app_main() {
  static const char *MAIN_TAG = "[APP MAIN]";
  static mqtt_topic_t mqtt_topic = {
      .sub_topic = "topic/sub_test",
      .pub_topic = "topic/pub_test",
  };

  cJSON *json_data = cJSON_CreateObject();
  cJSON_AddStringToObject(json_data, "MCU", "esp32");
  cJSON_AddNumberToObject(json_data, "node", 1);
  cJSON_AddNumberToObject(json_data, "temperature", 30);

  mqtt_topic.msg = cJSON_Print(json_data);

  wifi_start();
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  mqtt_start(&mqtt_topic);
  xTaskCreate(mqtt_pusblish_task, "publish task", 1024 * 2, &mqtt_topic, 2,
              NULL);
}
