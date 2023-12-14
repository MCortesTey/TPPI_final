#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#ifndef __bikesADT_h
#define __bikesADT_h
#define DAYS 7 
#define MONTHS 12

typedef struct query1 {
    char * name; // nombre de la estacion
    size_t cantMem; // cantidad de viajes empezados por miembros
    size_t cantCas; // cantidad de viajes empezados por casuales
    size_t cantTot; // cantidad de viajes empezados totales
    struct query1 * tail;
} Tquery1node;

typedef struct query1{
    Tquery1node * first;
    Tquery1node *  iter;
} Tquery1

typedef Tquery1 * TList1;

typedef struct query2 {
    char * nameSt; // nombre de la estacion de inicio
    char * nameEnd; // nombre de la esatcion de fin
    struct tm oldestTrip; // fecha y hora del viaje mas antiguo
} Tquery2;

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
void freebikeRentalSystem ( bikeRentalSystemADT bikeRentalSystem );

// retorna nombre de la estacion donde iter apunta
char * getName (bikeRentalSystemADT bikeRentalSystem);

// retorna cantidad de viajes empezados por miembros de la estacion donde iter apunta
size_t getMemTrips ( bikeRentalSystemADT bikeRentalSystem );

// retorna indice correspondiente a la estacion donde iter apunta, en la matriz de trips
size_t getIdx ( bikeRentalSystemADT bikeRentalSystem );

char * getPopularEnd (bikeRentalSystemADT bikeRentalSystem );

TList1 query1( bikeRentalSystemADT bikeRentalSystem );

// apunta al primer elemento de query1
void toBeginQuery1 ( TList q1 ); 

// retorna 1 si hay elemento siguiente 
int hasNextQuery1 ( TList q1 );

//apunta al siguiente elemento
void * nextQuery1 ( TList q1 );

char * getOldestEnd (bikeRentalSystemADT bikeRentalSystem );

Tquery2 * query2( bikeRentalSystemADT bikeRentalSystem );

TDayTrips * query3(bikeRentalSystemADT bikeRentalSystem  );

TQuery4 *query4(bikeRentalSystemADT bikeRentalSystem, int *dim);

void freeQuery1 ( TList1 list);

void freeQuery2(Tquery2 *q2, int dim);

void freeQuery3(TDayTrips *vec);

void freeQuery4(TQuery4 *vec, int dim);

void freeQuery5(TmonthSt *vec);

#endif