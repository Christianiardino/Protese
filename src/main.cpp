#include <Arduino.h>
#include <Servo.h>
#include <coletaDadosSensorCorrente.h>
#include <coletaDadosSensorFoto.h>
#include <controleMotor.h>
#include <defines.h>
#include <funcaoNeoPixel.h>
#include <modoTreino.h>
#include <redeNeural.h>
#include <webServer.h>

#include "driver/rmt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

void setup() {
    Serial.begin(115200);

    // 1. Configuração de Hardware antecipada
    pinMode(SC1, INPUT);
    pinMode(SC2, INPUT);
    pinMode(SC3, INPUT);
    pinMode(SC4, INPUT);
    pinMode(SC5, INPUT);

    pinMode(SL1, INPUT);
    pinMode(SL2, INPUT);
    pinMode(SL3, INPUT);
    pinMode(SL4, INPUT);

    setupNeoPixels();
    // startWebServer();
    setInternalLedColor(0, 0, 0);

    // Inicializa os servos
    for (int i = 0; i < 5; i++) {
        servoDedos[i].attach(MOTOR_PIN[i]);
        uiTargetPosServo[i] = 0;
    }

    // Sweep dos servos para garantir o funcionamento
    for(int j = 0; j <= 180; j++){
        for (int i = 0; i < 5; i++) {
            servoDedos[i].write(j);
            delay(10);
        }
    }

    for(int j = 180; j >= 0; j--){
        for (int i = 0; i < 5; i++) {
            servoDedos[i].write(j);
            delay(10);
        }
    }

    // Medição de ponto medio de corrente 
    for (int j = 0; j < 5; j++) {
        for (int i = 0; i < 100; i++) {
            iPontoMedioSensorCorrente[j] = (iPontoMedioSensorCorrente[j] + analogRead(SC1)) / 2;
        }
        if (DEBUG_PRINT) {
            printf("[SYS] Ponto medio sensor %d -> %d\n", j, iPontoMedioSensorCorrente[j]);
        }
    }

    // Criação dos semafaros
    xNeoPixelMutex = xSemaphoreCreateMutex();
    if (xNeoPixelMutex == NULL) {
        printf("[ERR] Falha ao criar Mutex update neoPixel.\n");
        return;
    }

    xAtuaMotorMutex = xSemaphoreCreateMutex();
    if (xAtuaMotorMutex == NULL) {
        printf("[ERR] Falha ao criar Mutex atua motor.\n");
        return;
    }

    xSensorFotoMutex = xSemaphoreCreateMutex();
    if (xSensorFotoMutex == NULL) {
        printf("[ERR] Falha ao criar Mutex sensor foto.\n");
        return;
    }

    xEstatisticaMutex = xSemaphoreCreateMutex();
    if (xEstatisticaMutex == NULL) {
        printf("[ERR] Falha ao criar Mutex Estatistica.\n");
        return;
    }

    xSensorCorrenteMutex = xSemaphoreCreateMutex();
    if (xSensorCorrenteMutex == NULL) {
        printf("[ERR] Falha ao criar Mutex Sensor corrente.\n");
        return;
    }

    // 2. Setup task neopixel
    xTaskCreatePinnedToCore(
        updateNeoPixel_task,         // Função da Task
        "updateNeoPixelTask",        // Nome identificador
        STACK_UPDATE_NEOPIXEL,       // Stack Depth
        NULL,                        // Parâmetros
        PRIORIDADE_UPDATE_NEOPIXEL,  // Prioridade (Acima da Idle)
        NULL,                        // Handle
        CORE_UPDATE_NEOPIXEL         // Core ID (1)
    );
    if (DEBUG_PRINT) {
        printf("[SYS] Task NeoPixel instanciada com sucesso.\n");
    }

    // 3. Setup task atua motor
    xTaskCreatePinnedToCore(
        atuaMotor_task,         // Função da Task
        "atuaMotor",            // Nome identificador
        STACK_ATUA_MOTOR,       // Stack Depth
        NULL,                   // Parâmetros
        PRIORIDADE_ATUA_MOTOR,  // Prioridade (Acima da Idle)
        NULL,                   // Handle
        CORE_ATUA_MOTOR         // Core ID (1)
    );
    if (DEBUG_PRINT) {
        printf("[SYS] Task AtuaMotor instanciada com sucesso.\n");
    }

    // 4. Setup task coleta dados sensor foto
    xTaskCreatePinnedToCore(
        coletaDadosSensorFotoeletrico_task,  // Função da Task
        "coletaDadosSensorFoto",             // Nome identificador
        STACK_COLETA_FOTO,                   // Stack Depth
        NULL,                                // Parâmetros
        PRIORIDADE_COLETA_FOTO,              // Prioridade (Acima da Idle)
        NULL,                                // Handle
        CORE_COLETA_FOTO                     // Core ID (1)
    );
    if (DEBUG_PRINT) {
        printf("[SYS] Task coleta dados sensor foto instanciada com sucesso.\n");
    }

    // 5. Setup task modo treino
    xTaskCreatePinnedToCore(
        modoTreinoAtivado_task,  // Função da Task
        "modoTreino",            // Nome identificador
        STACK_MODO_TREINO,       // Stack Depth
        NULL,                    // Parâmetros
        PRIORIDADE_MODO_TREINO,  // Prioridade (Acima da Idle)
        NULL,                    // Handle
        CORE_MODO_TREINO         // Core ID (1)
    );
    if (DEBUG_PRINT) {
        printf("[SYS] Task modo treino criada com sucesso.\n");
    }

    // 6. Setup task leitura corrente
    xTaskCreatePinnedToCore(
        leituraSensorCorrente_task,  // Função da Task
        "leituraSensorCorrente",     // Nome identificador
        STACK_COLETA_CORRENTE,       // Stack Depth
        NULL,                        // Parâmetros
        PRIORIDADE_COLETA_CORRENTE,  // Prioridade (Acima da Idle)
        NULL,                        // Handle
        CORE_COLETA_CORRENTE         // Core ID (1)
    );
    if (DEBUG_PRINT) {
        printf("[SYS] Task coleta dados sensor corrente instanciada com sucesso.\n");
    }

    // 7. Setup task Rede Neural
    xTaskCreatePinnedToCore(
        redeNeural_task,         // Função da Task
        "redeNeural",            // Nome
        STACK_REDE_NEURAL,       // Stack
        NULL,                    // Parametros
        PRIORIDADE_REDE_NEURAL,  // Prioridade
        NULL,                    // Handle
        CORE_REDE_NEURAL         // Core
    );
    if (DEBUG_PRINT) {
        printf("[SYS] Task Rede Neural iniciada.\n");
    }
}

void loop() {
}