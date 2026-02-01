#ifndef DEFINES_H
#define DEFINES_H

#include <Arduino.h>

#include "driver/rmt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

// Definições de DEBUG
#define DEBUG_STACK_SIZE false
#define DEBUG_PRINT false

#define INTERNAL_RGB 48

//______________________________________________ Atua Motores______________________________________________
// Definições de pinos dos motores
#define IN1M1 11
#define IN2M1 12
#define IN1M2 16
#define IN2M2 15
#define IN1M3 41
#define IN2M3 40
#define IN1M4 02
#define IN2M4 42
#define IN1M5 38
#define IN2M5 39
#define IN1M6 20

// Defines Atua motor
#define STEP_PWM_RAMPA 1

//______________________________________________ Update Neopixel______________________________________________
// Definições de Hardware e Protocolo NEOPIXEL
#define NEOPIXEL_PIN 19
#define N_PIXELS 7
#define RMT_TX_CHANNEL RMT_CHANNEL_0

// Timings baseados no APB Clock (80MHz), com divisor 2 (40MHz) -> 1 tick = 25ns
#define T0H_TICKS 14  // 350ns
#define T0L_TICKS 32  // 800ns
#define T1H_TICKS 28  // 700ns
#define T1L_TICKS 24  // 600ns
#define DIVISOR_CLOCK_PINO_LED 2

//______________________________________________ Leitura sensor corrente ______________________________________________
// Definições de Hardware pinos sensor de corrente
#define SC1 4
#define SC2 5
#define SC3 6
#define SC4 7
#define SC5 8

//______________________________________________ Leitura sensor luz ______________________________________________
// Definições de Hardware pinos sensor de luz
#define SL1 3
#define SL2 9
#define SL3 10
#define SL4 1

//______________________________________________ RTOS______________________________________________
// Frequência de atualização task
#define TAKS_FREQ_UPDATE_NEOPIXEL 10   // ms
#define TAKS_FREQ_ATUA_MOTOR 10        // ms
#define TASK_FREQ_COLETA_FOTO 10       // ms
#define TASK_FREQ_MODO_TREINO 10       // ms
#define TASK_FREQ_COLETA_CORRENTE 100  // ms
#define TASK_FREQ_REDE_NEURAL 10       // ms

// Tamanho Stack das task
#define STACK_UPDATE_NEOPIXEL 2048  // Memoria real -> 1724
#define STACK_ATUA_MOTOR 2048       // Memoria real -> 1732
#define STACK_COLETA_FOTO 3072      // Memoria real -> 2024
#define STACK_MODO_TREINO 3072      // Memoria real -> 2024
#define STACK_COLETA_CORRENTE 2048
#define STACK_REDE_NEURAL 2048

// Prioridade da tarefa
#define PRIORIDADE_UPDATE_NEOPIXEL 2
#define PRIORIDADE_ATUA_MOTOR 2
#define PRIORIDADE_COLETA_FOTO 2
#define PRIORIDADE_MODO_TREINO 3
#define PRIORIDADE_COLETA_CORRENTE 1
#define PRIORIDADE_REDE_NEURAL 2

// Core da task
#define CORE_UPDATE_NEOPIXEL 1
#define CORE_ATUA_MOTOR 1
#define CORE_COLETA_FOTO 1
#define CORE_MODO_TREINO 1
#define CORE_COLETA_CORRENTE 1
#define CORE_REDE_NEURAL 2

//______________________________________________ Variáveis ______________________________________________

extern bool bArrMotorLiberado[];
extern bool bArrDedoContraido[];
extern bool bModoTreino;
extern bool bCalibDone;

extern const uint8_t MOTOR_IN_1_PIN[];
extern const uint8_t MOTOR_IN_2_PIN[];
extern const uint8_t uiFreqModulacaoNeoPixelAtivado;

extern uint8_t uiConterFreqModulacaoNeoPixel;
extern uint8_t uiContadorTreino;
extern uint8_t uiTreinoE;
extern uint8_t uiTreinoVal;
extern uint8_t uiVetorDedosTreino[];
extern uint8_t uiArrCorNeoPixel[];
extern uint8_t uiArrPwmLevels[];
extern uint8_t uiArrPwmRampa[];
extern uint8_t uiCorNeoPixelInterno[];

extern int iContadorChamdaSensorFoto;
extern int iCorrenteMaxima;

extern SemaphoreHandle_t xNeoPixelMutex;
extern SemaphoreHandle_t xAtuaMotorMutex;
extern SemaphoreHandle_t xSensorFotoMutex;

extern double dTreinoCount;
extern double dArrDadosSensorFotoBruto[4][8];
extern double dArrDadosSensorFoto[4];
extern double dTreinoSum[4];
extern double dTreinoSumSq[4];
extern double dArrSensorCorrente[5];

extern const double dLayerBias[20];
extern const double dW1[4][20];
extern const double dW2[20][8];
extern const double dB1[20];
extern const double dB2[8];
extern const double dCost[8][8];
extern const double dPrior[8];
extern double dMu[4];
extern double dSigma[4];

#endif