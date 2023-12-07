#include "queries.h"
#include <errno.h>

// Query 1: Total de Viajes iniciados por estacion
// -diferenciar viajes de miembros y casuales
// -orden descendente

size_t compare ( size_t elem1, size_t elem2 ){
    return elem1 - elem2;
}

static List1 addRec (List1 list, char * name, size_t memTrips, size_t total ){
    if ( list == NULL || compare(list->cantTotales, total) < 0 ){
        List1 new = malloc(1, sizeof(Tquery1));
        CHECKMEMORY(new);
        size_t casuales = total - memTrips;
        new->name = name;
        new->cantMiembros = memTrips;
        new->cantCasuales = casuales;
        new->cantTotales = total;
        return new;
    }
    list->tail = addRec(list->tail, name, memTrips, total );
    return list;
}

List1 query1 ( bikeRentalSystemADT bikeRentalSystem, List1 query1 ){
    toBegin(bikeRentalSystem);
    while ( hasNext(bikeRentalSystem) ){
        size_t idx = getIdx(bikeRentalSystem);
        size_t total = 0;
        for ( size_t j=0; j < bikeRentalSystem->dim; j++ ){
            total += bikeRentalSystem->trips[idx][j];
        }
        query1 = addRec (query1, getName(bikeRentalSystem) , getMemTrips(bikeRentalSystem), total );
        next(bikeRentalSystem);
    }
    return query1;
}

// Query 2: Viaje mas antiguo por estación 
// -orden alfabetico 
// -no viajes circulares


// Query 3:  Total viajes iniciados y finalizados por dia de la semana


// Query 4: Ruta mas popular por estación
// - orden alfabetico

// Query 5: Top 3 estaciones con mayor cantidad de viajes circulares por mes