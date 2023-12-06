#include <stdio.h>
#include "htmlTable.h"

/**
 * Versión simple de test, no se controlan errores
  */

int
main(void) {

    htmlTable table = newTable("cTableTest.html", 2, "Column 1", "Column 2");

    for(int i = 0; i < 10; i++) {
        addHTMLRow(table, "Value 1", "Value 2");
    }
    closeHTMLTable(table);

    return 0;
}