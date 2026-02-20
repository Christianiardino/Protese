#include <Arduino.h>
#include <defines.h>

void leituraSensorCorrente_task(void* pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(TASK_FREQ_COLETA_CORRENTE);

    const uint8_t pinosSensorCorrente[5] = {SC1, SC2, SC3, SC4, SC5};

    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        int leiturasLocais[5];
        for(int i=0; i<5; i++) {
             leiturasLocais[i] = analogRead(pinosSensorCorrente[i]);
        }

        if (xSemaphoreTake(xSensorCorrenteMutex, (TickType_t)10) == pdTRUE) {
            for (int i = 0; i < 5; i++) {
                fArrSensorCorrente[i] = (fArrSensorCorrente[i] * 0.8) + (leiturasLocais[i] * 0.2);
                if(fArrDadosSensorFoto[i] > iPontoMedioSensorCorrente[i] + iCorrenteMaxima || 
                   fArrDadosSensorFoto[i] < iPontoMedioSensorCorrente[i] - iCorrenteMaxima ){
                    bArrSobrecorrenteDetectada[i] = true;
                    if(DEBUG_PRINT){
                        printf("[SYS] Aviso de corrente alta\n");
                    }
                }else{
                    bArrSobrecorrenteDetectada[i] = false;
                }
            }
            xSemaphoreGive(xSensorCorrenteMutex);
        }
    }
}