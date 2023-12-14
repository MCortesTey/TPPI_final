#include <stdio.h>
#include "htmlTable.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "bikesADT.h"
#include <ctype.h>

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
#define MAXDATE 20 

enum { ERR_PAR=1, ERR_YEAR, ERR_OPEN_FILE, ERR_READ };


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
#define MAXLINE 100
#define MAXLENGTH_DATE 20
#define FREEYEAR -1


enum position { FIRST=0, SECOND, THIRD, FOURTH, FIFTH };
enum arguments { PROGRAM=0, TRIPS, STATIONS, BEGINYEAR, ENDYEAR}


void closeFilesHTML (  FILE *files[], int fileCount);//recive una lista de archivos y los cierra
void closeFilesCSV (  FILE *files[], int fileCount);//recive una lista de archivos y los cierra
void printHeaders( FILE *files1[], char* headers[], int fileCount); // Recive dos listas de archivos con el mismo largo y copia los mimos titulos en orden para cada archivo 
FILE * newfileCSV(const char * fileName, char * header );// Recive el nombre del .CSV y los titulos y lo abre, si falla retorna null
int readStation ( const char * file, int station, int id, bikeRentalSystemADT bikeRentalSystem );
int readTrips( const char *file , int membercol ,bikeRentalSystemADT bikeRentalSystem );
void error_read_File ( bikeRentalSystemADT bikeRentalSystem, *  FILE *files[], int error, int count   );
int isNum( const char* str);



