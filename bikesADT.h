#ifndef __bikesADT_h
#define __bikesADT_h
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <time.h>

typedef struct bikeRentalSystemCDT * bikeRentalSystemADT;

// crea nuevo conjunto
bikeRentalSystemADT newBikeRentalSystem(int minYear, int maxYear);

//agrega una estacion en caso de que no estuviera registrada
//retorna 1 si se agrego, 0 si ya estaba registrada
int addStation(bikeRentalSystemADT bikeRentalSystem, char * name, int id);

//registra un viaje y devuelve 1 si se agrego con exito.
int addTrip(bikeRentalSystemADT bikeRentalSystem, int startId, int endId, int day, int month, int year, int isMember);

// apunta al primer elemento
void toBegin (bikeRentalSystemADT bikeRentalSystem);

// retorna 1 si hay elemento siguiente
int hasNext (bikeRentalSystemADT bikeRentalSystem);

// apunta al siguiente elemento
TList next (bikeRentalSystemADT bikeRentalSystem);

// libera el conjunto
void freebikeRentalSystem ( bikeRentalSystemADT bikeRentalSystem );

// retorna nombre de la estacion donde iter apunta
char * getName (bikeRentalSystemADT bikeRentalSystem);

// retorna cantidad de viajes empezados por miembros de la estacion donde iter apunta
size_t getMemTrips ( bikeRentalSystemADT bikeRentalSystem );

// retorna indice correspondiente a la estacion donde iter apunta, en la matriz de trips
size_t getIdx ( bikeRentalSystemADT bikeRentalSystem );

#endif