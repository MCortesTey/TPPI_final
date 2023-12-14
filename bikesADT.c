#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "bikesADT.h"
#define DAYS 7
#define MONTHS 12
#define TOP 3
#define CHECKMEMORY(ptr) if ( ptr == NULL ) { return NULL; }
#define SECONDSINAMONTH (30*24*60*60)

typedef struct TStation{
    char * name;
    int id;
    int idx; //indice correspondiente en la matriz de trips
    time_t oldestTrip; //tiempo del viaje mas antiguo
    struct tm oldestStruct;  
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
static TNameId *updateArr(TNameId *arr, size_t dim, TList save, int cir){
    arr = realloc(arr, sizeof(TNameId) * dim);
    arr[dim - 1].id = save->id;
    arr[dim - 1].st = save;
    if (cir){
        arr[dim - 1].cirTrips++;
        qsort(arr, dim, sizeof(TNameId), cirCmp);
    }
    else{
        qsort(arr, dim, sizeof(TNameId), idCmp);
    }
    return arr;
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

size_t **enlargeTrips(size_t **trips, const size_t dim, size_t old_dim)
{
    size_t **newTrips = calloc(dim, sizeof(size_t *));

    if (newTrips == NULL || errno == ENOMEM){
        return NULL; // seria mejor devolverlo con una flag
    }
    for (size_t index = 0; index < dim; index++){
        if ((newTrips[index] = malloc(dim * sizeof(size_t **))) == NULL || errno == ENOMEM){ 
            return NULL;
        }
        if (index < old_dim){
            memcpy(newTrips[index], trips[index], old_dim * sizeof(size_t **));
            memset(newTrips[index] + old_dim, 0, (dim - old_dim) * sizeof(size_t **));
        }
        else{
            memset(newTrips[index], 0, dim * sizeof(size_t **));
        }
    }
    // libero la matriz vieja
    for (size_t index = 0; index < old_dim; index++){
        free(trips[index]);
    }
    free(trips);
    return newTrips;
}

static TList addStationRec(TList list, char *name, int id, int *added, int idx, TList * save){
    int c;
    if (list == NULL || (c = strcasecmp(list->name, name)) > 0){
        errno = 0;
        TList new = malloc(sizeof(TStation));
        if (new == NULL || errno == ENOMEM){
            return NULL; // por ahi seria mejor tener una flag auxiliar para marcar errores
        }
        // new->name = malloc(sizeof(char) * (strlen(name) + 1));
        // strcpy(new->name, name);
        new->name = name;
        new->id = id;
        new->idx = idx;
        new->memTrips = 0;
        new->oldestTrip = 0; // no estoy seguro como se inicializa aun
        new->oldestEnd = 0;
        new->tripsPopularEnd = 0;
        new->popularEnd = 0;
        new->tail = list;
        (*save) = new;
        (*added) = 1;
        return new;
    }
    if (c == 0){
        return list;
    }
    list->tail = addStationRec(list->tail, name, id, added, idx, &save);
    return list;
}

int addStation(bikeRentalSystemADT bikeRentalSystem, char *name, int id){
    int added = 0;
    int cant = bikeRentalSystem->dim;
    TList save;
    bikeRentalSystem->first = addStationRec(bikeRentalSystem->first, name, id, &added, cant, &save);
    if (added){
        int old_dim = bikeRentalSystem->dim++;
        bikeRentalSystem->trips = enlargeTrips(bikeRentalSystem->trips, bikeRentalSystem->dim, old_dim);
        bikeRentalSystem->ids = updateArr(bikeRentalSystem->ids, bikeRentalSystem->dim, save, 0);
    }
    return added;
}

static struct tm mkTimeStruct(char * date){ //funcion para armar la estructura del nuevo dia
    struct tm info;
    //yyyy-MM-dd HH:mm:ss
    int year, month, day , hour, min, sec; 
    sscanf( date, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &min, &sec );
    info.tm_year = year - 1900;
    info.tm_mon = month - 1;
    info.tm_mday = day;
    info.tm_hour = hour;
    info.tm_min = min;
    info.tm_sec = sec;
    info.tm_isdst = -1;
    return info;
}

static void countToDay(bikeRentalSystemADT system, int wDay, int flagStart ){
    if(flagStart){
        system->days[wDay].started++;
        
    }else{
         system->days[wDay].ended++;
    }
    return ;
}



static void checkOldest(TList list, time_t date, struct tm datestruct, TList end){
    if (list->oldestTrip == 0 || difftime(list->oldestTrip, date) > 0){ // si la fecha a registrar es mas vieja, pasa a ser la oldest
        list->oldestTrip = date;
        list->oldestStruct = datestruct;
        list->oldestEnd = end->name;
    }
    return;
}
static time_t dateControl(bikeRentalSystemADT system, char * strDate, int start, struct tm *candidate){// start = 1 si es de inicio, 0 si es de end
    struct tm date = mkTimeStruct(strDate);
    if (start){
        (*candidate) = date;
    }
    time_t timeValue = mktime(&date);
    int wDay = date.tm_wday; // int con el numero del dia de la semana
    countToDay(system, wDay, start);
    return timeValue;
}

static void checkPop(TList list, size_t ntrips, TList end){
    if (list->tripsPopularEnd <= ntrips){ // si es que son iguales tengo que verificar que sea alfabeticamente menor
        if (list->tripsPopularEnd == 0){ //primer registro
            list->popularEnd = end->name;
            list->tripsPopularEnd = ntrips;
            return;
        }
        int c;
        if ((c = strcasecmp(list->popularEnd, end->name)) == 0){ // si son el mismo tengo que sumar
            list->tripsPopularEnd++;
        }
        else if (c > 0){ // si viene antes alfabeticamente hay nuevo popular
            list->popularEnd = end->name;
            list->tripsPopularEnd = ntrips;
        }
    }
    return;
}

static TTopMonth countCircularTop(TTopMonth mon, TList start){
    TList aux = binarySearch(mon.Top, 0, mon.dim - 1, start->id, 1); // busco si ya es candidato en el mes
    if (aux != NULL){ // en caso de que estuviera solo ordeno
        qsort(mon.Top, mon.dim, sizeof(TNameId), cirCmp);
    }else{
        mon.dim++;
        mon.Top = updateArr(mon.Top, mon.dim, start, 1);
    }
    return mon;
}

int addTrip(bikeRentalSystemADT bikeRentalSystem, int startId, int endId, char * startDate, int isMember, char * endDate){
    TList start, end;
    struct tm oldestCandidate;
    int cMon;

    time_t startTimeValue = dateControl(bikeRentalSystem, startDate, 1, &oldestCandidate);
    time_t endTimeValue = dateControl(bikeRentalSystem, endDate, 0, &oldestCandidate);
    if (startId == endId){ // si es viaje circular
        start = binarySearch(bikeRentalSystem->ids, 0, bikeRentalSystem->dim - 1, startId, 0);
        end = start;
        if (start == NULL || end == NULL)
            return 0;
        if (difftime(endTimeValue, startTimeValue) > SECONDSINAMONTH){
            bikeRentalSystem->circularTrips[cMon] = countCircularTop(bikeRentalSystem->circularTrips[cMon], start);
        }
    }else{
        start = binarySearch(bikeRentalSystem->ids, 0, bikeRentalSystem->dim - 1, startId, 0);
        end = binarySearch(bikeRentalSystem->ids, 0, bikeRentalSystem->dim - 1, endId, 0);
        if (start == NULL || end == NULL)
            return 0;
        checkOldest(start, startTimeValue, oldestCandidate, end);
    }

    int idxStart = start->idx; // consigo indices
    int idxEnd = end->idx;
    size_t ntrips = ++bikeRentalSystem->trips[idxStart][idxEnd]; // sumo en la matriz
    start->memTrips += isMember;
    if (startId != endId){ // si no es circular lo considero candidato para viaje mas antiguo y/o ruta mas popular
        checkPop(start, ntrips, end);
    }
    return 1;
}


static struct  time_conversion( bikeRentalSystemADT bikeRental){

}




void toBegin (bikeRentalSystemADT bikeRentalSystem) {
    bikeRentalSystem->iter = bikeRentalSystem->first;
    return ;
}

int hasNext (bikeRentalSystemADT bikeRentalSystem) { 
    return bikeRentalSystem->iter != NULL;
}

void * next (bikeRentalSystemADT bikeRentalSystem) {
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
        ans->oldestTrip = bikeRentalSystem->iter->oldestStruct;

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
    int i = 0;
    for ( int j=1 ; i<DAYS ;j++){
        ans[i].started= bikeRentalSystem->days[j].started;
        ans[i++].ended= bikeRentalSystem->days[j].ended;
    } 
    ans[i].started= bikeRentalSystem->days[0].started;
    ans[i].ended= bikeRentalSystem->days[0].ended;
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
            ans[i].FirstSt = "Empty";
            ans[i].SecondSt = "Empty";
            ans[i].ThirdSt = "Empty";
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

static void freeStations(TList list){
    if (list == NULL){
        return;
    }
    freeStations(list->tail);
    free(list);
}

static void freeMonths(TTopMonth *months){
    for (int i = 0; i < MONTHS; i++){
        free(months[i].Top);
    }
}

void freeBikeRentalSystem(bikeRentalSystemADT bikeRentalSystem){
    freeStations(bikeRentalSystem->first);
    freeTrips(bikeRentalSystem->trips, bikeRentalSystem->dim);
    freeMonths(bikeRentalSystem->circularTrips);
    free(bikeRentalSystem->ids);
    free(bikeRentalSystem);
}