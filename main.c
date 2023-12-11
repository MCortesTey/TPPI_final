#include <stdio.h>
#include "htmlTable.h"
#include <errno.h>
#include <stdlib.h>
#include "bikesADT.h"
#include "htmlTable.h"

#define HEADER1"bikeStation;memberTrips;casualTrips;allTrips"
#define HEADER2"bikeStation;bikeEndStation;oldestDateTime"
#define HEADER3"weekDay;startedTrips;endedTrips"
#define HEADER4"bikeStation;mostPopRouteEndStation;mostPopRouteTrips"
#define HEADER5"month;loopsTop1St;loopsTop2St;loopsTop3St"
#define DELIMIT ";"
#define COUNT_Q 5 //cantidad de queries 
#define FILES_PARAMETERS 2 //Cant archivos que hay que leer 
#define MIN_YEAR
#define MAX_YEAR

enum position { FIRST=0, SECOND, THIRD, FOURTH, FIFTH };


void closeFilesHTML (  FILE *files[], int fileCount);//recive una lista de archivos y los cierra
void closeFilesCSV (  FILE *files[], int fileCount);//recive una lista de archivos y los cierra
void printHeaders( FILE *files1[], char* headers[], int fileCount); // Recive dos listas de archivos con el mismo largo y copia los mimos titulos en orden para cada archivo 
FILE * newfileCSV(const char * fileName, char * header );// Recive el nombre del .CSV y los titulos y lo abre, si falla retorna null