int main ( int cantArg, char* args[]){
//Manejo de errores I: Verifico si se introdujo una cantidad valida de parametros 
    if ( cantArg > 5 || cantArg < 3 ){
        fprintf( stderr, "\nError: invalid amount of parameters\n");
        exit(ERR_PAR);
    }
    int beginYear = 0, endYear = 0;

    if ( cantArg < 4 ){
        beginYear = FREEYEAR;
        endYear = FREEYEAR;
    } else if ( cantArg == 4 ){
<<<<<<< HEAD
        if( !isNum( args[BEGINYEAR])){//Verifica que los parametros puedan ser pasados a int 
            fprintf( stderr, "\nError: invalid type of parameters\n");
            exit(ERR_PAR);
            }
        beginYear = atoi(args[BEGINYEAR]);
        endYear = MAX_YEAR;
=======
        beginYear = atoi(args[3]);
        endYear = FREEYEAR;
>>>>>>> 25727398359ba8eaadca9075874d93183299872c
    } else if ( cantArg == 5 ){
        if( !isNum( args[BEGINYEAR]) ||!isNum( args[ENDYEAR]) ){//Verifica que los parametros puedan ser pasados a int 
            fprintf( stderr, "\nError: invalid type of parameters\n");
            exit( ERR_PAR);
            }
        beginYear = atoi(args[BEGINYEAR]);
        endYear = atoi(args[ENDYEAR]);



//Manejo de errores II: Verifico que los a;os introducidos sean aptos
        if ( beginYear > endYear ){
            fprintf(stderr, "\nError: Year2<Year1\n");
            exit(ERR_YEAR);
        }
    }

//Inicializacion de archivos de lectura
FILE * trips = fopen( args[TRIPS], "r");// archivo de alquileres
FILE * stations = fopen( args[STATIONS], "r");// archivo de estaciones
FILE * files_data[] ={ trips, stations };

// Manejo de errores III:  Verifico se se abrieron  bien los archivos .csv de lectura
if ( files_data[TRIPS-1] == NULL || files_data[STATIONS-1] ){
    fprintf (stderr, "\nError: opening file\n");
    closeFilesCSV( files_data,FILES_PARAMETERS );
    exit(ERR_OPEN_FILE);
}

//Inicializacion del TAD
errno= 0; // Lo seteo en 0 para que el unico error en el el errno sea ENOMEM 
bikeRentalSystemADT bikeRentalSystem=  newBikeRentalSystem(beginYear, endYear);


//Manejo de errores IV: Verifico se inicializo correctametne el TAD debido al espacio de memoria 
if  ( bikeRentalSystem == NULL ||  errno == ENOMEM){
    errno=ENOMEM;//Por si entro por el NULL
    fprintf( stderr, "ERROR memory unavailable");
    closeFilesCSV( files_data, FILES_PARAMETERS );
    freebikeRentalSystem(bikeRentalSystem);
    exit(ENOMEM);
}

//Lectura de estaciones de archivo
int error = readStation(files_data[STATIONS-1], NAME, ID, bikeRentalSystem);

//Manejo de errores V: Verifico si se pudieron leer las estaciones

error_read_File( bikeRentalSystem, files_data, FILES_PARAMETERS);

//Lectura de viajes 
error = readTrips( files_data[TRIPS-1], MEMBERCOL, bikeRentalSystem);

//Manejo de errores VI: Verifico si se pudieron leer los viajes

error_read_File( bikeRentalSystem, files_data, FILES_PARAMETERS);




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
        closeFilesCSV( files_data, FILES_PARAMETERS );
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

toBeginQuery1 (TList q1);
while ( hasNextQuery1(q1) ){
    fprintf ( files_CSV[FIRST], "%s;%ld;%ld;%ld\n" , q1->iter->name, q1->iter->cantMem, q1->iter->cantCas, q1->iter->cantTot);

    sprintf (stringTrips, "%ld", q1->iter->cantMem );
    sprintf (stringTrips2, "%ld", q1->iter->cantCas );
    sprintf (stringTrips3, "%ld", q1->iter->cantTot );

    addHTMLRow(files_HTML[FIRST], q1->iter->name, stringTrips, stringTrips2, stringTrips3 );
    nextQuery1(q1);
}

//Upload Query 2 
char dayString[MAXLENGTH_DATE];

Tquery2 * q2 = query2( bikeRentalSystem);

for ( int i=0; i<bikeRentalSystem->dim ; i++){
    strftime(dateString, MAXLENGTH_DATE , "%x %H:%M", q2[i].oldestTrip );
    fprintf( files_CSV[SECOND],"%s;%s;%s\n" ,q2[i].nameSt, q2[i].nameEnd, dateString);
    addHTMLRow  ( files_HTML[SECOND], q2[i].nameSt, q2[i].nameEnd, dateString);
}


//Upload Query 3
char tripsDay[DAYS][MAXLENGTH];
char tripsDay2[DAYS][MAXLENGTH];

TDayTrips * q3= query3( bikeRentalSystem);
char* days[]={"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday","Sunday"};
for ( int i=0;i<DAYS;i++){
    fprint( files_CSV[THIRD], "%s;%ld;%ld\n", days[i], q3[i].started, q3[i].ended);
    sprintf (tripsDay[i], "%ld",q3[i].started);
    sprintf (tripsDay2[i], "%ld", q3[i].ended);
    addHTMLRow( files_HTML[THIRD],days[i],tripsDay[i],tripsDay2[i] );
}

// Upload Query 4 
int dim4;
TQuery4 * q4 = query4 ( bikeRentalSystem , &dim4);
for (int i = 0 ; i < dim4 ; i++){
    fprintf( files_CSV[FOURTH], "%s;%s;%ld\n", q4[i].nameSt , q4[i].nameEnd, q4[i].countTrips );

    sprintf(stringTrips, "%ld", q4[i].countTrips );
    addHTMLRow ( files_HTML[FOURTH], q4[i].nameSt, q4[i].nameEnd, stringTrips );
}

// Upload Query 5
TmonthSt * q5 = query5( bikeRentalSystem );
char * months[]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "Novemeber", "December" };
for ( int i=0; i<MONTHS; i++) {
    if ( q5[i].FirstSt != 0 )
        fprintf( files_CSV[FIFTH], "%s;%s;%s;%s\n", months[i], q5[i].FirstSt, q5[i].SecondSt, q5[i].ThirdSt );
        addHTMLRow ( files_HTML[FIFTH], months[i], q5[i].FirstSt, q5[i].SecondSt, q5[i].ThirdSt );
    else {
        fprintf( files_CSV[FIFTH], "%s;%s;%s;%s\n", months[i], "Empty", "Empty", "Empty" );
        addHTMLRow ( files_HTML[FIFTH], months[i], "Empty", "Empty", "Empty");
    }
}


// Fin: Cierre de los archivos de escritura
closeFiles (files_data, FILES_PARAMETERS);

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
            strcpy( date, token);
            token=strtok(NULL, DELIM);

            Id = atoi( token);
            token=strtok(NULL, DELIM);

            strcpy( endDate, token);
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


void error_read_File ( bikeRentalSystemADT bikeRentalSystem, *  FILE *files[], int error, int count   ){
    freebikeRentalSystem(bikeRentalSystem);
    closeFilesCSV( files_data, count );
    exit(ERR_READ) ;
}



int isNum( const char* str){
    for ( int i =0; str[i], i++){
        if ( !isdigit(str[i])){
            return 0;}
    }
    return 1;
}
