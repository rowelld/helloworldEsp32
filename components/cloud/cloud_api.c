#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "cloud_api.h"
#include "cloud_tcp.h"
#include "cloud_mqtt.h"
#include "cloud_config_default.h"
#include "cloud_nvs.h"
#include "esp_log.h"
#include "esp_err.h"

static cloud_app_cb app_layer_cb;
static cloud_tcp_handle_t tcp_handle;
static cloud_config_t cloud_config = {0};

static char mqtt_main_topic[32] = CLOUD_CFG_MQTT_TOPIC_MAIN;
static char mqtt_rc_topic_rx[256]; /*!< Receiver topic */
static char mqtt_rc_topic_tx[256];

void cloud_api_init(cloud_app_cb app_cb)
{
    app_layer_cb = app_layer_cb;
    esp_err_t res = cloud_nvs_get(&cloud_config);    

    if (res != ESP_OK) {
        cloud_config.conntype = CLOUD_CFG_CONN_TYPE;

        strcpy(&cloud_config.host, CLOUD_CFG_MQTT_HOST);
        
        cloud_config.port = CLOUD_CFG_MQTT_PORT;
    }

    if (cloud_config.conntype == CLOUD_TYPE_MQTT)
    {
        cloud_mqtt_init(app_cb);
    }
    else 
    {
        tcp_handle = cloud_tcp_init();
    }
}

void cloud_api_deinit(void)
{
    if (cloud_config.conntype == CLOUD_TYPE_MQTT)
    {
        cloud_mqtt_deinit();
    }
    else if (cloud_config.conntype == CLOUD_TYPE_TCP)
    {
        cloud_tcp_deinit(tcp_handle);
    }
}

cloud_ret_t cloud_api_connect(void) {
    return cloud_api_connect_host(cloud_config.host, cloud_config.port);
}

cloud_ret_t cloud_api_connect_host(const char* dst, uint16_t port)
{
    cloud_ret_t err = CLOUD_RET_UNKNOWN_PROTOCOL;
    assert(dst != NULL);

    if ((cloud_config.conntype == CLOUD_TYPE_MQTT) || (cloud_config.conntype == CLOUD_TYPE_MQTTS))
    {
        if (cloud_mqtt_connect(dst, port))
        {
            err = CLOUD_RET_OK;
        }
    }
    else if (cloud_config.conntype == CLOUD_TYPE_TCP)
    {
        if (!(cloud_tcp_connect(tcp_handle, dst, port) < 0))
        {
            err = CLOUD_RET_OK;
        }
    }

    return err;
}

cloud_ret_t cloud_api_disconnect(void)
{
    cloud_ret_t err = CLOUD_RET_ERR;

    if ((cloud_config.conntype == CLOUD_TYPE_MQTT) || (cloud_config.conntype == CLOUD_TYPE_MQTTS))
    {
        if (cloud_mqtt_disconnect())
        {
            err = CLOUD_RET_OK;
        }
    }
    else if (cloud_config.conntype == CLOUD_TYPE_TCP)
    {
        cloud_tcp_disconnect(tcp_handle);
        err = CLOUD_RET_OK;
    }

    return err;
}

cloud_ret_t cloud_api_send(const uint8_t *data, size_t len)
{
    cloud_ret_t err = CLOUD_RET_ERR;
    assert(data != NULL);

    if (strlen(mqtt_main_topic) && ((cloud_config.conntype == CLOUD_TYPE_MQTT) || (cloud_config.conntype == CLOUD_TYPE_MQTTS)))
    {
        if (cloud_mqtt_publish(mqtt_main_topic, data, len, 2))
        {
            err = CLOUD_RET_OK;
        }
    }
    else if (cloud_config.conntype == CLOUD_TYPE_TCP)
    {
        cloud_tcp_send(tcp_handle, data, len);
    }

    return err;
}

cloud_ret_t cloud_api_set_mqtt_topic(const char *topic, const char *id)
{
    assert(topic != NULL);

    strcpy(mqtt_main_topic, topic);
    return CLOUD_RET_OK;
}

cloud_ret_t cloud_api_set_mqtt_id(const char *id)
{
    assert(id != NULL);

    char mqttId[64] = {0};
    // sprintf(mqttId, "esp_client_%s", id);
    sprintf(mqttId, "icm_%s", id);

    sprintf(mqtt_rc_topic_rx, CLOUD_CFG_MQTT_BASE_TOPIC_RC_RX, id);
    sprintf(mqtt_rc_topic_tx, CLOUD_CFG_MQTT_BASE_TOPIC_RC_TX, id);

    // sprintf(mqtt_main_topic, "xeleqt/v1/%s", id);
    strcpy(mqtt_main_topic, "xeleqt/v1/user");
    
    return CLOUD_RET_OK;
}

const char *cloud_api_get_mqtt_topic(void)
{
    return mqtt_main_topic;
}

const char *cloud_api_get_mqtt_rx_topic(void)
{
    return mqtt_rc_topic_rx;
}

const char *cloud_api_get_mqtt_tx_topic(void)
{
    return mqtt_rc_topic_tx;
}

cloud_ret_t cloud_api_send_rc_response(const uint8_t *data, size_t len)
{
    cloud_ret_t err = CLOUD_RET_ERR;

    if ((cloud_config.conntype == CLOUD_TYPE_MQTT) || (cloud_config.conntype == CLOUD_TYPE_MQTTS))
    {
        if (cloud_mqtt_publish(mqtt_rc_topic_tx, data, len, 2))
        {
            err = CLOUD_RET_OK;
        }
    }
    else if (cloud_config.conntype == CLOUD_TYPE_TCP)
    {
        cloud_tcp_send(tcp_handle, data, len);
    }

    return err;
}

void cloud_api_get_config(cloud_config_t* config) {
    *config = cloud_config;
}

void cloud_api_set_config(const cloud_config_t* config) {
    if(cloud_config.conntype == CLOUD_TYPE_MQTT) {
        cloud_mqtt_disconnect();
    }
    cloud_config = *config;
    cloud_nvs_set_host(cloud_config.host);
    cloud_nvs_set_port(cloud_config.port);
    cloud_nvs_set_conntype(cloud_config.conntype);
}
