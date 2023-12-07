#include "bikesADT.h"

typedef struct query1 {
    char * name; // nombre de la estacion
    size_t cantMiembros; // cantidad de viajes empezados por miembros
    size_t cantCasuales; // cantidad de viajes empezados por casuales
    size_t cantTotales; // cantidad de viajes empezados totales
    List1 tail;
} Tquery1;

typedef Tquery1 * List1;


List1 query1 ( bikeRentalSystemADT bikeRentalSystem, List1 query1 );