#include <Arduino.h>
#include <defines.h>

void leituraSensorCorrente_task(void* pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(TASK_FREQ_COLETA_CORRENTE);  // Verifique se é 10ms ou 100ms no defines

    const uint8_t pinosSensorCorrente[5] = {SC1, SC2, SC3, SC4, SC5};

    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        for (int i = 0; i < 5; i++) {
            dArrSensorCorrente[i] = (dArrSensorCorrente[i] * 0.8) + (analogRead(pinosSensorCorrente[i]) * 0.2);
            // dArrSensorCorrente[i] = (dArrSensorCorrente[i] * 0.95) + (analogRead(pinosSensorCorrente[i]) * 0.05);

            // Verificação de Limites
            bool correnteAltaAgora = false;
            if (dArrSensorCorrente[i] > (2047 + iCorrenteMaxima) || dArrSensorCorrente[i] < (2047 - iCorrenteMaxima)) {
                correnteAltaAgora = true;
            }

            // Contador de Ciclos (Debounce + Filtro de Pico de Partida)
            if (correnteAltaAgora) {
                if (uiArrContadorCiclosAlta[i] < 255) {
                    uiArrContadorCiclosAlta[i]++;
                }
            } else {
                uiArrContadorCiclosAlta[i] = 0;
            }

            // Define flag de Sobrecorrente Baseada no Tempo
            if (uiArrContadorCiclosAlta[i] >= uiCliclosCorrenteAlta) {
                bArrSobrecorrenteDetectada[i] = true;
            } else {
                bArrSobrecorrenteDetectada[i] = false;
            }
        }
    }
}