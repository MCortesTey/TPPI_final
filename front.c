#include <string.h>
#include "front.h"
#define DELIM ;
#define MAXLINEA 100
#define MAXDATE 20 

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


int readTrips( const char *file , int membercol ,bikeRentalSystemADT bikeRentalSystem ){
    char line[MAXLINEA];
    char date[MAXDATE], endDate[MAXDATE]; //yyyy-MM-dd HH:mm:ss
    int error =0;
    int Id, endId, membership, ; 

    while( fgets ( line, sizeof( line), file )!=NULL){
        char * token  = strtok( line, DELIM);
        while( token != NULL ) {
            strncpy( date, token);
            token=strtok(NULL, DELIM);

            Id = atoi( token);
            token=strtok(NULL, DELIM);

            strncpy( endDate, token);
            token=strtok(NULL, DELIM);

            endId = atoi( token);
            token=strtok(NULL, DELIM);

            if ( membercol) 
                    token=strtok(NULL, DELIM); //Si member col es 1 significa que no es esta columna  (voy al siguiente) 

            membership  = (strcmp(  token , MEMBER) == 0); //verificar si membership se manejaba con 1 y 0 ;
                                                        
        error= addTrip( bikeRentalSystem, Id, endId, date, membership, endDate); //Hay que corregir la funcion addtrip para que no mezcle front y back y tome el strn como parametro y lo divida en tiempo en el back 
        }   
    }

 return error;
}