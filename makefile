COMPILER = gcc
FLAGS= -Wall -pedantic -std=c99 -fsanitize=address -g
OUTPUT_FILE_MON = bikeSharingMON
OUTPUT_FILE_NYC = bikeSharingNYC

DEPENDENCIES = bikesADT.c main.c htmlTable.c


MON:
	$(COMPILER) -o$(OUTPUT_FILE_MON) $(DEPENDENCIES) $(FLAGS) -DMON
NYC:
	$(COMPILER) -o $(OUTPUT_FILE_NYC) $(DEPENDENCIES) $(FLAGS) -DNYC
all: MON NYC

clean:
	rm -rf $(OUTPUT_FILE_MON) $(OUTPUT_FILE_NYC) *.o

cleanMON: 
	rm -rf $(OUTPUT_FILE_MON) *.o
cleanNYC:
	rm -rf $(OUTPUT_FILE_NYC) *.o
