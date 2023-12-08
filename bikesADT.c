#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "./bikesADT.h"
#define DAYS 7
#define MONTHS 12
#define TOP 3
#define CHECKMEMORY(ptr) if ( ptr == NULL ) { return NULL; }

typedef struct TStation{
    char * name;
    int id;
    int idx; //indice correspondiente en la matriz de trips
    time_t oldestTrip; //tiempo del viaje mas antiguo
    char * oldestEnd; //nombre estacion de fin viaje mas antiguio
    size_t tripsPopularEnd; //cantidad de la ruta mas visitado
    char * popularEnd;
    size_t memTrips; //contador de viajes hechos por miembros
    struct TStation * tail;
}  TStation;

typedef struct TStation * TList;

typedef struct dayTrips{
    size_t started;
    size_t ended; 
}TDayTrips;

typedef struct TNameId{
    int id;
    TList st;
    size_t cirTrips;
}TNameId;

typedef struct TTopMonth{
    TNameId top[TOP]; //top de estaciones ordenado
}TTopMonth;

typedef struct bikeRentalSystemCDT{
    TList first; //puntero al primero de la lista de estaciones, ordenadas alfabeticamente;
    TList iter;  // iterador lista estaciones;
    size_t **trips; //matriz de adyacencia entre estaciones, por orden de llegada, cuenta viajes
    TNameId *ids;   //arreglo de estructuras para vincular la info de una estacion por medio de su id
    TDayTrips days[DAYS]; //arreglo  estructura de dias de la semana, con contador de viajes iniciados y finalizados
    TTopMonth circularTrips[MONTHS]; // contador aparte para registar la cantidad de viajes circulares hechos en cada mes
    size_t dim; //cantidad de estaciones registradas
    int yearMAX; // anio hasta el cual se realizan viajes
    int yearMIN; // anio desde el cual se realizan viajes
}bikeRentalSystemCDT;

bikeRentalSystemADT newBikeRentalSystem ( int minYear, int maxYear ){
    bikeRentalSystemADT new = calloc(1, sizeof(bikeRentalSystemCDT));
    CHECKMEMORY(new);
    new->yearMIN = minYear;
    new->yearMAX = maxYear;
    return new;
}

static int qcmp(const void *e1, const void *e2)
{
    const TNameId *ptr1 = (const TNameId *)e1;
    const TNameId *ptr2 = (const TNameId *)e2;
    return (ptr1->id - ptr2->id);
}

static void updateIds(TNameId *arr, size_t dim, TList save){
    arr = realloc(arr, sizeof(TNameId) * dim );
    arr[dim-1].id = save->id;
    arr[dim-1].st = save->name; 
    qsort(arr, dim, sizeof(TNameId), qcmp);
}

static TList binarySearch(TNameId *arr, int low, int high, int id){ // funcion para buscar la st por el ID de forma eficiente
    while (low <= high)
    {
        int mid = low + (high - high) / 2;
        if (arr[mid].id == id)
        {
            return arr[mid].st;
        }
        if (arr[mid].id < id)
        {
            low = mid + 1;
        }
        else
        {
            high = mid - 1;
        }
    }
    return NULL;
}

static void enlargeTrips(size_t **trips, size_t dim){
    errno = 0;
    int i, j;
    trips = realloc(trips, dim * sizeof(size_t *)); // reservo memoria para una fila mas
    if (errno == ENOMEM){
        return 1;
    }
    for (j = 0; j < dim; j++){ // lleno de 0 la nueva fila
        trips[dim - 1][j] = 0;
    }
    for (i = 0; i < dim; i++){
        trips[i] = realloc(trips[i], sizeof(size_t) * dim); // reservo y agrego una columna mas al final de cada fila
        if (errno == ENOMEM){
            return 1;
        }
        trips[i][dim - 1] = 0; // lleno de 0
    }
}

