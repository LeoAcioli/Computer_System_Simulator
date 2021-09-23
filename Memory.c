#include "Memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


// Declaring functions...
void MemoryInit(char filename[]);
void runMemory(int wtpd, int rdpd);
static void loadMemoryTest();

//Memory space... -> 0-999: user space, 1000-1999: system space
int memory[MEMORY_SIZE];	

//MemoryInit:   Initialize Memory, Load data into memory              
void MemoryInit(char fileName[])
{
	// Load a program into memory
	int offset = 0;
	FILE *file = fopen(fileName, "r");
	if(file == NULL)			// Make sure the file open correctly.
	{
		printf("Error: File DNE\n");
		exit(1);
	}

	while(!feof(file))	// Load the whole file into memory, end until EOF appears
	{
		char buff[LINE_BUFFER_SIZE] = "";
		fgets(buff, sizeof(buff), file);         // Read line into buffer
		// Data line
		if(buff[0] >= '0' && buff[0] <= '9')
			memory[offset++] = atoi(buff);	   // Transfer char into instruction & store
		else if(buff[0] == '.')
			offset = atoi(&buff[1]);
	}

	// Test the content of memory
	//loadMemoryTest();
}

//runMemory:   Read/Write memory according to control signal         
//...int wtpd: write pipe description                      
//...int rdpd: read pipe description                       
void runMemory(int wtpd, int rdpd)
{
	char command;        // NOTE: 'w': write into memory; 'E': end process; 'r': read from memory
	int address, data;
	
	// read from pipe
	read(rdpd, &command, sizeof(command));
	
	while(command != 'E')
	{
		switch(command)
		{
			// Read instruction/data from memory
			case 'r':
				read(rdpd, &address, sizeof(address));                   // Read PC from pipe
				write(wtpd, &memory[address], sizeof(memory[address]));  // Write the content of address into pipe
				break;
			
			// Write data into memory
			case 'w':
				read(rdpd, &address, sizeof(address));   // Read add. from the pipe
				read(rdpd, &data, sizeof(address));		 // Read the data that processor want to save from pipe
				memory[address] = data;						
				break;
				
			default:
				printf("Command unexpected: %c\n", command);
				exit(-1);
				break;
		}
		read(rdpd, &command, sizeof(command));				// Read next command CPU
	}
}

//loadMemoryTest:   Test if memory load correctly => Output the value of mem[i]                            
void loadMemoryTest(){
	printf(" Testing Memory Now... \n");
	int offset = 0;
	printf("The size of the Memory is: %d\n", sizeof(memory)/sizeof(int));
	while(offset < sizeof(memory)/sizeof(int))
		printf("%d, %d\n", offset++, memory[offset]);
}