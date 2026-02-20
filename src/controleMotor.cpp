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
                    if (bArrMotorLiberado[i]) {
                        if (uiVetorDedosTreino[i] == 0) {
                            uiTargetPosServo[i] = 0;
                        } else {
                            uiTargetPosServo[i] = 180;
                        }
                    }
                }
            } else {
                for(int i = 0; i < 5; i++){
                    if(bArrDedoContraido[i]){
                        uiTargetPosServo[i] = 180;
                    }else{
                        uiTargetPosServo[i] = 0;
                    }
                }
            }

            // Loop para passar valor uiTargetPosServo para posServo com rampa
            for (uint8_t i = 0; i < 5; i++) {

                // Fail safe de posição
                if(uiTargetPosServo[i] > 180){
                    uiTargetPosServo[i] = 0;
                    printf("[ERR] POSIÇÃO MAIOR QUE 180");
                }

                // Inicia rampa para controle do servo
                if(uiTargetPosServo[i] > uiPosServo[i]){
                    uiPosServo[i] = uiPosServo[i] + uiStepAnguloServo;
                    if(uiPosServo[i] > 180){
                        uiPosServo[i] = 180;
                    }
                }else if(uiTargetPosServo[i] < uiPosServo[i]){
                    uiPosServo[i] = uiPosServo[i] - uiStepAnguloServo;
                    if(uiPosServo[i] > 180){
                        uiPosServo[i] = 0;
                    }
                }

                servoDedos[i].write(uiPosServo[i]);
            }
            xSemaphoreGive(xAtuaMotorMutex);
        }

        if (DEBUG_STACK_SIZE) {
            UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
            printf("[DEBUG] Stack atuaMotor: %d\n", (int)uxHighWaterMark);
        }
    }
}