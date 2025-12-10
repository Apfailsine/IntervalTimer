#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include "models/datastructures.h"
#include "globals.h"

#define BUTTON_PIN 0 // GPIO-Pin für den Button (z. B. GPIO 0)


// Zugangsdaten für den Access Point
const char* ssid = "ESP32_IntervalTimer";
const char* password = "12345678";

// Webserver auf Port 80
WebServer server(80);

unsigned long timeStart = 0.0;
ExerciseState E = ExerciseState::IDLE;

// U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);


void timerTask(void* parameter) {
    for (;;) {
        // ToDO: Input controls via wifi oder BLE implementieren
        // Setzt numSets, timeRep, timeRest, timeStart und unterbrochen entsprechend der Eingaben
        // Sowie die States STARTED, STOPPED, PAUSED, IDLE verwalten

        if (E == ExerciseState::STARTED) {
            // exercise();

        } else if (E == ExerciseState::PAUSED) {
            // Übung pausiert, nichts tun

        } else if (E == ExerciseState::STOPPED) {
            // Übung gestoppt, alles zurücksetzen
            // displayTime(0);
            // Reset aller Variablen und anzeige Stopped
            E = ExerciseState::IDLE;

        } else if (E == ExerciseState::IDLE) {
            LOG_COLOR_D("TimerTask: IDLE state - waiting for start command.\n");
            // Warte auf Startbefehl
            // timeStart = millis();
            // display.clearBuffer();                 // Puffer löschen
            // display.setFont(u8g2_font_ncenB08_tr); // Schriftart setzen
            // display.drawStr(0, 10, "Hello World!"); // Text zeichnen
            // display.sendBuffer();                  // Pufferinhalt anzeigen
            // delay(1000);
        }

        // if (const Exercise* last = webService.lastExercise()) {
        //     Serial.printf("[TimerTask] Last exercise: %s, sets=%u\n",
        //                   last->name.c_str(),
        //                   static_cast<unsigned>(last->sets.size()));
        //     for (size_t i = 0; i < last->sets.size(); ++i) {
        //         const Set& set = last->sets[i];
        //         Serial.printf("  Set %u (%s): intensity=%d%%, reps=%u\n",
        //                       static_cast<unsigned>(i + 1),
        //                       set.label.c_str(),
        //                       set.percentMaxIntensity,
        //                       static_cast<unsigned>(set.reps.size()));
        //     }
        // } else {
        //     Serial.println("[TimerTask] No exercise stored yet.");
        // }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}


// Webserver-Task
void webServerTask(void* parameter) {
    // Access Point starten
    WiFi.softAP(ssid, password);
    Serial.println("Access Point gestartet!");
    Serial.print("IP-Adresse: ");
    Serial.println(WiFi.softAPIP());

    // Routen für den Webserver definieren
    webService.registerRoutes(server);

    // Webserver starten
    server.begin();
    Serial.println("Webserver gestartet!");

    // Webserver-Loop
    while (true) {
        server.handleClient(); // Anfragen bearbeiten
        vTaskDelay(10 / portTICK_PERIOD_MS); // Task kurz pausieren
    }
}

void exercise() {
    




    unsigned long now = millis();
}

// void displayTime(unsigned long millisecs) {
//     // Zeigt die Zeit in min:sec:decimal format an
//     unsigned long totalSeconds = millisecs / 1000;
//     unsigned long minutes = totalSeconds / 60;
//     unsigned long seconds = totalSeconds % 60;
//     unsigned long decimals = (millisecs % 1000) / 10;
//     char timeString[9];
//     sprintf(timeString, "%02lu:%02lu:%02lu", minutes, seconds, decimals);
//     display.clearBuffer();
//     display.setFont(u8g2_font_ncenB08_tr);
//     display.drawStr(0, 30, timeString);
//     display.sendBuffer();

// }

/*
Exercise pushUps("Push-Ups");

    // Set 1 erstellen und hinzufügen
    Set set1(60, 80);
    set1.reps.push_back(Rep(30, 10));
    set1.reps.push_back(Rep(25, 15));
    pushUps.sets.push_back(set1);

    // Set 2 erstellen und hinzufügen
    Set set2(90, 70);
    set2.reps.push_back(Rep(20, 10));
    set2.reps.push_back(Rep(15, 20));
    pushUps.sets.push_back(set2);

*/

void loop() {
    // put your main code here, to run repeatedly:
}



void buttonTask(void* parameter) {
    for (;;) {
        // TODO: Button handling und State-Wechsel implementieren
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void setup() {
    Serial.begin(115200);

    // Webserver-Task starten
    xTaskCreate(
        webServerTask,   // Funktion
        "WebServerTask", // Name des Tasks
        8192,            // Stack-Größe
        NULL,            // Parameter
        1,               // Priorität
        NULL             // Task-Handle
    );

    // Timer-Task starten
    xTaskCreate(
        timerTask,       // Funktion
        "TimerTask",     // Name des Tasks
        2048,            // Stack-Größe
        NULL,            // Parameter
        1,               // Priorität
        NULL             // Task-Handle
    );

    // Button-Task starten
    xTaskCreate(
        buttonTask,       // Funktion
        "ButtonTask",     // Name des Tasks
        2048,             // Stack-Größe
        NULL,             // Parameter
        1,                // Priorität
        NULL              // Task-Handle
    );

    // display.begin();
    unsigned long currentMillis = millis();
    timeStart = currentMillis;
}