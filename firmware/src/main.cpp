#include <Arduino.h>

QueueHandle_t sampleQueue;

struct Reading {
    uint32_t seq;
    float value;
};

void producerTask(void *pvParameters) {
    TickType_t lastWake = xTaskGetTickCount();
    uint32_t seq = 0;
    while (true) {
        Reading r = { seq++, 0.1f * seq };

        if (xQueueSend(sampleQueue, &r, 0) != pdTRUE) {
        Serial.println("!! queue full, sample dropped");
        }

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(100));
        Serial.print("  stack free: ");
    Serial.println(uxTaskGetStackHighWaterMark(NULL));
    }
}

void consumerTask(void *pvParameters) {
    Reading r;
    while (true) {
        if (xQueueReceive(sampleQueue, &r, portMAX_DELAY) == pdTRUE) {
        Serial.print("got seq=");
        Serial.print(r.seq);
        Serial.print("  queued=");
        Serial.println(uxQueueMessagesWaiting(sampleQueue));
        vTaskDelay(pdMS_TO_TICKS(300));    // simulate a slow SD write
        }
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("FreeRTOS queue demo");

    sampleQueue = xQueueCreate(10, sizeof(Reading));

    xTaskCreatePinnedToCore(producerTask, "producer", 2048, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(consumerTask, "consumer", 2048, NULL, 3, NULL, 1);
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}