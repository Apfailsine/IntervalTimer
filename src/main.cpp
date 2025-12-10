#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

// put function declarations here:
int myFunction(int, int);

U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

void setup() {
    display.begin();
}

void loop() {
    display.clearBuffer();                 // Puffer löschen
    display.setFont(u8g2_font_ncenB08_tr); // Schriftart setzen
    display.drawStr(0, 10, "Hello World!"); // Text zeichnen
    display.sendBuffer();                  // Pufferinhalt anzeigen
    delay(1000);
}

