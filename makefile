COMPILER = gcc
FLAGS= -Wall -pedantic -std=c99 -fsanitize=address -g
OUTPUT_FILE_MON = bikeMON
OUTPUT_FILE_NYC = bikeNYC
DEPENDENCIES = bikesADT.c main.c htmlTable.c


MON:
	$(COMPILER) -o$(OUTPUT_FILE_MON) $(DEPENDENCIES) $(FLAGS) -DMON
NYC:
	$(COMPILER) -o $(OUTPUT_FILE_NYC) $(DEPENDENCIES) $(FLAGS) -DNYC
all: MON NYC

clean: cleanMON cleanNYC cleanQueries

cleanMON: 
	rm -rf $(OUTPUT_FILE_MON) *.o
cleanNYC:
	rm -rf $(OUTPUT_FILE_NYC) *.o

cleanQueries:
	rm -rf ./query*
