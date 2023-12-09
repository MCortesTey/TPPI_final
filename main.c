#include <stdio.h>
#include "htmlTable.h"
#include <errno.h>
#include <stdlib.h>
#include "bikesADT.h"

#define HEADER1"bikeStation;memberTrips;casualTrips;allTrips"
#define HEADER2"bikeStation;bikeEndStation;oldestDateTime"
#define HEADER3"weekDay;startedTrips;endedTrips"
#define HEADER4"bikeStation;mostPopRouteEndStation;mostPopRouteTrips"
#define HEADER5"month;loopsTop1St;loopsTop2St;loopsTop3St"
#define DELIMIT ";"
#define COUNT_Q 5 //cantidad de queries 
#define FILES_PARAMETERS 2 //Cant archivos que hay que leer 


void closeFiles (  FILE * files[], int fileCount);


int main ( int cantArg, char* args[]){
    //La cantidad de parametros tiene que ser tres ya que es el programa[0], el path al primer .csv[1], y el path al segundo .csv[2]
    if ( cantArg !=3)
    {
        fprintf( stderr, "ERROR invalid amount of parameters");
        exit(1);
    }

/*INICIALIZACION */


FILE * info  = fopen( args[1], "r");// archivo de alguileres
FILE * names = fopen( args[2], "r");// archivo de estaciones
FILE * files_data[] ={info, names};


//CSV
FILE * query1_CSV= fopen( "query1.csv","w");
FILE * query2_CSV= fopen( "query2.csv","w");
FILE * query3_CSV= fopen( "query3.csv","w");
FILE * query4_CSV= fopen( "query4.csv","w");
FILE * query5_CSV= fopen( "query5.csv","w");
FILE * files_CSV[]={query1_CSV,query2_CSV,query3_CSV,query4_CSV,query5_CSV};

//HTML
FILE * query1_HTML= fopen( "query1.html","w");
FILE * query2_HTML= fopen( "query2.html","w");
FILE * query3_HTML= fopen( "query3.html","w");
FILE * query4_HTML= fopen( "query4.html","w");
FILE * query5_HTML= fopen( "query5.html","w");
FILE * files_HTML[]={query1_HTML,query2_HTML,query3_HTML,query4_HTML,query5_HTML};

//verfica que los archivos se hayan abierto sin errores
for (int i= 0; i<COUNT_Q;i++){
    if ( files_data[0]==NULL || files_data[1] ==NULL ||  (files_CSV[i]==NULL)||(files_HTML[i]==NULL) )
       {
        closeFiles( files_CSV, COUNT_Q);
        closeFiles( files_HTML, COUNT_Q);
        closeFiles( files_data, FILES_PARAMETERS );
        fprintf( stderr, "ERROR in openning file")
        exit(1);
       }
}

//Inicializacion del TAD
bikeRentalSystemADT rentalSystem=  newBikeRentalSystem() ;

//en el casode ocurrir un error ( como  no hallar memoria para crear el tad), notificamos al usuario 

if  ( rentalSystem == NULL ||  errno == ENOMEM){
        fprintf( stderr, "ERROR memory unavailable")
        closeFiles( files_CSV, COUNT_Q);
        closeFiles( files_HTML, COUNT_Q);
        closeFiles( files_data,FILES_PARAMETERS );
        exit(1); //verificar si va el 1 u una macro asociada al error 
}








    /*HEADERS*/
//Se imprimen los encabezados para cada archivo
fprintf(query1, "%s\n", HEADER1 );
fprintf(query2, "%s\n", HEADER2 );
fprintf(query3, "%s\n", HEADER3 );
fprintf(query4, "%s\n", HEADER4 );
fprintf(query5, "%s\n", HEADER5 );

return 0;
}





void closeFiles (  FILE * files[], int fileCount){
    for( int i=0;i<fileCount, i++)
    {
        if ( files[i]!=NULL )
        {
            fclose( files[i]);
        }
    }
}



