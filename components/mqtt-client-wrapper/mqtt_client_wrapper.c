#include "mqtt_client_wrapper.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static const char *MQTT_TAG = "MQTT";
bool MQTT_CONNECTED_FLAG = false;

static void log_error_if_nonzero(const char *message, int error_code) {
  if (error_code != 0) {
    ESP_LOGE(MQTT_TAG, "Last error %s: 0x%x", message, error_code);
  }
}

static void event_handler(void *args, esp_event_base_t event_base,
                          int32_t event_id, void *event_data) {
  esp_mqtt_event_handle_t event = event_data;
  mqtt_topic_t *mqtt_topic = (mqtt_topic_t *)args;
  esp_mqtt_client_handle_t client = event->client;
  switch ((esp_mqtt_event_id_t)event_id) {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(MQTT_TAG, "MQTT CONNECTED");
    esp_mqtt_client_subscribe(client, mqtt_topic->sub_topic, 1);
    MQTT_CONNECTED_FLAG = true;
    break;
  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGI(MQTT_TAG, "MQTT DISCONNECTED");
    MQTT_CONNECTED_FLAG = false;
    break;
  case MQTT_EVENT_SUBSCRIBED:
    ESP_LOGI(MQTT_TAG, "MQTT SUBSCRIBE, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_UNSUBSCRIBED:
    ESP_LOGI(MQTT_TAG, "MQTT UNSUBSCRIBE, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_PUBLISHED:
    ESP_LOGI(MQTT_TAG, "MQTT PUBLISHED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_DATA:
    ESP_LOGI(MQTT_TAG, "MQTT EVENT DATA");
    printf("TOPIC:%.*s\r\n", event->topic_len, event->topic);
    printf("DATA:%.*s\r\n", event->data_len, event->data);
    break;
  case MQTT_EVENT_ERROR:
    ESP_LOGI(MQTT_TAG, "MQTT ERROR");
    if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
      log_error_if_nonzero("reported from esp-tls",
                           event->error_handle->esp_tls_last_esp_err);
      log_error_if_nonzero("reported from tls stack",
                           event->error_handle->esp_tls_stack_err);
      log_error_if_nonzero("captured as transport's socket errno",
                           event->error_handle->esp_transport_sock_errno);
      ESP_LOGI(MQTT_TAG, "Last errno string (%s)",
               strerror(event->error_handle->esp_transport_sock_errno));
    }
    break;
  default:
    ESP_LOGI(MQTT_TAG, "OTHER EVENT ID: %d", event->event_id);
    break;
  }
}

esp_mqtt_client_handle_t client = NULL;
void mqtt_start(mqtt_topic_t *mqtt_topic) {
  ESP_LOGI(MQTT_TAG, "MQTT START");
  esp_mqtt_client_config_t mqtt_cfg = {
      .broker.address.uri = CONFIG_ESP_MQTT_BROKER_URL,
      .broker.address.port = CONFIG_ESP_MQTT_BROKER_PORT,
      .credentials.username = CONFIG_ESP_MQTT_AUTH_USERNAME,
      .credentials.authentication.password = CONFIG_ESP_MQTT_AUTH_PASSWORD};

  client = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, event_handler,
                                 mqtt_topic);
  esp_mqtt_client_start(client);
}

void mqtt_pusblish(mqtt_topic_t *mqtt_topic) {
  if (MQTT_CONNECTED_FLAG) {
    esp_mqtt_client_publish(client, mqtt_topic->pub_topic, mqtt_topic->msg, 0,
                            1, 0);
  }
}
