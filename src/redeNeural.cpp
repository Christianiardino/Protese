#include <Arduino.h>
#include <defines.h>
#include <math.h>

void redeNeural_task(void* pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(TASK_FREQ_REDE_NEURAL);  // Certifique-se que essa define existe

    const double epsilon = 0.0000001;
    double x_norm[4];

    // Buffers para as camadas
    double layer1_a[20];
    double layer2_z[8];
    double probs_unnorm[8];  // Probabilidade com Prior (sem dividir pela soma)

    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        if (!bCalibDone) continue;

        double leituraAtual[4];  // Copia local
        if (xSemaphoreTake(xSensorFotoMutex, (TickType_t)5) == pdTRUE) {
            memcpy(leituraAtual, fArrDadosSensorFoto, sizeof(fArrDadosSensorFoto));
            xSemaphoreGive(xSensorFotoMutex);
        }

        // 1. Normalização (Z-Score)
        for (int i = 0; i < 4; i++) {
            x_norm[i] = (leituraAtual[i] - fMu[i]) / (fSigma[i] + epsilon);
        }

        // 2. Camada 1 (Dense 4->20 + Tanh)
        // layer1_a = tanh((x_norm * W1) + b1)
        for (int j = 0; j < 20; j++) {
            double z = 0.0;
            for (int i = 0; i < 4; i++) {
                z += x_norm[i] * fW1[i][j];
            }
            z += fB1[j];
            layer1_a[j] = tanh(z);
        }

        // 3. Camada 2 (Dense 20->8 Linear)
        // layer2_z = (layer1_a * W2) + b2
        double max_z = -99999.0;  // Para estabilidade do Softmax
        for (int k = 0; k < 8; k++) {
            double z = 0.0;
            for (int j = 0; j < 20; j++) {
                z += layer1_a[j] * fW2[j][k];
            }
            z += fB2[k];
            layer2_z[k] = z;

            if (z > max_z) max_z = z;  // Guarda o maior valor
        }

        // 4. Softmax + Priors (Combinados)
        // UnnormProbs = exp(z - max) * Prior
        for (int k = 0; k < 8; k++) {
            double exps = exp(layer2_z[k] - max_z);
            probs_unnorm[k] = exps * fPrior[k];
        }

        // 5. Matriz de Custo e Decisão (Argmin Risk)
        // Não precisamos dividir pela soma total das probabilidades para achar o menor risco
        double minRisk = 9999999.0;
        uint8_t bestClass = 0;

        for (int c = 0; c < 8; c++) {
            double currentRisk = 0.0;

            // Soma ponderada: Probabilidade(k) * Custo(k -> c)
            for (int k = 0; k < 8; k++) {
                currentRisk += probs_unnorm[k] * (double)fCost[k][c];
            }

            if (currentRisk < minRisk) {
                minRisk = currentRisk;
                bestClass = c;
            }
        }

        if (xSemaphoreTake(xAtuaMotorMutex, (TickType_t)5) == pdTRUE) {
            switch (bestClass) {
                case 0:
                    bArrDedoContraido[0] = false;
                    bArrDedoContraido[1] = false;
                    bArrDedoContraido[2] = false;
                    bArrDedoContraido[3] = false;
                    bArrDedoContraido[4] = false;
                    break;
                case 1:
                    bArrDedoContraido[0] = true;
                    bArrDedoContraido[1] = false;
                    bArrDedoContraido[2] = false;
                    bArrDedoContraido[3] = false;
                    bArrDedoContraido[4] = false;
                    break;
                case 2:
                    bArrDedoContraido[0] = false;
                    bArrDedoContraido[1] = true;
                    bArrDedoContraido[2] = false;
                    bArrDedoContraido[3] = false;
                    bArrDedoContraido[4] = false;
                    break;
                case 3:
                    bArrDedoContraido[0] = true;
                    bArrDedoContraido[1] = true;
                    bArrDedoContraido[2] = false;
                    bArrDedoContraido[3] = false;
                    bArrDedoContraido[4] = false;
                    break;
                case 4:
                    bArrDedoContraido[0] = true;
                    bArrDedoContraido[1] = true;
                    bArrDedoContraido[2] = true;
                    bArrDedoContraido[3] = true;
                    bArrDedoContraido[4] = true;
                    break;
                case 5:
                    bArrDedoContraido[0] = false;
                    bArrDedoContraido[1] = true;
                    bArrDedoContraido[2] = true;
                    bArrDedoContraido[3] = false;
                    bArrDedoContraido[4] = false;
                    break;
                case 6:
                    bArrDedoContraido[0] = false;
                    bArrDedoContraido[1] = true;
                    bArrDedoContraido[2] = true;
                    bArrDedoContraido[3] = true;
                    bArrDedoContraido[4] = false;
                    break;
                case 7:
                    bArrDedoContraido[0] = false;
                    bArrDedoContraido[1] = true;
                    bArrDedoContraido[2] = true;
                    bArrDedoContraido[3] = true;
                    bArrDedoContraido[4] = true;
                    break;
            }

            xSemaphoreGive(xAtuaMotorMutex);
        }
    }
}