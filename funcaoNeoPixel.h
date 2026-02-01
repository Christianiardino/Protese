#ifndef FUNCAO_NEO_PIXEL
#define FUNCAO_NEO_PIXEL
#include <Adafruit_NeoPixel.h>

void setupNeoPixels(); // Renomeado para refletir o uso da biblioteca
void updateNeoPixel_task(void *pvParameters);
void setInternalLedColor(uint8_t r, uint8_t g, uint8_t b);

#endif