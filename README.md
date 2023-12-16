#TRABAJO PRACTICO FINAL
#PROGRAMACION IMPERATIVA 

## Integrantes:

María Belén Petrikovich (belupetri) 
    mpetrikovich@itba.edu.ar 63689

Valentina Fernández Flores (ValentinaFernandezF)
    vfernandezflores@itba.edu.ar 64142

Manuel Cortés Teyssier (mcortest) 
    mcortesteyssier@itba.edu.ar 64159

## Creación de ejecutable

    - Al correr el comando 'make all' se crearan dos ejecutables en el directorio "bikeMON" y "bikeNYC".

    - Al correr el comando 'make MON' se creará el ejecutable "bikeMON".

    - Al correr el comando 'make NYC' se creará el ejecutable "bikeNYC".

## Ejecución del programa

    - Para ejecutar Montreal
    ```sh
    ./bikeMON bikesMON.csv stationsMON.csv primer_año (opcional) segundo_año (opcional)
    ```

    - Para ejecutar Nueva York
    ```sh
    ./bikeNYC bikesNYC.csv stationsNYC.csv primer_año (opcional) segundo_año (opcional)
    ```

## Eliminacion de archivos

    - Al correr el comando 'make clean' se eliminan todos los archivos.

    - Al correr el comando 'make cleanMON' se eliminan los archivos de Montreal.

    - Al correr el comando 'make cleanMYC' se eliminan los archivos de Nueva York

    - Al correr el comando 'make cleanQueries' se eliminan todos los archivos .csv y .html creados por el programa
