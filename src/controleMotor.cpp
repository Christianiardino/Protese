#include <Arduino.h>
#include <defines.h>

#include "driver/rmt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

/**
 * @brief Task responsavel por controlar os motores com proteção de corrente e otimização de PWM
 */
void atuaMotor_task(void* pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(TAKS_FREQ_ATUA_MOTOR);

    // SEGURANÇA NA INICIALIZAÇÃO:
    // Garante que os níveis alvo não sejam 0, senão o motor nunca gira no modo automático.
    // O ideal é definir isso no defines.cpp, mas aqui serve como fallback.
    for (int i = 0; i < 6; i++) {
        if (uiArrPwmLevels[i] == 0) uiArrPwmLevels[i] = 255;
    }

    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        if (xSemaphoreTake(xAtuaMotorMutex, (TickType_t)5) == true) {
            if (!bModoTreino) {
                // Loop para os 5 dedos principais
                for (uint8_t localN = 0; localN < 5; localN++) {
                    uint8_t pinoAtivo;
                    uint8_t pinoInativo;
                    bool direcaoAtual = bArrDedoContraido[localN];  // true=fechar, false=abrir

                    // DETECÇÃO DE MUDANÇA DE DIREÇÃO
                    if (direcaoAtual != bArrUltimaDirecao[localN]) {
                        bArrMotorEstadoTravado[localN] = false;      // Destrava
                        uiArrContadorCiclosAlta[localN] = 0;         // Reseta contadores de tempo
                        bArrSobrecorrenteDetectada[localN] = false;  // Limpa flag do sensor
                        bArrUltimaDirecao[localN] = direcaoAtual;    // Atualiza memória da direção
                    }

                    // CONFIGURAÇÃO DE PINOS (Swap de direção)
                    if (direcaoAtual) {
                        pinoAtivo = MOTOR_IN_1_PIN[localN];
                        pinoInativo = MOTOR_IN_2_PIN[localN];
                    } else {
                        pinoAtivo = MOTOR_IN_2_PIN[localN];
                        pinoInativo = MOTOR_IN_1_PIN[localN];
                    }

                    // Libera imediatamente o canal PWM do lado inativo - Só 8 canais pwm na esp32
                    digitalWrite(pinoInativo, LOW);

                    // MÁQUINA DE ESTADOS DO MOVIMENTO
                    if (bArrMotorEstadoTravado[localN]) {
                        // --- ESTADO: TRAVADO (Holding) ---
                        if (uiArrPwmCorrenteAtingida == 0) {
                            digitalWrite(pinoAtivo, LOW);
                        } else {
                            analogWrite(pinoAtivo, uiArrPwmCorrenteAtingida);
                        }

                        uiArrPwmRampa[localN] = uiArrPwmCorrenteAtingida;

                    } else {
                        // --- ESTADO: MOVENDO (Rampa) ---

                        // Verifica se o sensor de corrente detectou obstáculo
                        if (bArrSobrecorrenteDetectada[localN]) {
                            bArrMotorEstadoTravado[localN] = true;
                        } else {
                            // Caminho livre. Acelera rampa conforme configuração.
                            if (bArrMotorLiberado[localN]) {  // Flag geral de habilitação
                                // Incremento Suave (Rampa)
                                if (uiArrPwmRampa[localN] < uiArrPwmLevels[localN]) {
                                    uiArrPwmRampa[localN] += STEP_PWM_RAMPA;
                                } else {
                                    uiArrPwmRampa[localN] = uiArrPwmLevels[localN];
                                }
                                // Aplicação do PWM
                                if (uiArrPwmRampa[localN] == 0) {
                                    digitalWrite(pinoAtivo, LOW);
                                } else {
                                    analogWrite(pinoAtivo, uiArrPwmRampa[localN]);
                                }
                            } else {
                                digitalWrite(pinoAtivo, LOW);
                                uiArrPwmRampa[localN] = 0;
                            }
                        }
                    }
                }  // Fim do loop dedos

                // M6 (Giro) - Mantido em LOW para economizar canais enquanto você não implementa a lógica dele
                digitalWrite(MOTOR_IN_1_PIN[5], LOW);
                // digitalWrite(MOTOR_IN_2_PIN[5], LOW); // Se existir pino 2
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