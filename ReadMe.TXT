September 21st, 2021 -> Leonardo Acioli Arruda Sampaio


FILES**************************************************************************************************************************************************

CPU.h: Header file for CPU.cpp

Memory.h: Header file for Memory.cpp

CPU.cpp: Made of instructions/executions. It communicates with Memory by sending an instruction to be fetched, then executing the code as expected.

Memory.cpp: Initializes itself. Sends CPU instructions/operations.

main.cpp: C++ program that creates two pipes before forking two processes (child is memory, parent is CPU). Memory and CPU communicate by fetching and 
	  executing instructions until program is done. In main, only one method of Memory.cpp and CPU.cpp is called (the inits methods of each).
	  In main, the arguments from the command line are passed as well. Timer is passed to the CPU as argv[2], and filename as argv[1].

sample1.txt: general set up of how to work the program

sample2.txt: Tests stack

sample3.txt: Tests interrupt handling

sample4.txt: Continues to test stack

sample5.txt: Program that prints "All that glitters is not gold", but searches from a table the characters 
	     in ASCII, as well as print the character and its corresponding ASCII code by it. Similar to the logic
	     in sample1.txt

****************************************************************************************************************************************************
RUNNING INSTRUCTIONS:

-> Have all files in the same directory

-> Type "gcc Memory.c CPU.c main.c" on the command line

-> Running code above will spit out an a.out executable

->To run the executable, type on command line:

	./a.out <<filename>> <<timer>>

-> example: a.out sample2.txt 1000

If missing arguments or too many arguments are input, program will tell you and will abort itself, giving you a change to run again.