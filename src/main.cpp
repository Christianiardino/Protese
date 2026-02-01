#include <Arduino.h>
#include "driver/rmt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include <defines.h>
#include <webServer.h>
#include <funcaoNeoPixel.h>
#include <controleMotor.h>
#include <coletaDadosSensorFoto.h>
#include <modoTreino.h>
#include <coletaDadosSensorCorrente.h>

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
  startWebServer();
  setInternalLedColor(0, 0, 0);

  //2. Setup task neopixel
  xNeoPixelMutex = xSemaphoreCreateMutex();
  if (xNeoPixelMutex == NULL) {
    if(DEBUG_PRINT){ printf("[ERR] Falha ao criar Mutex update neoPixel.\n"); }
    return;
  }

  xTaskCreatePinnedToCore(
    updateNeoPixel_task,        // Função da Task
    "updateNeoPixelTask",       // Nome identificador
    STACK_UPDATE_NEOPIXEL,      // Stack Depth
    NULL,                       // Parâmetros
    PRIORIDADE_UPDATE_NEOPIXEL, // Prioridade (Acima da Idle)
    NULL,                       // Handle
    CORE_UPDATE_NEOPIXEL        // Core ID (1)
  );
  if(DEBUG_PRINT){ printf("[SYS] Task NeoPixel instanciada com sucesso.\n");}

  //3. Setup task atua motor
  xAtuaMotorMutex = xSemaphoreCreateMutex();
    if (xAtuaMotorMutex == NULL) {
      if(DEBUG_PRINT){ printf("[ERR] Falha ao criar Mutex atua motor.\n"); }
      return;
    }

    xTaskCreatePinnedToCore(
      atuaMotor_task,        // Função da Task
      "atuaMotor",           // Nome identificador
      STACK_ATUA_MOTOR,      // Stack Depth
      NULL,                  // Parâmetros
      PRIORIDADE_ATUA_MOTOR, // Prioridade (Acima da Idle)
      NULL,                  // Handle
      CORE_ATUA_MOTOR        // Core ID (1)
  );
  if(DEBUG_PRINT){ printf("[SYS] Task AtuaMotor instanciada com sucesso.\n");}

  //4. Setup task coleta dados sensor foto
    xTaskCreatePinnedToCore(
      coletaDadosSensorFotoeletrico_task, // Função da Task
      "coletaDadosSensorFoto",            // Nome identificador
      STACK_COLETA_FOTO,                  // Stack Depth
      NULL,                               // Parâmetros
      PRIORIDADE_COLETA_FOTO,             // Prioridade (Acima da Idle)
      NULL,                               // Handle
      CORE_COLETA_FOTO                    // Core ID (1)
  );
  if(DEBUG_PRINT){ printf("[SYS] Task coleta dados sensor foto instanciada com sucesso.\n");}

  //5. Setup task modo treino
    xTaskCreatePinnedToCore(
      modoTreinoAtivado_task, // Função da Task
      "modoTreino",            // Nome identificador
      STACK_MODO_TREINO,                  // Stack Depth
      NULL,                               // Parâmetros
      PRIORIDADE_MODO_TREINO,             // Prioridade (Acima da Idle)
      NULL,                               // Handle
      CORE_MODO_TREINO                    // Core ID (1)
  );
  if(DEBUG_PRINT){ printf("[SYS] Task modo treino criada com sucesso.\n");}

  //6. Setup task leitura corrente
    xTaskCreatePinnedToCore(
      leituraSensorCorrente_task, // Função da Task
      "leituraSensorCorrente",            // Nome identificador
      STACK_COLETA_CORRENTE,                  // Stack Depth
      NULL,                               // Parâmetros
      PRIORIDADE_COLETA_CORRENTE,             // Prioridade (Acima da Idle)
      NULL,                               // Handle
      CORE_COLETA_CORRENTE                    // Core ID (1)
  );
  if(DEBUG_PRINT){ printf("[SYS] Task coleta dados sensor corrente instanciada com sucesso.\n");}
}

void loop() {
}