int main ( int cantArg, char* args[]){
    //La cantidad de parametros tiene que ser tres ya que es el programa[0], el path al primer .csv[1], y el path al segundo .csv[2]
    if ( cantArg > 5 || cantArg < 3 ){
        fprintf( stderr, "ERROR invalid amount of parameters");
        exit(1);
    }
    int beginYear = 0, endYear = 0;
    if ( cantArg < 4 ){
        beginYear = MIN_YEAR;
        endYear = MAX_YEAR;
    } else if ( cantArg == 4 ){
        beginYear = atoi(args[3]);
        endYear = MAX_YEAR;
    } else if ( cantArg == 5 ){
        beginYear = atoi(args[3]);
        endYear = atoi(args[4]);

        if ( beginYear > endYear ){
            fprintf(stderr, "\nError: Year2<Year1\n");
            return 1;
        }
    }


/*INICIALIZACION */



FILE * info  = fopen( args[1], "r");// archivo de alguileres
FILE * names = fopen( args[2], "r");// archivo de estaciones
FILE * files_data[] ={info, names};


//CSV

FILE * query1_CSV= newfileCSV( "query1.csv", HEADER1);
FILE * query2_CSV= newfileCSV( "query2.csv",HEADER2);
FILE * query3_CSV= newfileCSV( "query3.csv",HEADER3);
FILE * query4_CSV= newfileCSV( "query4.csv",HEADER4);
FILE * query5_CSV= newfileCSV( "query5.csv",HEADER5);
FILE * files_CSV[]={query1_CSV,query2_CSV,query3_CSV,query4_CSV,query5_CSV};


//FALTA VER COMO PONER  LOS NOMBRES DE LAS COLUMNAS ( EL PARAMENTRO RESTANTE)
htmlTable query1_HTML= newTable( "query1.html", 4, "bikeStation", "memberTrips", "casualTrips", "allTrips" );
htmlTable query2_HTML= newTable( "query2.html", 3, "bikeStation" , "bikeEndStation", "oldestDateTime" );
htmlTable query3_HTML= newTable( "query3.html", 3, "weekDay", "startedTrips", "endedTrips" );
htmlTable query4_HTML= newTable( "query4.html", 4, "bikeStation", "mostPopRouteEndStation", "mostPopRouteTrips" );
htmlTable query5_HTML= newTable( "query5.html", 4, "month", "loopsTop1St", "loopsTop2St", "loopsTop3St");
FILE * files_HTML[]={query1_HTML,query2_HTML,query3_HTML,query4_HTML,query5_HTML};

//verfica que los archivos se hayan abierto sin errores
//?


//revisar si no estoy cerrando dos veces un archivo 
for (int i= 0; i<COUNT_Q;i++){
    if ( files_data[0]==NULL || files_data[1] ==NULL ||  (files_CSV[i]==NULL)||(files_HTML[i]==NULL) ){
        closeFiles( files_CSV, COUNT_Q);
        closeHTMLFiles( files_HTML, COUNT_Q);
        closeFiles( files_data, FILES_PARAMETERS );
        fprintf( stderr, "ERROR in openning file");
        exit(1);
       }
}

//Inicializacion del TAD
bikeRentalSystemADT bikeRentalSystem=  newBikeRentalSystem(/*Falta pasar los anos*/) ;

//en el casode ocurrir un error ( como  no hallar memoria para crear el tad), notificamos al usuario 

if  ( bikeRentalSystem == NULL ||  errno == ENOMEM){
        fprintf( stderr, "ERROR memory unavailable");
        closeFiles( files_CSV, COUNT_Q);
        closeHTMLFiles( files_HTML, COUNT_Q);
        closeFiles( files_data,FILES_PARAMETERS );
        exit(1); //verificar si va el 1 u una macro asociada al error 
}


    /*HEADERS*/
//Se imprimen los encabezados para cada archivo



//Arrancamos el iterador 
toBegin( newBikeRentalSystem);


// query1
TList1 q1 = query1(bikeRentalSystem);

while ( q1 ){
    fprintf ( files_CSV[FIRST], "%s;%ld;%ld;%ld\n" , q1->name, q1->cantMem, q1->cantCas, q1->cantTot);

    sprintf (stringMem, "%ld", q1->cantMem );
    sprintf (stringCas, "%ld", q1->cantCas );
    sprintf (stringTot, "%ld", q1->cantTot );

    addHTMLRow(files_HTML[FIRST], q1->name, stringMem, stringCas, stringTot );
    q1 = q1->tail;
}

//query2 
TList2 q2 = query2( bikeRentalSystem);

while ( q2){
    fprintf( files_CSV[SECOND],"%s;%s;%s\n" ,q2->nameSt, q2->nameEnd,/*hay que ver como pasar el q2->oldesttime a strn*/);
    addHTMLRow  ( files_HTML[SECOND],3/*habria que poner macro?*/,q2->nameSt, q2->nameEnd,/*hay que ver como pasar el q2->oldesttime a strn*/ );
    q2=q2->tail;
}

//query3
TDayTrips * q3= query3( bikeRentalSystem);
char* days[]={"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday","Sunday"};
for ( int i=0;i<DAYS;i++){
    fprint( files_CSV[THIRD], "%s;%ld;%ld\n", days[i], q3[i].started, q3[i].ended);
    sprintf (stringStart, "%ld",q3[i].started);
    sprintf (stringEnd, "%ld", q3[i].ended);
    addHTMLRow( files_HTML[THIRD],days[i],stringStart,stringEnd );

}

// query4 
TList4 q4 = query4 ( bikeRentalSystem );
while ( q4 ){
    fprintf( files_CSV[FOURTH], "%s;%s;%ld\n", q4->nameSt , q4->nameEnd, q4->countTrips );

    sprintf(stringTrips, "%ld", q4->countTrips );
    addHTMLRow ( files_HTML[FOURTH], q4->nameSt, q4->nameEnd, stringTrips );
    q4 = q4->tail;
}

// query5
TmonthSt * q5 = query5( bikeRentalSystem );
char * months[]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "Novemeber", "December" };
for ( int i=0; i<MONTHS; i++) {
    fprintf( files_CSV[FIFTH], "%s;%s;%s;%s\n", months[i], q5[i].FirstSt, q5[i].SecondSt, q5[i].ThirdSt );
    addHTMLRow ( files_HTML[FIFTH], months[i], q5[i].FirstSt, q5[i].SecondSt, q5[i].ThirdSt );
}

return 0;
}


//Funcion que crea archivo nuevo de csv y verifica si se creo bien
// se ingresan los headers  por parametro 
FILE * newfileCSV(const char * fileName, char * header )
{
    errno = 0;
    FILE * file = fopen(fileName, "w");
    if ( file == NULL ) {
	    return NULL;
    }	    
    if ( errno == ENOMEM ) {
	    fclose(file);
	    return NULL;
    }
    fprintf(fileName,"%s\n", header );
    return file;
}



//Ya no la necesitamos
void printHeaders( FILE *files1[], char* headers[], int fileCount){
    for( int i=0;i<fileCount;i++){
        fprintf(files1[i], "%s\n", headers[i]);
    }

}
void closeFilesCSV (  FILE *files[], int fileCount){
    for( int i=0;i<fileCount; i++)
    {
        if ( files[i]!=NULL )
        {
            fclose( files[i]);
        }
    }
}
void closeFilesHTML (FILE *files[], int fileCount){
    for( int i=0;i<fileCount; i++)
    {
        if ( files[i]!=NULL )
        {
            closeHTMLTable( files[i]);
        }
    }
}




