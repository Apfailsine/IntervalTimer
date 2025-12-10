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


#endif // GLOBALS_H
