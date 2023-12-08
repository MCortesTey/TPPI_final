#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#ifndef __bikesADT_h
#define __bikesADT_h
#define DAYS 7 


typedef struct query1 {
    char * name; // nombre de la estacion
    size_t cantMem; // cantidad de viajes empezados por miembros
    size_t cantCas; // cantidad de viajes empezados por casuales
    size_t cantTot; // cantidad de viajes empezados totales
    List1 tail;
} Tquery1;


typedef  struct Tdays{
    size_t started ;
    size_t ended; 
}Tdays;


typedef  struct query3 {
    Tdays query3[DAYS];
} Tquery3;


typedef Tquery1 * List1;

typedef struct query2 {
    char * nameSt; // nombre de la estacion de inicio
    char * nameEnd; // nombre de la esatcion de fin
    time_t oldestTrip; // fecha y hora del viaje mas antiguo
    List2 tail;
} Tquery2;

typedef Tquery2

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

List1 query1 ( bikeRentalSystemADT bikeRentalSystem );

List1 query2 ( bikeRentalSystem bikeRentalSystem )

#endif