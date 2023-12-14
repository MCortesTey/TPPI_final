#include <stdio.h>
#include "htmlTable.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "bikesADT.h"
#include "htmlTable.h"

#ifdef MON
#define CITY 1
#define NAME 1
#define ID 0
#define MEMBERCOL 0
#else
#define CITY 0
#define NAME 0
#define ID 3
#define MEMBERCOL 1

#define MAXLINEA 100
#define MAXDATE 20 

enum { ERR_PAR=1, ERR_YEAR, ERR_OPEN_FILE, ERR_ADD };


#define HEADER1"bikeStation;memberTrips;casualTrips;allTrips"
#define HEADER2"bikeStation;bikeEndStation;oldestDateTime"
#define HEADER3"weekDay;startedTrips;endedTrips"
#define HEADER4"bikeStation;mostPopRouteEndStation;mostPopRouteTrips"
#define HEADER5"month;loopsTop1St;loopsTop2St;loopsTop3St"
#define DELIMIT ";"
#define COUNT_Q 5 //cantidad de queries 
#define FILES_PARAMETERS 2 //Cant archivos que hay que leer 
#define DAYS 7
#define MAXLENGTH 10
#define MAXLENGTH_DATE 20
#define MIN_YEAR
#define MAX_YEAR

enum position { FIRST=0, SECOND, THIRD, FOURTH, FIFTH };


void closeFilesHTML (  FILE *files[], int fileCount);//recive una lista de archivos y los cierra
void closeFilesCSV (  FILE *files[], int fileCount);//recive una lista de archivos y los cierra
void printHeaders( FILE *files1[], char* headers[], int fileCount); // Recive dos listas de archivos con el mismo largo y copia los mimos titulos en orden para cada archivo 
FILE * newfileCSV(const char * fileName, char * header );// Recive el nombre del .CSV y los titulos y lo abre, si falla retorna null
int readStation ( const char * file, int station, int id, bikeRentalSystemADT bikeRentalSystem );
int readTrips( const char *file , int membercol ,bikeRentalSystemADT bikeRentalSystem );


