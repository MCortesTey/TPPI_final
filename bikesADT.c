#include "./bikesADT.h"
#define DAYS 7
#define MONTHS 12
#define CHECKMEMORY(ptr) if ( ptr == NULL ) {\
                            return NULL; \
                            \}

typedef struct TStation{
    char * name;
    size_t id;
    int idx; //indice correspondiente en la matriz de trips
    //[parametro para que guarde la fecha dell viaje mas antiguo para la estacion]
    size_t monthsCircular[MONTHS]; //contador aparte para registar la cantidad de viajes circulares hechos en cada mes,
}  TStation;

typedef struct TStation * TList;

typedef struct dayTrips{
    size_t started;
    size_t ended; 
}TDayTrips;

typedef struct bikeRentalSystemCDT{
    TList first; //puntero al primero de la lista de estaciones, ordenadas alfabeticamente;
    size_t **trips; //matriz de adyacencia entre estaciones, por orden de llegada, cuenta viajes
    TDayTrips days[DAYS]; //arreglo  estructura de dias de la semana, con contador de viajes iniciados y finalizados
    TList iter; //iterador lista estaciones;
    int year_MAX; // anio hasta el cual se realizan viajes
    int year_MIN; // anio desde el cual se realizan viajes
}bikeRentalSystemCDT;

bikeRentalSystemADT newBikeRentalSystem ( int minYear, int maxYear ){
    bikeRentalSystemADT new = calloc(sizeof(bikeRentalSystemCDT));
    CHECKMEMORY(new);
    new->year_MIN = minYear;
    new->year_MAX = maxYear;
    return new;
}

void toBegin (bikeRentalSystemADT bikeRentalSystemAdt) {
    bikeRentalSystemAdt->iter = bikeRentalSystemAdt->first;
}

int hasNext (bikeRentalSystemADT bikeRentalSystemAdt) { 
    return bikeRentalSystemAdt->iter != NULL;
}

TList next (bikeRentalSystemADT bikeRentalSystemAdt) {
    if ( ! hasNext(bikeRentalSystemAdt) ){
        return NULL
    }
    TList ans = bikeRentalSystemAdt->iter;
    bikeRentalSystemAdt->iter = bikeRentalSystemAdt->iter->tail;
    return ans;
}