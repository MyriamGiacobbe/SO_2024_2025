CC = gcc          # Compiler used is gcc
RM = rm -f        # Command to remove files

# -c to compile without linking

#all: Main        # 'all' depends on Main and produces the executable
all: run

run: Main          # 'run' calls the rule 'Main' and runs the linked program --- make all ARGS="records.csv 2 1"
	./bin/direttore	 

quickSort: mergeQuickSort/mergeQuickSort.c
	$(CC) -c mergeQuickSort/mergeQuickSort.c -o quickSort

compar: compare/compare.c
	$(CC) -c compare/compare.c -o compar 

Main: quickSort compar main_ex1.c # Compile main_ex1.c and mergeQuickSort.c into executable named Main
	$(CC) -o bin/main_ex1 main_ex1.c quickSort compar
                  

Test: quickSort compar testing/test_ex1.c testing/unity/unity.c  # Compile testing/test.c into executable named Test
	$(CC) -o bin/test_ex1 testing/test_ex1.c testing/unity/unity.c quickSort compar
                  

clean: 				# Remove the Main executable        # riga 30 : main e test
	$(RM) bin/*                                             
	$(RM) compar
	$(RM) quickSort
	$(RM) ../tmp/data/sorted.csv 

#make all ARGS="../tmp/data/records.csv ../tmp/data/sorted.csv 2 1"