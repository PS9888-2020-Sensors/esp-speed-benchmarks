#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/gpio.h"

#define LEN_PACKET 250

#define PIN_ROLE_ID 15
uint8_t IS_MASTER = 1;

static const uint8_t MAC_MASTER[6] = {0x12, 0x22, 0x30, 0x44, 0x55, 0xA5};
static const uint8_t MAC_SLAVE[6] = {0x12, 0x22, 0x30, 0x44, 0x55, 0x5A};

static xQueueHandle data_recv_queue;
static const char *TAG = "espnow_benchmark";

typedef struct {
    int len;
} event_t;

static void espnow_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int len) {
    if (IS_MASTER) {
        if (memcmp(mac_addr, MAC_SLAVE, 6) != 0) {
            return;
        }

        static int i = 0;
        if (len == 4) {
                printf("[%i] Slave received %d bytes in 1s\n", i++, *(uint32_t *) data);
        }
    } else {
        if (memcmp(mac_addr, MAC_MASTER, 6) != 0) {
            return;
        }

        event_t evt;
        evt.len = len;

        xQueueSend(data_recv_queue, &evt, portMAX_DELAY);
    }
}

static void wifi_init(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static void espnow_task(void *pvParameter) {
    if (IS_MASTER) {
        ESP_LOGI(TAG, "Acting as master");
        uint8_t *data = malloc(LEN_PACKET);

        if (data == NULL) {
            ESP_LOGW(TAG, "Failed to malloc for packet data");
            return;
        }

        esp_fill_random(data, LEN_PACKET);
        while(1) {
            int64_t start = esp_timer_get_time();
            while ((esp_timer_get_time() - start) < 3000000) {
                esp_now_send(MAC_SLAVE, data, LEN_PACKET);
                vTaskDelay(100 / portTICK_RATE_MS);
            }

            vTaskDelay(5000 / portTICK_RATE_MS);
        }
    } else {
        ESP_LOGI(TAG, "Acting as slave");
        event_t evt;

        uint16_t i = 0;

        uint32_t recv_count = 0;
        int64_t last_trigger = esp_timer_get_time();

        while(1) {
            if (xQueueReceive(data_recv_queue, &evt, 0) == pdTRUE) {
                recv_count += evt.len;
            }

            // every second
            if ((esp_timer_get_time() - last_trigger) > 1000000) {
                if (recv_count > 0) {
                    printf("[%d] Received %d bytes in the last 1s\n", i++, recv_count);
                    esp_now_send(MAC_MASTER, (uint8_t *) &recv_count, sizeof(recv_count));

                    recv_count = 0;
                    last_trigger = esp_timer_get_time();
                }
            }
            
            vTaskDelay(10);
        }
    }
}

static void espnow_init(void) {
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_recv_cb(espnow_recv_cb));

    esp_now_peer_info_t peer;
    memset(&peer, 0, sizeof(esp_now_peer_info_t));
    peer.ifidx = ESP_IF_WIFI_STA;
    peer.encrypt = false;

    if (IS_MASTER) {
        memcpy(peer.peer_addr, MAC_SLAVE, 6);
    } else {
        memcpy(peer.peer_addr, MAC_MASTER, 6);
    }

    ESP_ERROR_CHECK(esp_now_add_peer(&peer));

    xTaskCreate(espnow_task, "espnow_task", 4096, NULL, 4, NULL);
}

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    gpio_set_direction(PIN_ROLE_ID, GPIO_MODE_INPUT);
    gpio_pullup_en(PIN_ROLE_ID);

    IS_MASTER = gpio_get_level(PIN_ROLE_ID);

    if (IS_MASTER) {
        esp_base_mac_addr_set(MAC_MASTER);
    } else {
        esp_base_mac_addr_set(MAC_SLAVE);

        data_recv_queue = xQueueCreate(75, sizeof(event_t));
        if (data_recv_queue == NULL) {
            ESP_LOGE(TAG, "Create queue fail");
            return;
        }
    }

    wifi_init();
    espnow_init();
}
