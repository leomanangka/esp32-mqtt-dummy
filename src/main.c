#include "cJSON.h"
#include "driver/gpio.h"
#include "freertos/idf_additions.h"
#include "mqtt_client_wrapper.h"
#include "network_wrapper.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define BUILT_IN_LED GPIO_NUM_2

static float rand_number(float min, float max) {
  float scale = (float)rand() / RAND_MAX;
  return min + scale * (max - min);
}

static void publish_task(void *arg) {
  mqtt_topic_t *mqtt_topic = (mqtt_topic_t *)arg;
  static int interval = 1000;
  if (mqtt_topic->interval != 0) {
    interval = interval * mqtt_topic->interval;
  }

  gpio_set_direction(BUILT_IN_LED, GPIO_MODE_OUTPUT);
  cJSON *json_data = cJSON_CreateObject();
  while (true) {
    gpio_set_level(BUILT_IN_LED, 0);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    float temperature_rng = rand_number(-40, 80);
    char temperature[128];
    snprintf(temperature, sizeof(temperature), "%.2f", temperature_rng);

    float humidity_rng = rand_number(0, 100);
    char humidity[128];
    snprintf(humidity, sizeof(humidity), "%.2f", humidity_rng);

    float water_level_rng = rand_number(0, 100);
    char waterl_level[128];
    snprintf(waterl_level, sizeof(waterl_level), "%.2f", water_level_rng);

    cJSON_AddNumberToObject(json_data, "temperature", atof(temperature));
    cJSON_AddNumberToObject(json_data, "humidity", atof(humidity));
    cJSON_AddNumberToObject(json_data, "water_level", atoi(waterl_level));

    mqtt_topic->msg = cJSON_Print(json_data);

    mqtt_pusblish(mqtt_topic);
    gpio_set_level(BUILT_IN_LED, 1);
    vTaskDelay(interval / portTICK_PERIOD_MS);
  }
  cJSON_Delete(json_data);
  cJSON_free(mqtt_topic->msg);
  gpio_set_level(BUILT_IN_LED, 0);
}

void app_main() {
  static mqtt_topic_t mqtt_topic = {
      .sub_topic = "topic/sub_test",
      .pub_topic = "topic/pub_test",
      .interval = 10, // interval for 10s
  };

  wifi_start();
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  if (is_sta_connected()) {
    mqtt_start(&mqtt_topic);
    xTaskCreate(publish_task, "Publish Task", 2048 * 2, &mqtt_topic, 1, NULL);
  }
}
