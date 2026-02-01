#include <Arduino.h>
#include <defines.h>

#include "driver/rmt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

/**
 * @brief Task responsavel por jogar os PWM em cada pin
 */
void atuaMotor_task(void* pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(TAKS_FREQ_ATUA_MOTOR);

    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        if (xSemaphoreTake(xAtuaMotorMutex, (TickType_t)5) == true) {
            if (bModoTreino) {
                for (uint8_t i = 0; i < 5; i++) {
                    ledcDetachPin(MOTOR_IN_1_PIN[i]);
                    pinMode(MOTOR_IN_1_PIN[i], OUTPUT);
                    digitalWrite(MOTOR_IN_1_PIN[i], LOW);

                    if (uiVetorDedosTreino[i] == 1) {
                        analogWrite(MOTOR_IN_2_PIN[i], 255);
                    } else {
                        ledcDetachPin(MOTOR_IN_2_PIN[i]);
                        pinMode(MOTOR_IN_2_PIN[i], OUTPUT);
                        digitalWrite(MOTOR_IN_2_PIN[i], LOW);
                    }
                }
                ledcDetachPin(MOTOR_IN_1_PIN[5]);
                pinMode(MOTOR_IN_1_PIN[5], OUTPUT);
                digitalWrite(MOTOR_IN_1_PIN[5], LOW);
            } else {  // ! bModoTreino
                for (uint8_t localN = 0; localN < 5; localN++) {
                    uint8_t pinoAtivo;
                    uint8_t pinoInativo;

                    if (bArrDedoContraido[localN]) {
                        pinoAtivo = MOTOR_IN_1_PIN[localN];
                        pinoInativo = MOTOR_IN_2_PIN[localN];
                    } else {
                        pinoAtivo = MOTOR_IN_2_PIN[localN];
                        pinoInativo = MOTOR_IN_1_PIN[localN];
                    }

                    if (bArrMotorLiberado[localN]) {
                        if (uiArrPwmRampa[localN] >= uiArrPwmLevels[localN]) {
                            uiArrPwmRampa[localN] = uiArrPwmLevels[localN];
                        } else {
                            uiArrPwmRampa[localN] =
                                uiArrPwmRampa[localN] + STEP_PWM_RAMPA;
                        }

                        ledcDetachPin(pinoInativo);
                        pinMode(pinoInativo, OUTPUT);
                        digitalWrite(pinoInativo, LOW);

                        if (uiArrPwmRampa[localN] == 0) {
                            ledcDetachPin(pinoAtivo);
                            pinMode(pinoAtivo, OUTPUT);
                            digitalWrite(pinoAtivo, LOW);
                        } else {
                            analogWrite(pinoAtivo, uiArrPwmRampa[localN]);
                        }
                    } else {
                        ledcDetachPin(MOTOR_IN_1_PIN[localN]);
                        pinMode(MOTOR_IN_1_PIN[localN], OUTPUT);
                        digitalWrite(MOTOR_IN_1_PIN[localN], LOW);

                        ledcDetachPin(MOTOR_IN_2_PIN[localN]);
                        pinMode(MOTOR_IN_2_PIN[localN], OUTPUT);
                        digitalWrite(MOTOR_IN_2_PIN[localN], LOW);
                        uiArrPwmRampa[localN] = 0;
                    }
                }

                pinMode(MOTOR_IN_1_PIN[5], OUTPUT);
                analogWrite(MOTOR_IN_1_PIN[5], uiArrPwmRampa[5]);
            }
            xSemaphoreGive(xAtuaMotorMutex);
        }

        if (DEBUG_STACK_SIZE) {
            UBaseType_t uxHighWaterMark;
            uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
            printf("[DEBUG] Stack livre mínima atua motor: %d bytes\n", (int)uxHighWaterMark);
        }
    }
}