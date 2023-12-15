#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#ifndef __bikesADT_h
#define __bikesADT_h
#define DAYS 7 
#define MONTHS 12
#define FREEYEAR -1

typedef struct querynode1{
    char *name;     // nombre de la estacion
    size_t cantMem; // cantidad de viajes empezados por miembros
    size_t cantCas; // cantidad de viajes empezados por casuales
    size_t cantTot; // cantidad de viajes empezados totales
    struct querynode1 *tail;
} TNode1;

typedef TNode1 *TList1;

typedef struct query1{
    TList1 first;
    TList1 iter;
} TQuery1;

typedef struct query2 {
    char * nameSt; // nombre de la estacion de inicio
    char * nameEnd; // nombre de la esatcion de fin
    struct tm * oldestTrip; // fecha y hora del viaje mas antiguo
} TQuery2;

typedef struct dayTrips{
    size_t started;
    size_t ended;
} TDayTrips;

typedef struct query4{ 
    char* nameSt; // nombre estacion inicio
    char* nameEnd;// nombre estacion fin
    size_t countTrips;// cantidad de viajes entre ambas estaciones
}TQuery4;

typedef struct monthSt{
    char * FirstSt;
    char * SecondSt;
    char * ThirdSt;
}TmonthSt;

typedef struct bikeRentalSystemCDT * bikeRentalSystemADT;

// crea nuevo conjunto
bikeRentalSystemADT newBikeRentalSystem(int minYear, int maxYear);

//agrega una estacion en caso de que no estuviera registrada
//retorna 1 si se agrego, 0 si ya estaba registrada
int addStation(bikeRentalSystemADT bikeRentalSystem, char * name, int id);

//registra un viaje y devuelve 1 si se agrego con exito.
int addTrip(bikeRentalSystemADT bikeRentalSystem, int startId, int endId, char *startDate, int isMember, char *endDate);

// apunta al primer elemento
void toBegin (bikeRentalSystemADT bikeRentalSystem);

// retorna 1 si hay elemento siguiente
int hasNext (bikeRentalSystemADT bikeRentalSystem);

// apunta al siguiente elemento
void * next (bikeRentalSystemADT bikeRentalSystem);

// libera el conjunto
void freeBikeRentalSystem ( bikeRentalSystemADT bikeRentalSystem );

// retorna nombre de la estacion donde iter apunta
char * getName (bikeRentalSystemADT bikeRentalSystem);

// retorna cantidad de viajes empezados por miembros de la estacion donde iter apunta
size_t getMemTrips ( bikeRentalSystemADT bikeRentalSystem );

// retorna indice correspondiente a la estacion donde iter apunta, en la matriz de trips
size_t getIdx ( bikeRentalSystemADT bikeRentalSystem );

// retorna nombre de estacion de fin mas popular
char * getPopularEnd (bikeRentalSystemADT bikeRentalSystem );

// apunta al primer elemento de query1
void toBeginQuery1 ( TQuery1 * q1 );

// retorna 1 si hay elemento siguiente 
int hasNextQuery1( TQuery1 * q1);

//apunta al siguiente elemento
void *nextQuery1(TQuery1 * q1);

// retorna nombre de estacion de fin mas antigua
char * getOldestEnd (bikeRentalSystemADT bikeRentalSystem );

// queries
TQuery1 * query1 ( bikeRentalSystemADT bikeRentalSystem );

TQuery2 * query2( bikeRentalSystemADT bikeRentalSystem, int * dim );

TDayTrips * query3(bikeRentalSystemADT bikeRentalSystem  );

TQuery4 *query4(bikeRentalSystemADT bikeRentalSystem, int *dim);

void freeQuery1 ( TQuery1 * q1);

void freeQuery2(TQuery2 *q2, int dim2);

void freeQuery3(TDayTrips *vec);

void freeQuery4(TQuery4 *vec, int dim);

void freeQuery5(TmonthSt *vec);

#endif