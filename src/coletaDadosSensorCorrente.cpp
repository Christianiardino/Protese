#include <Arduino.h>
#include <defines.h>

void leituraSensorCorrente_task(void* pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(TASK_FREQ_COLETA_CORRENTE);  // Verifique se é 10ms ou 100ms no defines

    const uint8_t pinosSensorCorrente[5] = {SC1, SC2, SC3, SC4, SC5};

    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        for (int i = 0; i < 5; i++) {
            // TODO IMPLEMETAR ESTA PARTE
            vTaskDelayUntil(&xLastWakeTime, xFrequency);
        }
    }
}