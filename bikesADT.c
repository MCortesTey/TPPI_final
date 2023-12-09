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

typedef struct TNameId{
    int id;
    TList st;
    size_t cirTrips;
}TNameId;

typedef struct query5{
    int id;
    char* st;
    size_t cirTrips;
    struct query5 * tail;
}Tquery5;

typedef Tquery5 * TList5;

typedef struct TTopMonth{
    TNameId * Top; //top de estaciones ordenado
    size_t dim;
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

static int idCmp(const void *e1, const void *e2){
    const TNameId *ptr1 = (const TNameId *)e1;
    const TNameId *ptr2 = (const TNameId *)e2;
    return (ptr1->id - ptr2->id);
}

static int cirCmp(const void *e1, const void *e2){
    const TNameId *ptr1 = (const TNameId *)e1;
    const TNameId *ptr2 = (const TNameId *)e2;
    return (ptr1->cirTrips - ptr2->cirTrips);
}
static void updateArr(TNameId *arr, size_t dim, TList save, int cir){
    arr = realloc(arr, sizeof(TNameId) * dim );
    arr[dim-1].id = save->id;
    arr[dim-1].st = save->name;
    if(cir){
        arr[dim-1].cirTrips ++;
        qsort(arr, dim, sizeof(TNameId), cirCmp);
    }else{
        qsort(arr, dim, sizeof(TNameId), idCmp);
    }
    return;
}

static TList binarySearch(TNameId *arr, int low, int high, int id, int cir){ // funcion para buscar la st por el ID de forma eficiente
    while (low <= high){
        int mid = low + (high - low) / 2;
        if (arr[mid].id == id){
            if(cir){
                arr[mid].cirTrips++;
            }
            return arr[mid].st;
        }
        if (arr[mid].id < id){
            low = mid + 1;
        }
        else{
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
        new->oldestTrip = 0;// no estoy seguro como se inicializa aun
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
        updateArr(bikeRentalSystem->ids, bikeRentalSystem->dim, save, 0);
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

static void countToDay(TDayTrips * days, int wDayStart, int wDayEnd){
    days[wDayStart].started++;
    days[wDayEnd].ended++;
}

static void checkOldest(TList list, time_t date, TList end){
    if(list->oldestTrip == 0 || difftime(list->oldestTrip, date) > 0){ //si la fecha a registrar es mas vieja, pasa a ser la oldest
        list->oldestTrip = date;
        list->oldestEnd = end->name;
    }
    return;
}

static void checkPop(TList list, size_t ntrips, TList end){
    if( list->tripsPopularEnd <= ntrips){ //si es que son iguales tengo que verificar que sea alfabeticamente menor
        int c;
        if((c = strcasecmp(list->oldestEnd, end->name)) == 0){// si son el mismo tengo que sumar
            list->tripsPopularEnd++;
        }else if(c > 0 ){ //si viene antes alfabeticamente hay nuevo popular
            list->popularEnd = end->name;
            list->tripsPopularEnd = ntrips;
        }
    }
    return;
}

static void countCircularTop(TTopMonth mon, TList start){
    TList aux = binarySearch(mon.Top, 0, mon.dim-1, start->id, 1); //busco si ya es candidato en el mes
    if (aux != NULL){ //en caso de que estuviera solo ordeno
        qsort(mon.Top, mon.dim, sizeof(TNameId), cirCmp);
        return;
    }
    mon.dim++;
    updateArr(mon.Top, mon.dim, start, 1);
    return ;
}

int addTrip(bikeRentalSystemADT bikeRentalSystem, int startId, int endId, int iminutes, int ihour, int iday, int imonth, int iyear, int isMember, int fminutes, int fhour, int fday, int fmonth, int fyear)
{
    TList start, end;
    if(startId == endId){ //si es viaje circular
        start = binarySearch(bikeRentalSystem->ids, 0, bikeRentalSystem->dim - 1, startId, 0);
        end = start;
    } else{
        start = binarySearch(bikeRentalSystem->ids, 0, bikeRentalSystem->dim - 1, startId, 0);
        end = binarySearch(bikeRentalSystem->ids, 0, bikeRentalSystem->dim - 1, endId, 0);
    }
    if (start == NULL || end == NULL){
        return 0;
    }
    struct tm dateStart = mkTimeStruct(iminutes, ihour, iday, imonth, iyear);
    time_t startTimeValue = mktime(&dateStart);
    int wDayStart = dateStart.tm_wday; // int con el numero del dia de la semana

    struct tm dateEnd = mkTimeStruct(fminutes, fhour, fday, fmonth, fyear);
    time_t endTimeValue = mktime(&dateEnd);
    int wDayEnd = dateStart.tm_wday;

    countToDay(bikeRentalSystem->days, wDayStart, wDayEnd );

    int idxStart = start->idx; //consigo indices
    int idxEnd = end->idx;

    bikeRentalSystem->trips[idxStart][idxEnd]++; //sumo en la matriz
    start->memTrips+=isMember; 
    if(idxStart != idxEnd){ //si no es circular lo considero candidato para viaje mas antiguo y/o ruta mas popular
        checkOldest(start, startTimeValue, end);
        checkPop(start, bikeRentalSystem->trips[idxStart][idxEnd], end);
    }else{
        int cMon = dateStart.tm_mon;
        countCircularTop(bikeRentalSystem->circularTrips[cMon], start);
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

void* next (bikeRentalSystemADT bikeRentalSystem) {
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

static TList1 addRecq1 (TList1 list, char * name, size_t memTrips, size_t total ){
    if ( list == NULL || compare(list->cantTot, total) < 0 ){
        TList1 new = malloc(sizeof(Tquery1));
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

TList1 query1 ( bikeRentalSystemADT bikeRentalSystem ){
    TList1 query1 = calloc(1, sizeof(Tquery1));
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

void freeQuery1 ( TList1 list ){
    if ( list == NULL ){
        return;
    }
    freeQuery1(list->tail);
    free(list);
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

TList2 query2( bikeRentalSystemADT bikeRentalSystem ){
    TList2 ans = calloc(1, sizeof(Tquery2));
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

void freeQuery2 ( TList2 list ){
    if ( list == NULL ){
        return;
    }
    freeQuery2 (list->tail);
    free(list);
}

// Query 3:  Total viajes iniciados y finalizados por dia de la semana
TDayTrips * query3(bikeRentalSystemADT bikeRentalSystem ){
    TDayTrips * ans = malloc(sizeof(TDayTrips) * DAYS);
    CHECKMEMORY( ans);
    if ( errno == ENOMEM ) 
        return NULL;
    for ( int i=0 ; i<DAYS ;i++){
        ans[i].started= bikeRentalSystem->days[i].started;
        ans[i].ended= bikeRentalSystem->days[i].ended;
    } 
    return ans;
}

void freeQuery3 ( TDayTrips * vec ){
    free(vec);
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
    return bikeRentalSystem->iter->popularEnd;
}


TList4 query4( bikeRentalSystemADT bikeRentalSystem){
    toBegin( bikeRentalSystem);
    TList4 ans;
    while( hasNext( bikeRentalSystem)){
        ans= malloc( sizeof( Tquery4));
        CHECKMEMORY( ans);
        ans->nameSt= copyStr( getName( bikeRentalSystem));
        CHECKMEMORY( ans->nameSt);
        ans->nameEnd= copyStr(  getPopularEnd( bikeRentalSystem) );
        CHECKMEMORY( ans->nameEnd);
        ans->countTrips= bikeRentalSystem->iter->tripsPopularEnd;
        ans=ans->tail;
        next( bikeRentalSystem);
    }
    return ans;
}

void freeQuery4 ( TList4 list ){
    if ( list == NULL ){
        return;
    }
    freeQuery4(list->tail);
    free(list);
}


// Query 5: Top 3 estaciones con mayor cantidad de viajes circulares por mes

TmonthSt * query5 (bikeRentalSystemADT bikeRentalSystem ){
    TmonthSt * ans = malloc(sizeof(TmonthSt) * MONTHS );
    CHECKMEMORY(ans);
    if ( errno == ENOMEM ) 
        return NULL;
    for (int i=0; i<MONTHS ; i++){
        if ( bikeRentalSystem->circularTrips[i].dim >= 3){
            ans[i].FirstSt = malloc (strlen(bikeRentalSystem->circularTrips[i].Top[0].st->name) + 1);
            CHECKMEMORY(ans[i].FirstSt);
            strcpy(ans[i].FirstSt, bikeRentalSystem->circularTrips[i].Top[0].st->name);

            ans[i].SecondSt = malloc(strlen(bikeRentalSystem->circularTrips[i].Top[1].st->name) + 1);
            CHECKMEMORY(ans[i].SecondSt);
            strcpy(ans[i].SecondSt, bikeRentalSystem->circularTrips[i].Top[1].st->name);

            ans[i].ThirdSt = malloc(strlen(bikeRentalSystem->circularTrips[i].Top[2].st->name) + 1);
            CHECKMEMORY(ans[i].ThirdSt);
            strcpy(ans[i].ThirdSt, bikeRentalSystem->circularTrips[i].Top[2].st->name);
        } else{
            ans[i].FirstSt = 0;
            ans[i].SecondSt = 0;
            ans[i].ThirdSt = 0;
        }
    }
    return ans;
}

void freeQuery5 ( TmonthSt * vec ){
    for ( int i=0; i<MONTHS; i++ ){
        free(vec[i].FirstSt);
        free(vec[i].SecondSt);
        free(vec[i].ThirdSt);
    }
    free(vec);
}