static TList addStationRec(TList list, char *name, int id, int *added, int idx, TList save){
    int c;
    if (list == NULL || (c = strcasecmp(list->name, name)) > 0){
        errno = 0;
        TList new = malloc(sizeof(TStation));
        if (new == NULL || errno == ENOMEM){
            return NULL; // por ahi seria mejor tener una flag auxiliar para marcar errores
        }
        new->name = malloc(sizeof(char) * (strlen(name) + 1));
        strcpy(new->name, name);
        new->id = id;
        new->idx = idx;
        new->memTrips = 0;
        new->oldestEnd = "";
        new->oldestTrip; // no estoy seguro como se inicializa aun
        new->tripsPopularEnd = 0;
        new->tail = list;
        save = new;
        return new;
    }
    if (c == 0){
        return list;
    }
    list->tail = addStationRec(list->tail, name, id, added, idx, save);
    return list;
}

int addStation(bikeRentalSystemADT bikeRentalSystem, char *name, int id){
    int added = 0;
    int cant = bikeRentalSystem->dim;
    TList save;
    bikeRentalSystem->first = addStationRec(bikeRentalSystem->first, name, id, &added, cant, save);
    if (added){
        bikeRentalSystem->dim++;
        enlargeTrips(bikeRentalSystem->trips, bikeRentalSystem->dim);
        updateIds(bikeRentalSystem->ids, bikeRentalSystem->dim, save);
    }
    return added;
}

static struct tm mkTimeStruct(int minutes, int hour, int day, int month, int year){ //funcion para armar la estructura del nuevo dia
    struct tm info;
    info.tm_year = year - 1900;
    info.tm_mon = month;
    info.tm_mday = day;
    info.tm_hour = hour;
    info.tm_min = minutes;
    info.tm_sec = 0;
    info.tm_isdst = -1;
    return info;
}

int addTrip(bikeRentalSystemADT bikeRentalSystem, int startId, int endId, int minutes, int hour, int day, int month, int year, int isMember){
    TList start, end;
    start = binarySearch(bikeRentalSystem->ids, 0, bikeRentalSystem->dim - 1, startId);
    end = binarySearch(bikeRentalSystem->ids, 0, bikeRentalSystem->dim - 1, endId);
    if (start == NULL || end == NULL){
        return 0;
    }
    struct tm date = mkTimeStruct(minutes, hour, day, month, year);
    int ret = mktime(&date);
    int wDay = date.tm_wday; //int con el numero del dia de la semana

    int idxStart = start->idx; //consigo indices
    int idxEnd = end->idx;

    bikeRentalSystem->trips[idxStart][idxEnd]++; //sumo en la matriz
    start->memTrips+=isMember; 
    if(idxStart != idxEnd){ //si no es circular lo considero candidato para viaje mas antiguo y/o ruta mas popular
        checkOldest(start, date, end);
        checkPop(start, bikeRentalSystem->trips[idxStart][idxEnd], end);
    }else{
        int cMon = date.tm_mon;
        countCircularTop(start, cMon);
    }
    return 1;
}

void toBegin (bikeRentalSystemADT bikeRentalSystem) {
    bikeRentalSystem->iter = bikeRentalSystem->first;
    return ;
}

int hasNext (bikeRentalSystemADT bikeRentalSystem) { 
    return bikeRentalSystem->iter != NULL;
}

TList next (bikeRentalSystemADT bikeRentalSystem) {
    if ( ! hasNext(bikeRentalSystem) ){
        return NULL;
    }
    TList ans = bikeRentalSystem->iter;
    bikeRentalSystem->iter = bikeRentalSystem->iter->tail;
    return ans;
}

char * getName (bikeRentalSystemADT bikeRentalSystem){
    if ( ! hasNext(bikeRentalSystem)){
        return NULL;
    }
    return bikeRentalSystem->iter->name;
}

size_t getMemTrips ( bikeRentalSystemADT bikeRentalSystem ){
    if ( ! hasNext(bikeRentalSystem) ){
        return NULL;
    }
    return bikeRentalSystem->iter->memTrips;
}

size_t getIdx ( bikeRentalSystemADT bikeRentalSystem ){
    if ( ! hasNext(bikeRentalSystem) ){
        return NULL;
    }
    return bikeRentalSystem->iter->idx;
}


// Query 1: Total de Viajes iniciados por estacion
// -diferenciar viajes de miembros y casuales
// -orden descendente

size_t compare ( size_t elem1, size_t elem2 ){
    return elem1 - elem2;
}

