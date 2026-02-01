#include "modoTreino.h"

#include <Arduino.h>
#include <defines.h>
#include <funcaoNeoPixel.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TEMPO_ESTADO_MS 3000

enum EstadoTreino {
    ST_OPERACAO,
    ST_FECHADO,
    ST_POLEGAR,
    ST_INDICADOR,
    ST_PINCA,
    ST_ABRE_PALMA,
    ST_DOIS,
    ST_TRES,
    ST_QUATRO
};

enum EstadoCalibracao {
    CALIB_IDLE,
    CALIB_COLETANDO,
    CALIB_CALCULO
};

void atualizaMaquinaEstadosCalibracao();
void setDedos(uint8_t v1, uint8_t v2, uint8_t v3, uint8_t v4, uint8_t v5);

static EstadoTreino estadoMotorAtual = ST_OPERACAO;
static EstadoCalibracao estadoCalibAtual = CALIB_IDLE;
static unsigned long ultimoTempoTroca = 0;

void modoTreinoAtivado_task(void* pvParameters) {
    const TickType_t xFrequency = pdMS_TO_TICKS(TASK_FREQ_MODO_TREINO);
    TickType_t xLastWakeTime = xTaskGetTickCount();

    uiTreinoVal = 0;

    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        atualizaMaquinaEstadosCalibracao();

        if (!bModoTreino && estadoMotorAtual != ST_OPERACAO) {
            estadoMotorAtual = ST_OPERACAO;
            setDedos(0, 0, 0, 0, 0);
            setInternalLedColor(255, 0, 0);
            printf("[TREINO] Interrompido externamente. Resetando motores.\n");
        }

        if (bModoTreino) {
            if (millis() - ultimoTempoTroca >= TEMPO_ESTADO_MS) {
                switch (estadoMotorAtual) {
                    case ST_OPERACAO:
                        setDedos(0, 0, 0, 0, 0);
                        setInternalLedColor(255, 0, 0);
                        printf("[TREINO-MOTOR] Estado: OPERACAO (Inicio)\n");

                        estadoMotorAtual = ST_FECHADO;
                        ultimoTempoTroca = millis();
                        break;

                    case ST_FECHADO:
                        setDedos(0, 0, 0, 0, 0);
                        uiTreinoE = 0;
                        setInternalLedColor(0, 0, 0);
                        printf("[TREINO-MOTOR] Estado: FECHADO\n");

                        if (uiTreinoVal == 0)
                            estadoMotorAtual = ST_POLEGAR;
                        else if (uiTreinoVal == 1)
                            estadoMotorAtual = ST_INDICADOR;
                        else if (uiTreinoVal == 2)
                            estadoMotorAtual = ST_PINCA;
                        else if (uiTreinoVal == 3)
                            estadoMotorAtual = ST_ABRE_PALMA;
                        else if (uiTreinoVal >= 4)
                            estadoMotorAtual = ST_ABRE_PALMA;

                        ultimoTempoTroca = millis();
                        break;

                    case ST_POLEGAR:
                        setDedos(1, 0, 0, 0, 0);
                        setInternalLedColor(0, 0, 255);
                        printf("[TREINO-MOTOR] Estado: POLEGAR\n");

                        uiTreinoVal = 1;
                        uiTreinoE = 1;
                        estadoMotorAtual = ST_FECHADO;
                        ultimoTempoTroca = millis();
                        break;

                    case ST_INDICADOR:
                        setDedos(0, 1, 0, 0, 0);
                        setInternalLedColor(0, 50, 0);
                        printf("[TREINO-MOTOR] Estado: INDICADOR\n");

                        uiTreinoVal = 2;
                        uiTreinoE = 2;
                        estadoMotorAtual = ST_FECHADO;
                        ultimoTempoTroca = millis();
                        break;

                    case ST_PINCA:
                        setDedos(1, 1, 0, 0, 0);
                        setInternalLedColor(0, 100, 0);
                        printf("[TREINO-MOTOR] Estado: PINCA\n");

                        uiTreinoVal = 3;
                        uiTreinoE = 3;
                        estadoMotorAtual = ST_FECHADO;
                        ultimoTempoTroca = millis();
                        break;

                    case ST_ABRE_PALMA:
                        setDedos(1, 1, 1, 1, 1);
                        setInternalLedColor(50, 50, 50);
                        printf("[TREINO-MOTOR] Estado: ABRE_PALMA (Val: %d)\n", uiTreinoVal);

                        uiTreinoE = 4;

                        if (uiTreinoVal == 0) {
                            estadoMotorAtual = ST_OPERACAO;
                            bModoTreino = false;
                            bCalibDone = true;
                            printf("[TREINO-MOTOR] Sequencia Finalizada. Calculando estatisticas...\n");
                        } else {
                            estadoMotorAtual = ST_DOIS;
                        }
                        ultimoTempoTroca = millis();
                        break;

                    case ST_DOIS:
                        setDedos(0, 1, 1, 0, 0);
                        setInternalLedColor(0, 150, 0);
                        printf("[TREINO-MOTOR] Estado: DOIS\n");

                        uiTreinoVal = 5;
                        uiTreinoE = 5;
                        estadoMotorAtual = ST_TRES;
                        ultimoTempoTroca = millis();
                        break;

                    case ST_TRES:
                        setDedos(0, 1, 1, 1, 0);
                        setInternalLedColor(0, 200, 0);
                        printf("[TREINO-MOTOR] Estado: TRES\n");

                        uiTreinoVal = 6;
                        uiTreinoE = 6;
                        estadoMotorAtual = ST_QUATRO;
                        ultimoTempoTroca = millis();
                        break;

                    case ST_QUATRO:
                        setDedos(0, 1, 1, 1, 1);
                        setInternalLedColor(0, 255, 0);
                        printf("[TREINO-MOTOR] Estado: QUATRO\n");

                        uiTreinoVal = 0;
                        uiTreinoE = 7;
                        estadoMotorAtual = ST_ABRE_PALMA;
                        ultimoTempoTroca = millis();
                        break;
                }
            }
        }
    }
}

