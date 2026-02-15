#include <Arduino.h>
#include <defines.h>

#include "driver/rmt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#define PWM_FREQ 5000
#define PWM_RES 8

const uint8_t MOT_CHANNELS[6] = {0, 1, 2, 3, 4, 5};

void atuaMotor_task(void* pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(TAKS_FREQ_ATUA_MOTOR);

    for (int i = 0; i < 6; i++) {
        ledcSetup(MOT_CHANNELS[i], PWM_FREQ, PWM_RES);
        if (uiArrPwmLevels[i] == 0) uiArrPwmLevels[i] = 255;
    }

    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        if (xSemaphoreTake(xAtuaMotorMutex, (TickType_t)5) == true) {
            if (bModoTreino) {
                for (uint8_t i = 0; i < 5; i++) {
                    if (!bArrMotorEstadoTravado && bArrMotorLiberado && !bArrSobrecorrenteDetectada) {
                        if (bArrDedoContraido) {
                            iPosServo[i] = 0;
                        } else {
                            iPosServo[i] = 180;
                        }
                    }
                }
            } else {
            }
            xSemaphoreGive(xAtuaMotorMutex);
        }

        if (DEBUG_STACK_SIZE) {
            UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
            printf("[DEBUG] Stack atuaMotor: %d\n", (int)uxHighWaterMark);
        }
    }
}