#ifndef DEFINES_H
#define DEFINES_H

#include <Arduino.h>
#include <Servo.h>

#include "driver/rmt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#define INTERNAL_RGB 48

//______________________________________________ Atua Motores______________________________________________
// Definições de pinos dos motores
#define MOTOR1 38
#define MOTOR2 39
#define MOTOR3 40
#define MOTOR4 41
#define MOTOR5 42
#define MOTOR_VIB 47

//______________________________________________ Update Neopixel______________________________________________
// Definições de Hardware e Protocolo NEOPIXEL
#define NEOPIXEL_PIN 45
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
#define SC5 1

//______________________________________________ Leitura sensor luz ______________________________________________
// Definições de Hardware pinos sensor de luz
#define SL1 8
#define SL2 3
#define SL3 9
#define SL4 10

//______________________________________________ RTOS______________________________________________
// Frequência de atualização task
#define TAKS_FREQ_UPDATE_NEOPIXEL 10  // ms
#define TAKS_FREQ_ATUA_MOTOR 20       // ms
#define TASK_FREQ_COLETA_FOTO 10      // ms
#define TASK_FREQ_MODO_TREINO 10      // ms
#define TASK_FREQ_COLETA_CORRENTE 30  // ms
#define TASK_FREQ_REDE_NEURAL 10      // ms

// Tamanho Stack das task
#define STACK_UPDATE_NEOPIXEL 4096 
#define STACK_ATUA_MOTOR 4096
#define STACK_COLETA_FOTO 4096
#define STACK_MODO_TREINO 4096
#define STACK_COLETA_CORRENTE 4096
#define STACK_REDE_NEURAL 4096

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
#define CORE_COLETA_FOTO 0
#define CORE_MODO_TREINO 1
#define CORE_COLETA_CORRENTE 0
#define CORE_REDE_NEURAL 1

//______________________________________________ Variáveis ______________________________________________

extern bool DEBUG_STACK_SIZE;
extern bool DEBUG_PRINT;

extern bool bArrMotorLiberado[];
extern bool bArrDedoContraido[];
extern bool bArrSobrecorrenteDetectada[5];
extern bool bModoTreino;
extern bool bCalibDone;

extern const uint8_t MOTOR_PIN[];
extern const uint8_t uiFreqModulacaoNeoPixelAtivado;

extern uint8_t uiConterFreqModulacaoNeoPixel;
extern uint8_t uiContadorTreino;
extern uint8_t uiTreinoE;
extern uint8_t uiTreinoVal;
extern uint8_t uiCliclosCorrenteAlta;
extern uint8_t uiArrContadorCiclosAlta[5];
extern uint8_t uiVetorDedosTreino[];
extern uint8_t uiArrCorNeoPixel[];
extern uint8_t uiCorNeoPixelInterno[];
extern uint8_t uiPosServo[];
extern uint8_t uiLastPosServo[];
extern uint8_t uiTargetPosServo[];
extern uint8_t uiStepAnguloServo;

extern int iContadorChamdaSensorFoto;
extern int iCorrenteMaxima;
extern int iPontoMedioSensorCorrente[];

extern Servo servoDedos[5];

extern SemaphoreHandle_t xNeoPixelMutex;
extern SemaphoreHandle_t xAtuaMotorMutex;
extern SemaphoreHandle_t xSensorFotoMutex;
extern SemaphoreHandle_t xEstatisticaMutex;
extern SemaphoreHandle_t xSensorCorrenteMutex;

extern float fTreinoCount;
extern float fArrDadosSensorFotoBruto[4][8];
extern float fArrDadosSensorFoto[4];
extern float fTreinoSum[4];
extern float fTreinoSumSq[4];
extern float fArrSensorCorrente[5];

extern const float fLayerBias[20];
extern const float fW1[4][20];
extern const float fW2[20][8];
extern const float fB1[20];
extern const float fB2[8];
extern const float fCost[8][8];
extern const float fPrior[8];
extern float fMu[4];
extern float fSigma[4];

#endif