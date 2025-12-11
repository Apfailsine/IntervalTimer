#ifndef GLOBALS_H
#define GLOBALS_H

// core
#include <Arduino.h>
#include "models/datastructures.h"
#include "services/board/board.h"
#include "services/storage/storageservice.h"
#include "services/web/webpage.h"


//+  ######################     HARDWARE SETUP      ########################
// TODO: consider removing sensorType

//*  ####################     END HARDWARE SETUP      ######################


extern BoardService boardService;
extern StorageService storageService;
extern WebService webService;

unsigned long timePrev = 0;
unsigned long timePauseStart = 0;
unsigned long now = 0;
unsigned long timeRep = 0;

#endif // GLOBALS_H
