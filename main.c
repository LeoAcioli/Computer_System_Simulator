#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


int main(int argc, char *argv[])
{
	// Creating pipe...
	int rdpd[2];				// Read pipe, rdpd[0]: read - CPU, rdpd[1]: write - Memory
	int wtpd[2];				// Write pipe, wtpd[0]: read - Memory, wtpd[1]: write - CPU
	if(pipe(rdpd) == -1 || pipe(wtpd) == -1)  // Create pipe
	{
		printf("pipe() failed!\n");
		exit(1);
	} 
 
  if (argc != 3){ //If user didn't type both arguments, abort and ask for correct number.
   printf("Error! Missing or too many arguments. Programing aborting. Run again with correct number of arguments.\n");
   exit(1);
 }
 
   FILE *file = fopen(argv[1], "r"); //File is supposed to be second argument. Command line argv[0]->program,argv[1]=FileName.argv[2]=timer
   if(file == NULL)			// Make sure the file open correctly. If not existent, program ends.
	{
		printf("Error! File does not exist. Program aborting. Try running again with an existing file.\n");
		return(0);
	}
	
	// Create processes: CPU, Memory
	int pid = fork();
	switch(pid)
	{
		case -1:
			// If fork fails...
			printf("The fork failed!");
			exit(-1);	

		case 0:	
			// pid == 0 is child -> child is memory
			MemoryInit(argv[1]);
			runMemory(rdpd[1], wtpd[0]);	// Param:(write pd, read pd)
			exit(0);
			
		default:
			// pid > 0 is Parent -> Parent is CPU
			CPUInit(argv[2]);
			runCPU(wtpd[1], rdpd[0]);		// Param:(write pd, read pd)
			waitpid(pid, NULL, 0);			// Waiting for memory process exit
			exit(0);
	}
}