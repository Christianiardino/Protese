#include <Arduino.h>
#include <defines.h>

void coletaDadosSensorFotoeletrico_task(void* pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(TASK_FREQ_COLETA_FOTO);
    const int pinos[] = {SL1, SL2, SL3, SL4};

    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        if (bModoTreino) {
            iContadorChamdaSensorFoto += 1;
        }

        for (int i = 0; i < 4; i++) {
            double somaTemporaria = 0;

            for (int j = 7; j > 0; j--) {
                dArrDadosSensorFotoBruto[i][j] = dArrDadosSensorFotoBruto[i][j - 1];
            }

            dArrDadosSensorFotoBruto[i][0] = (double)analogRead(pinos[i]);

            for (int j = 0; j < 7; j++) {
                somaTemporaria += dArrDadosSensorFotoBruto[i][j];
            }

            dArrDadosSensorFoto[i] = somaTemporaria * 0.125;
        }

        if (DEBUG_STACK_SIZE) {
            UBaseType_t uxHighWaterMark;
            uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
            printf("[DEBUG] Stack livre mínima coleta dados foto: %d bytes\n", (int)uxHighWaterMark);
        }
    }
}