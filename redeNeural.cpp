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

        // Se não tiver calibração, pode pular ou rodar com zeros (opcional)
        // if(!bCalibDone) continue;

        // 1. Normalização (Z-Score)
        for (int i = 0; i < 4; i++) {
            x_norm[i] = (dArrDadosSensorFoto[i] - dMu[i]) / (dSigma[i] + epsilon);
        }

        // 2. Camada 1 (Dense 4->20 + Tanh)
        // layer1_a = tanh((x_norm * W1) + b1)
        for (int j = 0; j < 20; j++) {
            double z = 0.0;
            for (int i = 0; i < 4; i++) {
                z += x_norm[i] * dW1[i][j];
            }
            z += dB1[j];
            layer1_a[j] = tanh(z);
        }

        // 3. Camada 2 (Dense 20->8 Linear)
        // layer2_z = (layer1_a * W2) + b2
        double max_z = -99999.0;  // Para estabilidade do Softmax
        for (int k = 0; k < 8; k++) {
            double z = 0.0;
            for (int j = 0; j < 20; j++) {
                z += layer1_a[j] * dW2[j][k];
            }
            z += dB2[k];
            layer2_z[k] = z;

            if (z > max_z) max_z = z;  // Guarda o maior valor
        }

        // 4. Softmax + Priors (Combinados)
        // UnnormProbs = exp(z - max) * Prior
        for (int k = 0; k < 8; k++) {
            double exps = exp(layer2_z[k] - max_z);
            probs_unnorm[k] = exps * dPrior[k];
        }

        // 5. Matriz de Custo e Decisão (Argmin Risk)
        // Não precisamos dividir pela soma total das probabilidades para achar o menor risco
        double minRisk = 9999999.0;
        uint8_t bestClass = 0;

        for (int c = 0; c < 8; c++) {
            double currentRisk = 0.0;

            // Soma ponderada: Probabilidade(k) * Custo(k -> c)
            for (int k = 0; k < 8; k++) {
                currentRisk += probs_unnorm[k] * (double)dCost[k][c];
            }

            if (currentRisk < minRisk) {
                minRisk = currentRisk;
                bestClass = c;
            }
        }
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
    }
}