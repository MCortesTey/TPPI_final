#include <string.h>
#include "front.h"
#define DELIM ;
#define MAXLINEA 100

int readStation ( const char * file, int station, int id, bikeRentalSystemADT bikeRentalSystem ){
    char line[MAXLINEA], *token, *stationName;
    int error = 0, stationId, i=0;

    while ( fgets(line, sizeof(line), file) != NULL ){
        token = strtok(line, DELIM);

        for (i=0; token != NULL; i++) {
            if ( i == station ){
                stationName = token;

            } else if ( i == id ){
                stationId = atoi(token);

            } 
            token=strtok(NULL, DELIM);
        }
        error = addStation(bikeRentalSystem, stationName, stationId );
    }
    return error;
}