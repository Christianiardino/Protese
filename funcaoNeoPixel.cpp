#include "funcaoNeoPixel.h"

#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <defines.h>

Adafruit_NeoPixel strip(N_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel internalLed(1, INTERNAL_RGB, NEO_GRB + NEO_KHZ800);

void setupNeoPixels() {
    strip.begin();             // Inicializa o driver RMT dedicado
    strip.show();              // Garante que iniciem apagados
    strip.setBrightness(255);  // Brilho máximo (controlado via cor posteriormente)

    // Setup LED Interno
    internalLed.begin();
    internalLed.setBrightness(255);
    internalLed.setPixelColor(0, internalLed.Color(0, 0, 0));  // Inicia apagado
    internalLed.show();

    if (DEBUG_PRINT) {
        printf("[SYS] Adafruit NeoPixel (Fita) inicializado no pino %d\n", NEOPIXEL_PIN);
    }
}

void setInternalLedColor(uint8_t r, uint8_t g, uint8_t b) {
    internalLed.fill(internalLed.Color(r, g, b));
    if (DEBUG_PRINT) {
        printf("setInternalLedColor chamado");
    }
    internalLed.show();
}

void updateNeoPixel_task(void* pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(TAKS_FREQ_UPDATE_NEOPIXEL);

    uint8_t r = 0, g = 0, b = 0;

    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        if (xSemaphoreTake(xNeoPixelMutex, (TickType_t)5) == true) {
            if (uiConterFreqModulacaoNeoPixel < uiFreqModulacaoNeoPixelAtivado) {
                r = 0;
                g = 0;
                b = 0;
                uiConterFreqModulacaoNeoPixel += 1;
            } else {
                r = uiArrCorNeoPixel[0];
                g = uiArrCorNeoPixel[1];
                b = uiArrCorNeoPixel[2];
                uiConterFreqModulacaoNeoPixel = 0;
            }
            xSemaphoreGive(xNeoPixelMutex);
        }

        if (DEBUG_STACK_SIZE) {
            UBaseType_t uxHighWaterMark;
            uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
            printf("[DEBUG] Stack livre mínima Neo Pixel: %d bytes\n", (int)uxHighWaterMark);
        }

        strip.fill(strip.Color(r, g, b));
        strip.show();
    }
}