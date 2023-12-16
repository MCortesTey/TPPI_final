#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "bikesADT.h"
#define CHECKMEMORY(ptr) if ( (ptr) == NULL ) { return NULL; }
#define SECONDSINAMONTH (30*24*60*60)

typedef struct TStation{
    char * name;
    int id;
    int idx; //indice correspondiente en la matriz de trips
    time_t oldestTrip; //tiempo del viaje mas antiguo
    struct tm * oldestStruct;  
    char * oldestEnd; //nombre estacion de fin viaje mas antiguo
    size_t tripsPopularEnd; //cantidad de la ruta mas visitado
    char * popularEnd; // nombre estacion de fin viaje mas popular
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
    TList first; //puntero al primero de la lista de estaciones, ordenadas alfabeticamente
    TList iter;  // iterador lista estaciones
    size_t **trips; //matriz de adyacencia entre estaciones ordenadas por orden de llegada, cuenta viajes
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

static int cirCmp(const void *e1, const void *e2){// ordena ascendentemente, primero por viajes circulares y sino alfabeticamente inverso
    const TNameId *ptr1 = (const TNameId *)e1;
    const TNameId *ptr2 = (const TNameId *)e2;
    if (ptr1->cirTrips == ptr2->cirTrips){ // en caso tengan la misma cantidad, se decide alfabeticamente
        return strcasecmp(ptr2->st->name, ptr1->st->name);
    }
    return (ptr1->cirTrips - ptr2->cirTrips);
}

static TNameId *updateArr(TNameId *arr, size_t dim, TList save, int cir){
    arr = realloc(arr, sizeof(TNameId) * dim);
    arr[dim - 1].id = save->id;
    arr[dim - 1].st = save;
    if (cir){
        arr[dim - 1].cirTrips++;
    }
     qsort(arr, dim, sizeof(TNameId), idCmp);
    return arr;
}



static int checkYear( int year , int min,  int max ){
        return (  min==FREEYEAR ||  (year >= min && (  max==FREEYEAR || year <= max)));
}


static TList binarySearch(TNameId *arr, int low, int high, int id, int cir){ // funcion para buscar la estacion por el ID de forma eficiente
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

void freeTrips(size_t **matrix, int dim){
    for (int i = 0; i < dim; i++){
        free(matrix[i]);
    }
    free(matrix);
}

size_t **enlargeTrips(size_t **trips, const size_t dim, size_t old_dim)
{
    size_t **newTrips = calloc(dim, sizeof(size_t *));

    if (newTrips == NULL || errno == ENOMEM){
        freeTrips( newTrips, dim);

        return NULL; 
    }
    for (size_t index = 0; index < dim; index++){
        newTrips[index] = malloc(dim * sizeof(size_t));
          
        if  ( newTrips[index] == NULL || errno == ENOMEM ){
            freeTrips( newTrips, dim);
            return NULL;
        }
        if (index < old_dim){
            memcpy(newTrips[index], trips[index], old_dim * sizeof(size_t));
            memset(newTrips[index] + old_dim, 0, (dim - old_dim) * sizeof(size_t));
        }
        else{
            memset(newTrips[index], 0, dim * sizeof(size_t));
        }
    }
    freeTrips( trips, old_dim);
    return newTrips;
}


static TList addStationRec(TList list, char *name, int id, int *added, int idx, TList * save, int * memflag){
    int c;
    if (list == NULL || (c = strcasecmp(list->name, name)) > 0){
        errno = 0;
        TList new = malloc(sizeof(TStation));
        if (new == NULL || errno == ENOMEM){
            (*memflag) = 1 ;
            return NULL; // por ahi seria mejor tener una flag auxiliar para marcar errores
        }
        new->name = malloc(sizeof(char) * (strlen(name) + 1));
        strcpy(new->name, name);
        new->id = id;
        new->idx = idx;
        new->memTrips = 0;
        new->oldestTrip = 0; 
        new->oldestEnd = 0;
        new->oldestStruct = malloc(sizeof(struct tm));
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
    list->tail = addStationRec(list->tail, name, id, added, idx, save, memflag);
    return list;
}

int addStation(bikeRentalSystemADT bikeRentalSystem, char *name, int id){
    int added = 0;
    int memflag = 0;
    int cant = bikeRentalSystem->dim;
    TList save;
    bikeRentalSystem->first = addStationRec(bikeRentalSystem->first, name, id, &added, cant, &save, &memflag);
    if ( memflag)
    {
        return 0;
    }
    if (added){
        int old_dim = bikeRentalSystem->dim;
        bikeRentalSystem->dim++;
        bikeRentalSystem->trips = enlargeTrips(bikeRentalSystem->trips, bikeRentalSystem->dim, old_dim);
        bikeRentalSystem->ids = updateArr(bikeRentalSystem->ids, bikeRentalSystem->dim, save, 0);
    }
    return 1;
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

static void checkOldest(TList list, time_t date, struct tm datestruct, TList end){ // chequea si la fecha a registrar es mas antigua
    if (list->oldestTrip == 0 || difftime(list->oldestTrip, date) > 0){ 
        list->oldestTrip = date;
        (*list->oldestStruct) = datestruct;
        list->oldestEnd = end->name;
    }
    return;
}

static time_t dateControl(bikeRentalSystemADT system, char * strDate, int start, struct tm *candidate, int * cMon, int *year){// start = 1 si es de inicio, 0 si es de end
    struct tm date = mkTimeStruct(strDate);
    (*cMon) = date.tm_mon;
    (*year) = date.tm_year+1900;
    if (start){
        (*candidate) = date;
    }
    time_t timeValue = mktime(&date);
    int wDay = date.tm_wday; //numero del dia de la semana
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
    TList aux = binarySearch(mon.Top, 0, mon.dim - 1, start->id, 1); // busca si ya es candidato en el mes
    if (aux == NULL){ 
        mon.dim++;
        mon.Top = updateArr(mon.Top, mon.dim, start, 1);
    }
    return mon;
}

// int addTrip(bikeRentalSystemADT bikeRentalSystem, int startId, int endId, char * startDate, int isMember, char * endDate){
//     TList start, end;
//     struct tm oldestCandidate;
//     int cMonStart, cMonEnd;

//     time_t startTimeValue = dateControl(bikeRentalSystem, startDate, 1, &oldestCandidate, &cMonStart);
//     time_t endTimeValue = dateControl(bikeRentalSystem, endDate, 0, &oldestCandidate, &cMonEnd);

//     if (startId == endId)
//     { // si es viaje circular
//         start = binarySearch(bikeRentalSystem->ids, 0, bikeRentalSystem->dim - 1, startId, 0);
//         end = start;

//         if (start == NULL || end == NULL)
//         {
//             return 0;
//         }

//         if ( cMonStart == cMonEnd && difftime(endTimeValue, startTimeValue) <= SECONDSINAMONTH)
//         {
//             bikeRentalSystem->circularTrips[cMonStart] = countCircularTop(bikeRentalSystem->circularTrips[cMonStart], start);
//         }
//     }
//     else
//     {
//         start = binarySearch(bikeRentalSystem->ids, 0, bikeRentalSystem->dim - 1, startId, 0);
//         end = binarySearch(bikeRentalSystem->ids, 0, bikeRentalSystem->dim - 1, endId, 0);
//         if (start == NULL || end == NULL)
//         {
//             return 0;
//         }
//         checkOldest(start, startTimeValue, oldestCandidate, end);
//     }


//     int idxStart = start->idx; // consigo indices
//     int idxEnd = end->idx;
//     size_t ntrips = ++bikeRentalSystem->trips[idxStart][idxEnd]; // sumo en la matriz
//     start->memTrips += isMember;

//     if (startId != endId)
//     { // si no es circular lo considero candidato para viaje mas antiguo y/o ruta mas popular
//         checkPop(start, ntrips, end);
//     }
//     return 1;
// }



void addTrip(bikeRentalSystemADT bikeRentalSystem, int startId, int endId, char * startDate, int isMember, char * endDate){
    TList start, end;
    struct tm oldestCandidate;
    int cMonStart, cMonEnd;
    int yearStart, yearEnd;


    start = binarySearch(bikeRentalSystem->ids, 0, bikeRentalSystem->dim - 1, startId, 0);
    if ( start == NULL ||((end = binarySearch(bikeRentalSystem->ids, 0, bikeRentalSystem->dim - 1, endId, 0))==NULL))//si start es null ya entra y sino significan que no son iguales
        {
            return ;
        }
    
    time_t startTimeValue = dateControl(bikeRentalSystem, startDate, 1, &oldestCandidate, &cMonStart, &yearStart);
    time_t endTimeValue = dateControl(bikeRentalSystem, endDate, 0, &oldestCandidate, &cMonEnd, &yearEnd);
    int idxStart = start->idx; // consigo indices
    int idxEnd = end->idx;
    size_t ntrips = ++bikeRentalSystem->trips[idxStart][idxEnd]; // sumo en la matriz
    start->memTrips += isMember;

    if (startId == endId){
        if (cMonStart == cMonEnd && difftime(endTimeValue, startTimeValue) <= SECONDSINAMONTH && (checkYear(yearStart, bikeRentalSystem->yearMIN, bikeRentalSystem->yearMAX) && checkYear(yearEnd, bikeRentalSystem->yearMIN, bikeRentalSystem->yearMAX))){
            bikeRentalSystem->circularTrips[cMonStart] = countCircularTop(bikeRentalSystem->circularTrips[cMonStart], start);
        }
    }
    else 
    {
        if (checkYear(yearStart, bikeRentalSystem->yearMIN, bikeRentalSystem->yearMAX) && checkYear(yearEnd, bikeRentalSystem->yearMIN, bikeRentalSystem->yearMAX)){
            checkPop(start, ntrips, end);
        }
        checkOldest(start, startTimeValue, oldestCandidate, end);
    }

    return ;
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
        return 0;
    }
    return bikeRentalSystem->iter->memTrips;
}

size_t getIdx ( bikeRentalSystemADT bikeRentalSystem ){
    if ( ! hasNext(bikeRentalSystem) ){
        return 0;
    }
    return bikeRentalSystem->iter->idx;
}


// Query 1: Total de Viajes iniciados por estacion

size_t compare ( size_t elem1, size_t elem2 ){
    return elem1 - elem2;
}

static TList1 addRecQ1(TList1 list, char *name, size_t memTrips, size_t total){
    int c;
    if (list == NULL || (c = list->cantTot - total) < 0 || (c == 0 && strcasecmp(list->name, name) > 0)){ // en caso de que tengan el mismo total se decide alfabeticamente
        TList1 new = malloc(sizeof(TNode1));
        CHECKMEMORY(new)
        if (errno == ENOMEM){
            return NULL;
        }
        size_t casTrips = total - memTrips;
        new->name = malloc(strlen(name) + 1);
        CHECKMEMORY(new->name);
        strcpy(new->name, name);
        new->cantMem = memTrips;
        new->cantCas = casTrips;
        new->cantTot = total;
        new->tail = list;
        return new;
    }
    list->tail = addRecQ1(list->tail, name, memTrips, total);
    return list;
}

TQuery1 * query1 ( bikeRentalSystemADT bikeRentalSystem ){
    TQuery1 * ans = calloc(1, sizeof(TQuery1));
    CHECKMEMORY(query1)
    toBegin(bikeRentalSystem);
    while ( hasNext(bikeRentalSystem) ){
        size_t idx = getIdx(bikeRentalSystem);
        size_t total = 0;
        for ( size_t j=0; j < bikeRentalSystem->dim; j++ ){
            total += bikeRentalSystem->trips[idx][j];
        }
        ans->first = addRecQ1(ans->first, getName(bikeRentalSystem) , getMemTrips(bikeRentalSystem), total );
        next(bikeRentalSystem);
    }
    return ans;
}

void freeList1 ( TList1 list ){
    if ( list == NULL ){
        return;
    }
    free(list->name);
    freeList1(list->tail);
    free(list);
}

void freeQuery1 ( TQuery1 * q1){
    freeList1(q1->first);
    free(q1);
}

void toBeginQuery1 ( TQuery1 * q1 ){
    q1->iter = q1->first;
    return;
}

int hasNextQuery1( TQuery1 * q1){
    return q1->iter != NULL;
}

void *nextQuery1(TQuery1 * q1){
    if ( !hasNextQuery1( q1)){
        return NULL;
    }
    TList1 ans=q1->iter;
    q1->iter=q1->iter->tail;
    return ans;
    
}

// Query 2: Viaje mas antiguo por estación 

char * getOldestEnd (bikeRentalSystemADT bikeRentalSystem ){
    if ( ! hasNext(bikeRentalSystem)){
        return NULL;
    }
    return bikeRentalSystem->iter->oldestEnd;
}

TQuery2 *query2(bikeRentalSystemADT bikeRentalSystem, int *dim){
    TQuery2 *ans = calloc(1, bikeRentalSystem->dim * sizeof(TQuery2));
    CHECKMEMORY(ans);

    toBegin(bikeRentalSystem);
    int i = 0;
    while ( hasNext(bikeRentalSystem)){
        ans[i].nameSt = malloc(strlen(getName(bikeRentalSystem)) + 1);
        CHECKMEMORY(ans[i].nameSt);
        strcpy(ans[i].nameSt, getName(bikeRentalSystem));
        if (getOldestEnd(bikeRentalSystem) == NULL){
            ans[i].nameEnd = 0;
            ans[i++].oldestTrip = 0;
        }else{
            ans[i].nameEnd = malloc(strlen(getOldestEnd(bikeRentalSystem)) + 1);
            CHECKMEMORY(ans[i].nameEnd);
            strcpy(ans[i].nameEnd, getOldestEnd(bikeRentalSystem));
            ans[i++].oldestTrip = bikeRentalSystem->iter->oldestStruct;
        }

        next(bikeRentalSystem);
    }
    (*dim) = i;
    return ans;
}

void freeQuery2(TQuery2 *q2, int dim){
    for (int i = 0; i < dim; i++){
        free(q2[i].nameSt);
        free(q2[i].nameEnd);
    }
    free(q2);
}

// Query 3:  Total viajes iniciados y finalizados por dia de la semana

TDayTrips * query3(bikeRentalSystemADT bikeRentalSystem ){
    TDayTrips * ans = malloc(sizeof(TDayTrips) * DAYS);
    CHECKMEMORY( ans);
    if ( errno == ENOMEM ) 
        return NULL;
    int i = 0;
    for ( int j=1 ; j<DAYS ;j++){
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

char * getPopularEnd (bikeRentalSystemADT bikeRentalSystem ){
    if ( ! hasNext(bikeRentalSystem)){
        return NULL;
    }
    return bikeRentalSystem->iter->popularEnd;
}

TQuery4 *query4(bikeRentalSystemADT bikeRentalSystem, int *dim){
    toBegin(bikeRentalSystem);
    TQuery4 *ans = malloc(bikeRentalSystem->dim * sizeof(TQuery4));
    CHECKMEMORY(ans);
    int i = 0;
    while (hasNext(bikeRentalSystem)){
        ans[i].nameSt = malloc(strlen(getName(bikeRentalSystem)) + 1);
        CHECKMEMORY(ans[i].nameSt);
        strcpy(ans[i].nameSt, getName(bikeRentalSystem));
        if (getPopularEnd(bikeRentalSystem) == NULL){ //caso de que sea una estacion sin salidas
            ans[i].nameEnd = NULL;
        }else{
            ans[i].nameEnd = malloc(strlen(getPopularEnd(bikeRentalSystem)) + 1);
            CHECKMEMORY(ans[i].nameEnd);
            strcpy(ans[i].nameEnd, getPopularEnd(bikeRentalSystem));
        }
        ans[i++].countTrips = bikeRentalSystem->iter->tripsPopularEnd;
        next(bikeRentalSystem);
    }
    (*dim) = i;
    return ans;
}

void freeQuery4(TQuery4 *vec, int dim){
    for (int i = 0; i < dim; i++){
        free(vec[i].nameSt);
        if (vec[i].nameEnd != NULL){
            free(vec[i].nameEnd);
        }
    }
    free(vec);
    return;
}

// Query 5: Top 3 estaciones con mayor cantidad de viajes circulares por mes

TmonthSt * query5 (bikeRentalSystemADT bikeRentalSystem ){
    TmonthSt * ans = malloc(sizeof(TmonthSt) * MONTHS );
    CHECKMEMORY(ans);
    if ( errno == ENOMEM ) 
        return NULL;
    for (int i=0; i<MONTHS ; i++){
        if ( bikeRentalSystem->circularTrips[i].dim >= 3){
            size_t dim = bikeRentalSystem->circularTrips[i].dim;
            qsort(bikeRentalSystem->circularTrips[i].Top, dim, sizeof(TNameId), cirCmp );

            ans[i].FirstSt = malloc (strlen(bikeRentalSystem->circularTrips[i].Top[dim-1].st->name) + 1);
            CHECKMEMORY(ans[i].FirstSt);
            strcpy(ans[i].FirstSt, bikeRentalSystem->circularTrips[i].Top[dim-1].st->name);

            ans[i].SecondSt = malloc(strlen(bikeRentalSystem->circularTrips[i].Top[dim-2].st->name) + 1);
            CHECKMEMORY(ans[i].SecondSt);
            strcpy(ans[i].SecondSt, bikeRentalSystem->circularTrips[i].Top[dim-2].st->name);

            ans[i].ThirdSt = malloc(strlen(bikeRentalSystem->circularTrips[i].Top[dim-3].st->name) + 1);
            CHECKMEMORY(ans[i].ThirdSt);
            strcpy(ans[i].ThirdSt, bikeRentalSystem->circularTrips[i].Top[dim-3].st->name);
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

// funciones para liberar 

static void freeStations(TList list){
    if (list == NULL){
        return;
    }
    free(list->name);
    free(list->oldestStruct);
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