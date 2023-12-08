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

typedef struct TTopMonth{
    char *top[TOP]; //top de estaciones ordenado
}TTopMonth;

typedef struct TNameId{
    int id;
    TList st;
}TNameId;

typedef struct bikeRentalSystemCDT{
    TList first; //puntero al primero de la lista de estaciones, ordenadas alfabeticamente;
    TList iter;  // iterador lista estaciones;
    size_t **trips; //matriz de adyacencia entre estaciones, por orden de llegada, cuenta viajes
    TNameId *ids;   //arreglo de estructuras para vincular la info de una estacion por medio de su id
    TDayTrips days[DAYS]; //arreglo  estructura de dias de la semana, con contador de viajes iniciados y finalizados
    TTopMonth circularTrips[MONTHS]; // contador aparte para registar la cantidad de viajes circulares hechos en cada mes
    size_t dim; //cantidad de esatciones registradas
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

static void enlargeTrips(size_t ** trips, size_t dim){
    errno = 0;
    int i, j;
    trips = realloc(trips, dim * sizeof(size_t *)); // reservo memoria para una fila mas
    if (errno == ENOMEM){
        return 1;
    }
    for (j = 0 ; j < dim ; j++){ //lleno de 0 la nueva fila
        trips[dim-1][j] = 0;
    }
    for (i = 0; i < dim; i++){
        trips[i] = realloc(trips[i], sizeof(size_t) * dim); // reservo y agrego una columna mas al final de cada fila
        if (errno == ENOMEM){
            return 1;
        }
        trips[i][dim-1] = 0; //lleno de 0
    }
}

static TList addStationRec(TList list, char *name, int id, int * added, int idx, TList save ){
    int c;
    if( list == NULL || (c = strcasecmp(list->name, name)) > 0 ){
        errno = 0;
        TList new = malloc(sizeof(TStation));
        if (new == NULL || errno == ENOMEM){
            return NULL; //por ahi seria mejor tener una flag auxiliar para marcar errores
        }
        new->name = malloc(sizeof(char) * (strlen(name)+1));
        strcpy(new->name, name);
        new->id = id;
        new->idx = idx; 
        new->memTrips = 0;
        new->oldestEnd =""; 
        new->oldestTrip; //no estoy seguro como se inicializa aun 
        new->tripsPopularEnd = 0;
        new->tail = list; 
        save = new;
        return new;
    }
    if(c == 0){
        return list;
    }
    list->tail = addStationRec(list->tail, name, id, added, idx, save);
    return list;
} 

int addStation(bikeRentalSystemADT bikeRentalSystem, char *name, int id){
    int added = 0;
    int cant = bikeRentalSystem->dim;
    TList save;
    bikeRentalSystem->first = addStationRec( bikeRentalSystem->first, name, id, &added, cant, save);
    if(added){
        bikeRentalSystem->dim++;
        enlargeTrips(bikeRentalSystem->trips, bikeRentalSystem->dim);
        updateIds(bikeRentalSystem->ids, bikeRentalSystem->dim, save);
    }
    return added;
}

int addTrip(bikeRentalSystemADT bikeRentalSystem, int startId, int endId, int day, int month, int year, int isMember);

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