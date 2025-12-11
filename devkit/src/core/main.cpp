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
RepState R = RepState::PRE;
static ExerciseRuntime runtime;

namespace {
StorageService::ExerciseId g_selectedExerciseId{};
bool g_hasSelectedExercise = false;

void logSelectedExercise(const StorageService::ExerciseRecord& record) {
    Serial.printf("[Button] Selected exercise: %s (%u sets)\n",
                  record.exercise.name.c_str(),
                  static_cast<unsigned>(record.exercise.sets.size()));
    for (size_t setIndex = 0; setIndex < record.exercise.sets.size(); ++setIndex) {
        const Set& set = record.exercise.sets[setIndex];
        Serial.printf("  Set %u (%s): intensity=%d%%, reps=%u\n",
                      static_cast<unsigned>(setIndex + 1),
                      set.label.c_str(),
                      set.percentMaxIntensity,
                      static_cast<unsigned>(set.reps.size()));
    }
}
} // namespace

// U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);


void timerTask(void* parameter) {
    for (;;) {
        
        // Setzt numSets, timeRep, timeRest, timeStart und unterbrochen entsprechend der Eingaben
        // Sowie die States STARTED, STOPPED, PAUSED, IDLE verwalten
        
        now = millis();

        if (E == ExerciseState::STARTED) {
            // exercise();
            resumeExercise(now);
            if (g_hasSelectedExercise) {
                if (const Exercise* exercise = storageService.findExercise(g_selectedExerciseId)) {
                    Serial.printf("[TimerTask] Übung läuft: %s\n", exercise->name.c_str());
                    // Hier die Logik zum Verarbeiten der Übung implementieren
                    doExerciseStep(*exercise, now);
                } else {
                    Serial.println("[TimerTask] Ausgewählte Übung nicht mehr verfügbar.");
                    g_hasSelectedExercise = false;
                    E = ExerciseState::IDLE;
                }
            } else {
                Serial.println("[TimerTask] Keine Übung ausgewählt.");
                E = ExerciseState::IDLE;
            }

        } else if (E == ExerciseState::PAUSED) {
            // Übung pausiert, nichts tun
            pauseExercise(now);
            LOG_COLOR_D("TimerTask: PAUSED state - exercise is paused.\n");

        } else if (E == ExerciseState::STOPPED) {
            // Übung gestoppt, alles zurücksetzen
            // displayTime(0);
            // Reset aller Variablen und anzeige Stopped
            runtime.active = false;
            runtime.paused = false;
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
            timePrev = millis();
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

void doExerciseStep(const Exercise& exercise, unsigned long now) {
    if (!runtime.active || runtime.paused || runtime.setIndex >= exercise.sets.size()) {
        return;
    }

    const Set& set = exercise.sets[runtime.setIndex];
    const Rep& rep = set.reps[runtime.repIndex];
    unsigned long elapsed = now - runtime.phaseStart;

    switch (runtime.phase) {
    case RepState::PRE:
        if (elapsed >= 3000UL) {
            runtime.phase = RepState::IN_PROGRESS;
            runtime.phaseStart = now;
        } else {
            // display "Get Ready" with countdown
        }
        break;
    case RepState::IN_PROGRESS:
        if (elapsed >= rep.timeRep * 1000UL) {
            runtime.phase = RepState::POST;
            runtime.phaseStart = now;
        } else {
            // display "Exercise" with countdown and intensity
        }
        break;
    case RepState::POST:
        if (elapsed >= rep.timeRest * 1000UL) {
            // zur nächsten Rep (ggf. Setwechsel, Pause nach Set usw.)
            if (++runtime.repIndex < set.reps.size()) {
                runtime.phase = RepState::PRE;
                runtime.phaseStart = now;
            } else {
                // Set erledigt → Set-Pause einleiten
                runtime.phase = RepState::SET_PAUSE;      // zusätzlicher Zustand
                runtime.phaseStart = now;
                runtime.repIndex = 0;
            }
        } else {
            // display "Rest" with countdown
        }
        break;
    case RepState::SET_PAUSE:
        if (elapsed >= set.timePauseAfter * 1000UL) {
            if (++runtime.setIndex < exercise.sets.size()) {
                runtime.phase = RepState::PRE;
                runtime.phaseStart = now;
            } else {
                // gesamte Übung fertig
                runtime.active = false;
                E = ExerciseState::STOPPED;
            }
        } else {
            // display "Set Pause" with countdown
        }
    break;
    }
}

static unsigned long elapsedMs(unsigned long now, unsigned long since) {
    return now - since; // bei Bedarf auf millis()-Overflow anpassen
}

void pauseExercise(unsigned long now) {
    if (!runtime.active || runtime.paused) return;
    runtime.paused = true;
    runtime.pauseOffset = elapsedMs(now, runtime.phaseStart);
}

void resumeExercise(unsigned long now) {
    if (!runtime.active || !runtime.paused) return;
    runtime.paused = false;
    runtime.phaseStart = now - runtime.pauseOffset;
    runtime.pauseOffset = 0;
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


void loop() {
    // put your main code here, to run repeatedly:
}



void buttonTask(void* parameter) {
    static size_t currentExerciseIndex = 0;
    for (;;) {
        // TODO: Button handling und State-Wechsel implementieren
        // load first exercise from storage
        
        const auto& records = storageService.exercises();

        
        // bei short press:
        // if (shortpress){
           if (E == ExerciseState::IDLE){
                if (!records.empty()) {
                    const size_t safeIndex = currentExerciseIndex % records.size();
                    const auto& record = records[safeIndex];
                    currentExerciseIndex = (safeIndex + 1) % records.size();
                    g_selectedExerciseId = record.id;
                    g_hasSelectedExercise = true;
                    logSelectedExercise(record);
                } else {
                    Serial.println("[Button] Keine gespeicherten Übungen vorhanden.");
                }
           }
           else if(E == ExerciseState::PAUSED){
                Serial.println("[Button] Setze Übung fort.");
                E = ExerciseState::STARTED;
           } else if(E == ExerciseState::STARTED){
                Serial.println("[Button] Pausiere Übung.");
                E = ExerciseState::PAUSED;
           }
        //}
        // bei long press:
        // if (longpress){
           if (E == ExerciseState::IDLE){
                if (g_hasSelectedExercise) {
                    if (const Exercise* exercise = storageService.findExercise(g_selectedExerciseId)) {
                        Serial.printf("[Button] Starte Übung: %s\n", exercise->name.c_str());
                        // Hier muss die Übung Starten oder ein Flag gesetzt werden
                        E = ExerciseState::STARTED;
                        // Kann ich außerhalb auf den recied zugreifen?
                        runtime.active = true;
                        runtime.paused = false;
                    } else {
                        Serial.println("[Button] Ausgewählte Übung nicht mehr verfügbar.");
                        g_hasSelectedExercise = false;
                    }
                }
           } else {
               if (const Exercise* exercise = storageService.findExercise(g_selectedExerciseId)){
                   Serial.printf("[Button] Stoppe Übung: %s\n", exercise->name.c_str());
                }
                g_hasSelectedExercise = false;
                E = ExerciseState::STOPPED;
           }
        //}
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