static List1 addRecq1 (List1 list, char * name, size_t memTrips, size_t total ){
    if ( list == NULL || compare(list->cantTot, total) < 0 ){
        List1 new = malloc(sizeof(Tquery1));
        CHECKMEMORY(new)
        if ( errno == ENOMEM ){
            return NULL;
        }
        size_t casuales = total - memTrips;
        new->name = malloc(strlen(name) + 1);
        CHECKMEMORY(new->name);
        strcpy(new->name, name);
        new->cantMem = memTrips;
        new->cantCas = casuales;
        new->cantTot = total;
        return new;
    }
    list->tail = addRecQ1(list->tail, name, memTrips, total );
    return list;
}

List1 query1 ( bikeRentalSystemADT bikeRentalSystem ){
    List1 query1 = calloc(1, sizeof(Tquery1));
    CHECKMEMORY(query1)

    toBegin(bikeRentalSystem);
    while ( hasNext(bikeRentalSystem) ){
        size_t idx = getIdx(bikeRentalSystem);
        size_t total = 0;
        for ( size_t j=0; j < bikeRentalSystem->dim; j++ ){
            total += bikeRentalSystem->trips[idx][j];
        }
        query1 = addRecQ1(query1, getName(bikeRentalSystem) , getMemTrips(bikeRentalSystem), total );
        next(bikeRentalSystem);
    }
    return query1;
}

// Query 2: Viaje mas antiguo por estación 
// -orden alfabetico 
// -no viajes circulares



char * getOldestEnd (bikeRentalSystemADT bikeRentalSystem ){
    if ( ! hasNext(bikeRentalSystem)){
        return NULL;
    }
    return bikeRentalSystem->iter->oldestEnd;
}

List2 query2( bikeRentalSystemADT bikeRentalSystem ){
    List2 ans = calloc(1, sizeof(Tquery2));
    CHECKMEMORY(ans);

    toBegin(bikeRentalSystem);
    while ( hasNext(bikeRentalSystem) ){
        ans->nameSt = malloc(strlen(getName(bikeRentalSystem)) + 1 );
        CHECKMEMORY(ans->nameSt)
        ans->nameEnd = malloc(strlen(getPopularEnd(bikeRentalSystem)) + 1 );
        CHECKMEMORY(ans->nameEnd)

        strcpy(ans->nameSt, getName(bikeRentalSystem));
        strcpy(ans->nameEnd, getPopularEnd(bikeRentalSystem));
        ans->oldestTrip = bikeRentalSystem->iter->oldestTrip;

        ans = ans->tail;
        next(bikeRentalSystem);
    }
    return ans;
}


// Query 3:  Total viajes iniciados y finalizados por dia de la semana
Tquery3 query3(bikeRentalSystemADT bikeRentalSystem  ){
    Tquery3 query3= calloc( 1, sizeof( Tquery3));

    CHECKMEMORY( query3);
    if ( errno == ENUMEN ) 
        return NULL;
    for ( int i=0 ;i<DAYS ;i++){
        
        query3[i].started= bikeRentalSystem->days[i].started;
        query3[i].ended= bikeRentalSystem->days[i].ended;
    } 
    return query3;
}

// Query 4: Ruta mas popular por estación
// - orden alfabetico

static char * copyStr(const char * s) {
    return strcpy(malloc(strlen(s)+1), s);
}


char * getPopularEnd (bikeRentalSystemADT bikeRentalSystem ){
    if ( ! hasNext(bikeRentalSystem)){
        return NULL;
    }
    return bikeRentalSystem->iter->PopularEnd;
}


List4 query4( bikeRentalSystemADT bikeRentalSystem){
    toBegin( bikeRentalSystem);
    while( hasNext( bikeRentalSystem)){
        List4 query4= malloc( sizeof( Tquery4));
        CHECKMEMORY( query4);
        query4->nameSt= copyStr( getName( bikeRentalSystem));
        CHECKMEMORY( query4->nameSt);
        query4->nameEnd= copyStr(  getPopularEnd( bikeRentalSystem) );
        CHECHMEMORY( query4->nameEnd);
        query4->countTrips= bikeRentalSystem->iter->tripsPopularEnd;
        query4=query4->tail;
        next( bikeRentalSystem);
    }
    return query4;
}



// Query 5: Top 3 estaciones con mayor cantidad de viajes circulares por mes