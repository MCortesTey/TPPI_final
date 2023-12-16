
#include "htmlTable.h"
#include "bikesADT.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef MON
enum stations { ID = 0, NAME, LAT , LONG} ; 
#define MEMBERCOL 0 
#define MEMBER_TYPE '1'
#define MAXLENGTH_DATE 20 //longitud maxima de string de fechas para guardar en archivos html
#else 
enum stations { NAME = 0, LAT, LONG, ID };
#define MEMBERCOL 1
#define MEMBER_TYPE 'm'
#define MAXLENGTH_DATE 27 
#endif

enum { OK=0,ERR_PAR, ERR_YEAR, ERR_OPEN_FILE, ERR_STATION_FILE, ERR_TRIP_FILE };

#define HEADER1 "bikeStation;memberTrips;casualTrips;allTrips"
#define HEADER2 "bikeStation;bikeEndStation;oldestDateTime"
#define HEADER3 "weekDay;startedTrips;endedTrips"
#define HEADER4 "bikeStation;mostPopRouteEndStation;mostPopRouteTrips"
#define HEADER5 "month;loopsTop1St;loopsTop2St;loopsTop3St"
#define DELIMIT ";"
#define COUNT_Q 5 //cantidad de queries 
#define FILES_READ 2 //Cantidad archivos que hay que leer 
#define DAYS 7
#define MAXLENGTH 10 // longitud maxima de string de numeros para guardar en archivos html
#define MAXLINE 100 // longitud maxima de cada linea de los archivos csv



enum position { FIRST=0, SECOND, THIRD, FOURTH, FIFTH }; // posicion para buscar archivos en files_csv y files_htm
enum arguments { PROGRAM=0, TRIPS, STATIONS, BEGINYEAR, ENDYEAR};


void closeFilesHTML (  htmlTable files[], int fileCount);//recibe una lista de archivos HTML y los cierra
void closeFilesCSV (  FILE *files[], int fileCount);//recibe una lista de archivos CSV y los cierra
FILE * newfileCSV(const char * fileName, char * header );// Recibe el nombre del .CSV y los titulos y lo abre, si falla retorna null
int readStation ( FILE * file, int station, int id, bikeRentalSystemADT bikeRentalSystem ); // lee el archivo csv de estaciones 
int readTrips( FILE *file , int membercol ,bikeRentalSystemADT bikeRentalSystem ); // lee el archivo csv de trips
void error_read_File ( bikeRentalSystemADT bikeRentalSystem, FILE *files[], int error, int count );
void close_write_files ( bikeRentalSystemADT bikeRentalSystem,  FILE *filesCSV[], htmlTable filesHTML[], int error, int count );
int isNum( const char* str);


