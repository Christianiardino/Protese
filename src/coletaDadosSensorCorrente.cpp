#include <Arduino.h>
#include <defines.h>

#include "driver/rmt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

void leituraSensorCorrente_task(void* pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(TASK_FREQ_COLETA_CORRENTE);

    const uint8_t pinosSensorCorrente[5] = {SC1, SC2, SC3, SC4, SC5};
    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        for (int i = 0; i < 5; i++) {
            dArrSensorCorrente[i] = (dArrSensorCorrente[i] * 0.8) + (analogRead(pinosSensorCorrente[i]) * 0.2);
            if (dArrSensorCorrente[i] > (2047 + iCorrenteMaxima) || dArrSensorCorrente[i] < (2047 - iCorrenteMaxima)) {
                bArrMotorLiberado[i] = false;
            } else {
                if (abs(dArrSensorCorrente[i] - 2047) < (iCorrenteMaxima * 0.8)) {
                    bArrMotorLiberado[i] = true;
                }
            }
        }
    }
}