int main ( int cantArg, char* args[]){
//Manejo de errores I: Verifico si se introdujo una cantidad valida de parametros 
    if ( cantArg > 5 || cantArg < 3 ){
        fprintf( stderr, "\nError: invalid amount of parameters\n");
        exit(ERR_PAR);
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

//Manejo de errores II: Verifico que los a;os introducidos sean aptos
        if ( beginYear > endYear ){
            fprintf(stderr, "\nError: Year2<Year1\n");
            exit(ERR_YEAR);
        }
    }



//Inicializacion de archivos de lectura
FILE * trips = fopen( args[1], "r");// archivo de alquileres
FILE * stations = fopen( args[2], "r");// archivo de estaciones
FILE * files_data[] ={ trips, stations };

// Manejo de errores III:  Verifico se se abrieron  bien los archivos .csv de lectura
if ( files_data[FIRST] == NULL || files_data[SECOND] ){
    fprintf (stderr, "\nError: opening file\n");
    closeFiles( files_data,FILES_PARAMETERS );
    exit(ERR_OPEN_FILE);
}

//Inicializacion del TAD
errno= 0; // Lo seteo en 0 para que el unico error en el el errno sea ENOMEM 
bikeRentalSystemADT bikeRentalSystem=  newBikeRentalSystem(beginYear, endYear);


//Manejo de errores IV: Verifico se inicializo correctametne el TAD debido al espacio de memoria 
if  ( bikeRentalSystem == NULL ||  errno == ENOMEM){
    errno=ENOMEM;//Por si entro por el NULL
    fprintf( stderr, "ERROR memory unavailable");
    closeFiles( files_data, FILES_PARAMETERS );
    freebikeRentalSystem(bikeRentalSystem);
    exit(ENOMEM);
}

//Lectura de estaciones de archivo
int error = readStation(files_data[SECOND], NAME, ID, bikeRentalSystem);

//Manejo de errores V: Verifico si se pudieron agrear las estaciones
if ( error ){
    freebikeRentalSystem(bikeRentalSystem);
    closeFiles( files_data, FILES_PARAMETERS );
    exit(ERR_ADD) ;
}

//Lectura de viajes 
error = readTrips( files_data[FIRST], MEMBERCOL, bikeRentalSystem);

//Manejo de errores VI: Verifico si se pudieron agrear los viajes
if ( error ){
    freebikeRentalSystem(bikeRentalSystem);
    closeFiles( files_data, FILES_PARAMETERS );
    exit(ERR_ADD) ;
}



//Inicializacion de archivos de escritura ( .csv y .html) 
//CSV
FILE * query1_CSV= newfileCSV( "query1.csv", HEADER1);
FILE * query2_CSV= newfileCSV( "query2.csv",HEADER2);
FILE * query3_CSV= newfileCSV( "query3.csv",HEADER3);
FILE * query4_CSV= newfileCSV( "query4.csv",HEADER4);
FILE * query5_CSV= newfileCSV( "query5.csv",HEADER5);
FILE * files_CSV[]={query1_CSV,query2_CSV,query3_CSV,query4_CSV,query5_CSV};


//HTML
htmlTable query1_HTML= newTable( "query1.html", 4, "bikeStation", "memberTrips", "casualTrips", "allTrips" );
htmlTable query2_HTML= newTable( "query2.html", 3, "bikeStation" , "bikeEndStation", "oldestDateTime" );
htmlTable query3_HTML= newTable( "query3.html", 3, "weekDay", "startedTrips", "endedTrips" );
htmlTable query4_HTML= newTable( "query4.html", 4, "bikeStation", "mostPopRouteEndStation", "mostPopRouteTrips" );
htmlTable query5_HTML= newTable( "query5.html", 4, "month", "loopsTop1St", "loopsTop2St", "loopsTop3St");
FILE * files_HTML[]={query1_HTML,query2_HTML,query3_HTML,query4_HTML,query5_HTML};



//Manejo de errores VII: Verifico que se hayan abierto correctamente los archivos de escritura
for (int i= 0; i<COUNT_Q;i++){
    if (files_CSV[i]==NULL)||(files_HTML[i]==NULL){
        closeFilesCSV( files_CSV, COUNT_Q);
        closeHTMLFiles( files_HTML, COUNT_Q);
        closeFiles( files_data, FILES_PARAMETERS );
        fprintf( stderr, "ERROR in openning file");
        freebikeRentalSystem(bikeRentalSystem);
        exit(ERR_OPEN_FILE);
       }
}











//Arrancamos el iterador 
toBegin( newBikeRentalSystem);

char stringTrips[MAXLENGTH];
char stringTrips2[MAXLENGTH];
char stringTrips3[MAXLENGTH];


// Upload Query 1 
TList1 q1 = query1(bikeRentalSystem);

while ( q1 ){
    fprintf ( files_CSV[FIRST], "%s;%ld;%ld;%ld\n" , q1->name, q1->cantMem, q1->cantCas, q1->cantTot);

    sprintf (stringTrips, "%ld", q1->cantMem );
    sprintf (stringTrips2, "%ld", q1->cantCas );
    sprintf (stringTrips3, "%ld", q1->cantTot );

    addHTMLRow(files_HTML[FIRST], q1->name, stringTrips, stringTrips2, stringTrips3 );
    q1 = q1->tail;
}

//Upload Query 2 
char dayString[MAXLENGTH_DATE];

TList2 q2 = query2( bikeRentalSystem);

while ( q2){
    strftime(dateString, MAXLENGTH_DATE , "%x %H:%M", q2->oldestTrip );
    fprintf( files_CSV[SECOND],"%s;%s;%s\n" ,q2->nameSt, q2->nameEnd, dateString);
    addHTMLRow  ( files_HTML[SECOND], q2->nameSt, q2->nameEnd, dateString);
    q2=q2->tail;
}

char tripsDay[DAYS][MAXLENGTH];
char tripsDay2[DAYS][MAXLENGTH];

//Upload Query 3
TDayTrips * q3= query3( bikeRentalSystem);
char* days[]={"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday","Sunday"};
for ( int i=0;i<DAYS;i++){
    fprint( files_CSV[THIRD], "%s;%ld;%ld\n", days[i], q3[i].started, q3[i].ended);
    sprintf (tripsDay[i], "%ld",q3[i].started);
    sprintf (tripsDay2[i], "%ld", q3[i].ended);
    addHTMLRow( files_HTML[THIRD],days[i],tripsDay[i],tripsDay2[i] );

}

// Upload Query 4 
TList4 q4 = query4 ( bikeRentalSystem );
while ( q4 ){
    fprintf( files_CSV[FOURTH], "%s;%s;%ld\n", q4->nameSt , q4->nameEnd, q4->countTrips );

    sprintf(stringTrips, "%ld", q4->countTrips );
    addHTMLRow ( files_HTML[FOURTH], q4->nameSt, q4->nameEnd, stringTrips );
    q4 = q4->tail;
}

// Upload Query 5
TmonthSt * q5 = query5( bikeRentalSystem );
char * months[]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "Novemeber", "December" };
for ( int i=0; i<MONTHS; i++) {
    fprintf( files_CSV[FIFTH], "%s;%s;%s;%s\n", months[i], q5[i].FirstSt, q5[i].SecondSt, q5[i].ThirdSt );
    addHTMLRow ( files_HTML[FIFTH], months[i], q5[i].FirstSt, q5[i].SecondSt, q5[i].ThirdSt );
}



// Fin: Cierre de los archivos de escritura
closeFilesCSV(files_CSV, COUNT_Q);
closeFilesHTML(files_HTML, COUNT_Q);

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