int main ( int cantArg, char* args[]){
//Manejo de errores I: Verifico si se introdujo una cantidad valida de parametros 
    if ( cantArg > 5 || cantArg < 3 ){
        fprintf( stderr, "\nError: invalid amount of parameters\n");
        exit(ERR_PAR);
    }
    int beginYear = FREEYEAR, endYear = FREEYEAR;

    if ( cantArg >= 4 ){
        if( !isNum( args[BEGINYEAR])){//Verifica que los parametros puedan ser pasados a int 
            fprintf( stderr, "\nError: invalid type of parameters\n");
            exit(ERR_PAR);
            }
        beginYear = atoi(args[BEGINYEAR]);

    } if ( cantArg == 5 ){
        if(!isNum( args[ENDYEAR]) ){//Verifica que los parametros puedan ser pasados a int 
            fprintf( stderr, "\nError: invalid type of parameters\n");
            exit( ERR_PAR);
            }
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
if ( files_data[TRIPS-1] == NULL || files_data[STATIONS-1] == NULL ){
    fprintf (stderr, "\nError: opening file\n");
    closeFilesCSV( files_data, FILES_READ );
    exit(ERR_OPEN_FILE);
}

//Inicializacion del TAD
errno= 0; // Lo seteo en 0 para que el unico error en el el errno sea ENOMEM 
bikeRentalSystemADT bikeRentalSystem=  newBikeRentalSystem(beginYear, endYear);


//Manejo de errores IV: Verifico se inicializo correctametne el TAD debido al espacio de memoria 
if  ( bikeRentalSystem == NULL ||  errno == ENOMEM){
    errno=ENOMEM;//Por si entro por el NULL
    fprintf( stderr, "\nError: memory unavailable\n");
    closeFilesCSV( files_data, FILES_READ );
    freeBikeRentalSystem(bikeRentalSystem);
    exit(ENOMEM);
}

//Lectura de estaciones de archivo
int errorStation = readStation(files_data[STATIONS-1], NAME, ID, bikeRentalSystem);

//Manejo de errores V: Verifico si se pudieron leer las estaciones
if ( errorStation){
    fprintf( stderr, "\nError: imposible to read stations file\n");
    error_read_File( bikeRentalSystem, files_data, ERR_STATION_FILE ,FILES_READ);
}

//Lectura de viajes 
int empty= readTrips( files_data[TRIPS-1], MEMBERCOL, bikeRentalSystem);

if ( empty){
    fprintf( stderr, "\nError: File has no trips\n");
    error_read_File( bikeRentalSystem, files_data, ERR_TRIP_FILE ,FILES_READ);
}

//Cerramos archivos de lectura
closeFilesCSV( files_data, FILES_READ ); 



//Inicializacion de archivos de escritura ( .csv y .html) 
//CSV
FILE * query1_CSV= newfileCSV( "query1.csv", HEADER1);
FILE * query2_CSV= newfileCSV( "query2.csv",HEADER2);
FILE * query3_CSV= newfileCSV( "query3.csv",HEADER3);
FILE * query4_CSV= newfileCSV( "query4.csv",HEADER4);
FILE * query5_CSV= newfileCSV( "query5.csv",HEADER5);
FILE * files_CSV[]={query1_CSV, query2_CSV, query3_CSV, query4_CSV, query5_CSV};


//HTML
htmlTable query1_HTML= newTable( "query1.html", 4, "bikeStation", "memberTrips", "casualTrips", "allTrips" );
htmlTable query2_HTML= newTable( "query2.html", 3, "bikeStation" , "bikeEndStation", "oldestDateTime" );
htmlTable query3_HTML= newTable( "query3.html", 3, "weekDay", "startedTrips", "endedTrips" );
htmlTable query4_HTML= newTable( "query4.html", 3, "bikeStation", "mostPopRouteEndStation", "mostPopRouteTrips" );
htmlTable query5_HTML= newTable( "query5.html", 4, "month", "loopsTop1St", "loopsTop2St", "loopsTop3St");
htmlTable files_HTML[]={query1_HTML, query2_HTML, query3_HTML, query4_HTML, query5_HTML};



//Manejo de errores VII: Verifico que se hayan abierto correctamente los archivos de escritura
for (int i= 0; i<COUNT_Q;i++){
    if ((files_CSV[i]==NULL)||(files_HTML[i]==NULL)){
        fprintf( stderr, "ERROR in openning file");
        close_write_files( bikeRentalSystem, files_CSV,files_HTML, ERR_OPEN_FILE, COUNT_Q);
       }
}


// Upload Query 1 
char stringTrips[MAXLENGTH];
char stringTrips2[MAXLENGTH];
char stringTrips3[MAXLENGTH];

TQuery1 * q1 = query1(bikeRentalSystem);

toBeginQuery1 (q1);
while ( hasNextQuery1(q1) ){
    fprintf ( files_CSV[FIRST], "%s;%ld;%ld;%ld\n" , q1->iter->name, q1->iter->cantMem, q1->iter->cantCas, q1->iter->cantTot);

    sprintf (stringTrips, "%ld", q1->iter->cantMem );
    sprintf (stringTrips2, "%ld", q1->iter->cantCas );
    sprintf (stringTrips3, "%ld", q1->iter->cantTot );

    addHTMLRow(files_HTML[FIRST], q1->iter->name, stringTrips, stringTrips2, stringTrips3 );
    nextQuery1(q1);
}
freeQuery1(q1);

//Upload Query 2 
char dayString[MAXLENGTH_DATE];

int dim2;
TQuery2 * q2 = query2( bikeRentalSystem, &dim2);

for ( int i=0; i < dim2 ; i++){
    if (q2[i].nameEnd != NULL){
        strftime(dayString, MAXLENGTH_DATE, "%d/%m/%Y  %H:%M", q2[i].oldestTrip);
        fprintf( files_CSV[SECOND],"%s;%s;%s\n" ,q2[i].nameSt, q2[i].nameEnd, dayString);
        addHTMLRow  ( files_HTML[SECOND], q2[i].nameSt, q2[i].nameEnd, dayString);
    } else{
        fprintf(files_CSV[SECOND], "%s;%s;%s\n", q2[i].nameSt, "empty", "empty");
        addHTMLRow(files_HTML[SECOND], q2[i].nameSt, "empty", "empty");
    }
}
freeQuery2(q2, dim2);


//Upload Query 3
char tripsDay[DAYS][MAXLENGTH];
char tripsDay2[DAYS][MAXLENGTH];

TDayTrips * q3= query3( bikeRentalSystem);
char* days[]={"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday","Sunday"};
for ( int i=0;i<DAYS;i++){
    fprintf( files_CSV[THIRD], "%s;%ld;%ld\n", days[i], q3[i].started, q3[i].ended);
    sprintf (tripsDay[i], "%ld",q3[i].started);
    sprintf (tripsDay2[i], "%ld", q3[i].ended);
    addHTMLRow( files_HTML[THIRD],days[i],tripsDay[i],tripsDay2[i] );
}
freeQuery3(q3);

// Upload Query 4 
int dim4;
TQuery4 * q4 = query4 ( bikeRentalSystem , &dim4);

for (int i = 0 ; i < dim4 ; i++){
    if( q4[i].nameEnd != NULL ){
        fprintf( files_CSV[FOURTH], "%s;%s;%ld\n", q4[i].nameSt , q4[i].nameEnd, q4[i].countTrips );
        sprintf(stringTrips, "%ld", q4[i].countTrips );
        addHTMLRow ( files_HTML[FOURTH], q4[i].nameSt, q4[i].nameEnd, stringTrips );
    }else{
        fprintf(files_CSV[FOURTH], "%s;%s;%ld\n", q4[i].nameSt, "empty", q4[i].countTrips);
        sprintf(stringTrips, "%ld", q4[i].countTrips);
        addHTMLRow(files_HTML[FOURTH], q4[i].nameSt, "empty", stringTrips);
    }
    
}
freeQuery4(q4, dim4);

// Upload Query 5
TmonthSt * q5 = query5( bikeRentalSystem );
char * months[]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "Novemeber", "December" };
for ( int i=0; i<MONTHS; i++) {
    if ( q5[i].FirstSt ){
        fprintf( files_CSV[FIFTH], "%s;%s;%s;%s\n", months[i], q5[i].FirstSt, q5[i].SecondSt, q5[i].ThirdSt );
        addHTMLRow ( files_HTML[FIFTH], months[i], q5[i].FirstSt, q5[i].SecondSt, q5[i].ThirdSt );
    } else {
        fprintf( files_CSV[FIFTH], "%s;%s;%s;%s\n", months[i], "Empty", "Empty", "Empty" );
        addHTMLRow ( files_HTML[FIFTH], months[i], "Empty", "Empty", "Empty");
    }
}
freeQuery5(q5);

// Fin: Cierre de los archivos de escritura
// closeFilesCSV( files_CSV, COUNT_Q);
// closeFilesHTML( files_HTML, COUNT_Q);
// freeBikeRentalSystem(bikeRentalSystem);

close_write_files( bikeRentalSystem, files_CSV,files_HTML, OK, COUNT_Q);  

return OK;
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
    fprintf(file,"%s\n", header );
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

/*Cierra los archivos HTML*/
void closeFilesHTML (htmlTable files[], int fileCount){
    for( int i=0;i<fileCount; i++)
    {
        if ( files[i]!=NULL )
        {
            closeHTMLTable( files[i]);
        }
    }
}

/*Lee las estaciones de un archivo y su id y las agrega al sistema ( toma por parametro las columna )*/
int readStation ( FILE * file, int station, int id, bikeRentalSystemADT bikeRentalSystem ){
    char line[MAXLINE];
    int ok = 0, stationId, i=0;

    fgets ( line, sizeof( line), file ); //La primera linea son titulos
    while ( fgets(line, sizeof(line), file) != NULL )
    {
        char * token = strtok(line, DELIMIT);
        char * stationName ;
        for (i=0; token != NULL; i++) 
        {
           
            if ( i == station )
            { 
                stationName = token;
            } 
            else if ( i == id )
            {
                stationId = atoi(token);
            } 
            token=strtok(NULL, DELIMIT);
        }
        ok = addStation(bikeRentalSystem, stationName, stationId );
        if (ok == 0 ){ // En caso de que falle por error de memoria 
            return 1;
        }
    }
    return 0;
}



int readTrips( FILE *file , int membercol ,bikeRentalSystemADT bikeRentalSystem ){
    char line[MAXLINE];
    char date[MAXLENGTH_DATE], endDate[MAXLENGTH_DATE]; //yyyy-MM-dd HH:mm:ss
    int Id, endId, membership ; 
    int empty=1;

    fgets ( line, sizeof( line), file ); // Saltea la primera linea de titulos

    while( fgets ( line, sizeof( line), file )!=NULL)
    {
        empty=0; //Verifica que no sea un archivo vacio, si lo es e
        char * token  = strtok( line, DELIMIT);
        while( token != NULL )
        {
            
            strcpy( date, token);

            token=strtok(NULL, DELIMIT);
            Id = atoi(token);

            token=strtok(NULL, DELIMIT);
            strcpy( endDate, token);

            token=strtok(NULL, DELIMIT);
            endId = atoi( token);

            token=strtok(NULL, DELIMIT);

            if ( membercol) {
                    token=strtok(NULL, DELIMIT); //Si member col es 1 significa que no esta en la columa correcta 
            }

            membership  = ( token[0]== MEMBER_TYPE) ; //verificar : si membership se manejaba con 1 y 0 ;
            
            token = strtok(NULL, DELIMIT);
                
            addTrip( bikeRentalSystem, Id, endId, date, membership, endDate);
            
            }   
    }

 return empty ;
}

/*Cierra los archivos de lectura y limpia el sistema en caso de error*/
void error_read_File ( bikeRentalSystemADT bikeRentalSystem,  FILE *files[], int error, int count ){
    freeBikeRentalSystem(bikeRentalSystem);
    closeFilesCSV( files, count );
    exit(error) ;
}

/*Cierra los archivos de escritura y limpia el sistema ( en caso de error aborta con el debido error)*/
void close_write_files ( bikeRentalSystemADT bikeRentalSystem,  FILE *filesCSV[], htmlTable filesHTML[], int error, int count )
{
    freeBikeRentalSystem(bikeRentalSystem);
    closeFilesCSV( filesCSV, count);
    closeFilesHTML( filesHTML, count);
    if (error)
        exit(error) ;
}

/*Verifica si un str esta formado por numeros */
int isNum( const char* str){
    for ( int i =0; str[i]; i++){
        if ( !isdigit(str[i])){
            return 0;}
    }
    return 1;
}