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

    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        if (xSemaphoreTake(xAtuaMotorMutex, (TickType_t)5) == true) {
            if (bModoTreino) {
                for (uint8_t i = 0; i < 5; i++) {
                    if (bArrMotorLiberado) {
                        if (bArrDedoContraido) {
                            iPosServo[i] = 0;
                        } else {
                            iPosServo[i] = 180;
                        }
                        servoDedos[i].write(iPosServo[i]);
                        Serial.println("Escreveu no servo!");
                    }
                }
            } else {
                Serial.println("Escreveu no servo!");
                servoDedos[0].write(30);
                servoDedos[1].write(60);
                servoDedos[2].write(90);
                servoDedos[3].write(120);
                servoDedos[4].write(150);
            }
            xSemaphoreGive(xAtuaMotorMutex);
        }

        if (DEBUG_STACK_SIZE) {
            UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
            printf("[DEBUG] Stack atuaMotor: %d\n", (int)uxHighWaterMark);
        }
    }
}