#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

typedef struct bikeRentalSystemCDT * bikeRentalSystemADT;

// crea nuevo conjunto
bikeRentalSystemADT newBikeRentalSystem ();

// apunta al primer elemento
void toBegin (bikeRentalSystemADT bikeRentalSystemAdt);

// retorna 1 si hay elemento siguiente
int hasNext (bikeRentalSystemADT bikeRentalSystemAdt);

// apunta al siguiente elemento
TList next (bikeRentalSystemADT bikeRentalSystemAdt);

// libera el conjunto
void freebikeRentalSystem ( bikeRentalSystemADT bikeRentalSystemAdt );