void atualizaMaquinaEstadosCalibracao() {
    switch (estadoCalibAtual) {
        case CALIB_IDLE:
            if (!bModoTreino) {
                dTreinoCount = 0.0;
                for (int i = 0; i < 4; i++) {
                    dTreinoSum[i] = 0.0;
                    dTreinoSumSq[i] = 0.0;
                }
            }

            if (bModoTreino) {
                printf("[CALIB] Iniciando Coleta (IDLE -> COLETANDO)\n");
                estadoCalibAtual = CALIB_COLETANDO;
            }
            break;

        case CALIB_COLETANDO:
            if (!bModoTreino) {
                printf("[CALIB] Fim da Coleta (COLETANDO -> CALCULO)\n");
                estadoCalibAtual = CALIB_CALCULO;
            } else {
                for (int i = 0; i < 4; i++) {
                    double valorSensor = dArrDadosSensorFoto[i];
                    dTreinoSum[i] += valorSensor;
                    dTreinoSumSq[i] += (valorSensor * valorSensor);
                }
                dTreinoCount += 1.0;
            }
            break;

        case CALIB_CALCULO:
            printf("[CALIB] Realizando calculos estatisticos...\n");
            setInternalLedColor(0, 0, 0);

            if (dTreinoCount > 0) {
                for (int i = 0; i < 4; i++) {
                    double mu_novo = dTreinoSum[i] / dTreinoCount;
                    dMu[i] = mu_novo;

                    double var_novo = (dTreinoSumSq[i] / dTreinoCount) - (mu_novo * mu_novo);

                    if (var_novo < 0) var_novo = 0;

                    dSigma[i] = sqrt(var_novo);

                    printf("  Sensor[%d]: Mu=%.2f, Sigma=%.2f\n", i, dMu[i], dSigma[i]);
                }
            } else {
                printf("[CALIB] Erro: Divisao por zero (Count = 0)\n");
            }

            estadoCalibAtual = CALIB_IDLE;
            break;
    }
}

void setDedos(uint8_t v1, uint8_t v2, uint8_t v3, uint8_t v4, uint8_t v5) {
    uiVetorDedosTreino[0] = v1;
    uiVetorDedosTreino[1] = v2;
    uiVetorDedosTreino[2] = v3;
    uiVetorDedosTreino[3] = v4;
    uiVetorDedosTreino[4] = v5;
}