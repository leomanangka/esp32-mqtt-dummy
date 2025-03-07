#ifndef MQTT_CLIENT_WRAPPER_H
#define MQTT_CLIENT_WRAPPER_H

typedef struct {
  const char *pub_topic;
  const char *sub_topic;
  char *msg;
  int interval;
} mqtt_topic_t;

void mqtt_start(mqtt_topic_t *mqtt_topic);
void mqtt_pusblish(mqtt_topic_t *mqtt_topic);

#endif
