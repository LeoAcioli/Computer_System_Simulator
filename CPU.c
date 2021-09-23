#include "CPU.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>


// Declaring functions...
void CPUInit(char timer[]);
void runCPU(int wtpd, int rdpd);
static int readMemory(int addr, int wpd, int rpd);
static void writeMemory(int addr, int data, int wpd);
static int fetch(int wtpd, int rdpd);
static void exeInstruction(int wtpd, int rdpd);


//Remember...
//DEFAULT_TIME_SET = 1000
//USER_MODE = 0
//KERNEL_MODE = 1
//USER_ADDRESS =	0	-> Beginning of user program
//USER_STACK 	=	1000	-> Beginning of user stack
//INT_ADDRESS =	1500	-> Beginning of interrupt handler
//TIMER_ADDRESS = 1000	-> Beginning of timer interrupt handler
//SYS_STACK	= 2000	-> Beginning of system stack (count backwards)


int PC, SP, IR, AC, X, Y;
int COUNTER;				// Counts how many instructions the CPU has ran up to now
int counterSet;			// Set counter parameter
Boolean mode;				// USER_MODE or KERNEL_MODE


// CPUinit: Initial CPU, including set initial value of register, get timer interrupt parameter from command line       
void CPUInit(char timer[])
{
	PC = USER_ADDRESS;      // Set Program Counter to user memory
	SP = USER_STACK;        // Set Stack Pointer to user stack
	IR = 0;					        // No instruction present
	AC = 0;	
	X = 0;
	Y = 0;
	COUNTER = 0;            // Store the number of instructions that CPU runs so far
	mode = USER_MODE;		    // USER_MODE as default value because CPU runs from memory 0
  counterSet = atoi(timer); //getting string of timer and converting into an integer
 
	if(counterSet <= 0)
	{ 
		counterSet = DEFAULT_TIME_SET;	// Set counterSet to default value
		printf("Invalid timer parameter! Set to default value: %d!\n", DEFAULT_TIME_SET);
	}
}

//runCPU:   Simulate CPU, including fetch instruction,            
void runCPU(int wtpd, int rdpd)
{
	do
  {
		if(mode == USER_MODE)			// Timer works only if in user mode
			COUNTER++;
			
		IR = fetch(wtpd, rdpd);         // Fetch instruction to Instruction Register
		exeInstruction(wtpd, rdpd);		// Execute instruction
		
		// Check timer interrupt flag
		if(mode == USER_MODE && COUNTER == counterSet)
		{
			COUNTER = 0;				// Clear timer
			mode = KERNEL_MODE;			// Set mode to kernel mode to access interrupt handler
				
			int tmp = SP;				// Record user stack pointer
			SP = SYS_STACK;				// Stack Pointer switch to system stack
			SP--;                       
			writeMemory(SP, tmp, wtpd); // Save user SP into system stack
			SP--;
			writeMemory(SP, PC, wtpd);  // Save current PC into system stack
			PC = TIMER_ADDRESS;			// Set PC to timer interrupt handler		
		}
	}while(IR != END);
	
	write(wtpd, "E", sizeof(char));
}

//readMemory:   Read data from memory                                 
int readMemory(int addr, int wpd, int rpd)
{
	// Protection of Memory
	if(mode == USER_MODE && addr >= TIMER_ADDRESS)
	{
		printf("Memory violation: accessing system address %d in user mode\n", addr);
		write(wpd, "E", sizeof(char));    // Error occur, exit processes
		exit(-1);
	}
	
	int tmp;
	write(wpd, "r", sizeof(char));    // Control signal
	write(wpd, &addr, sizeof(addr));  // Address
	read(rpd, &tmp, sizeof(tmp));     // Returned data
	
	return tmp;
}


//writeMemory:   Write data to memory                                  
void writeMemory(int addr, int data, int wpd)
{
	// Memory Protection
	if(mode == USER_MODE && addr >= TIMER_ADDRESS)
	{
		printf("Memory violation: accessing system address %d in user mode\n", addr);
		write(wpd, "E", sizeof(char));    // Error occur, exit processes
		exit(-1);
	}
	
	write(wpd, "w", sizeof(char));    // Control signal 
	write(wpd, &addr, sizeof(addr));  // Address 
	write(wpd, &data, sizeof(data));  // Data
}


//fetch:   Fetch data from memory, data: instruction or data     
int fetch(int wtpd, int rdpd)
{
	return readMemory(PC++, wtpd, rdpd);
}


//exeInstruction:   Execute the instruction                               
void exeInstruction(int wtpd, int rdpd)
{
	switch(IR){
		// Load the value into the AC
		case LOAD_VALUE:
			AC = fetch(wtpd, rdpd);
			break;
		
		// Load the value at the address into the AC 
		case LOAD_ADDR:
		{
			int addr = fetch(wtpd, rdpd);
			AC = readMemory(addr, wtpd, rdpd);
			break;
		}
		
		// Load the value from the address found in the given address into AC 
		case LOAD_IND_ADDR:
		{
			int addr = fetch(wtpd, rdpd);
			addr = readMemory(addr, wtpd, rdpd);
			AC = readMemory(addr, wtpd, rdpd);
			break;
		}
			
		//Load the value at (address + X) into the AC 
		case LOAD_IDX_X_ADDR:
		{
			// Calculate address
			int addr = fetch(wtpd, rdpd) + X;				// Base address + offset
			AC = readMemory(addr, wtpd, rdpd);
			break;
		}
		
		// Load the value at (address + Y) into the AC 
		//Similar with LOAD_IDX_X_ADDR 
		case LOAD_IDX_Y_ADDR:
		{
			int addr = fetch(wtpd, rdpd) + Y;
			AC = readMemory(addr, wtpd, rdpd);
			break;
		}
		
		//Load from (SP + X) into the AC 
		case LOAD_SP_X:
		{
			int addr = SP + X;
			AC = readMemory(addr, wtpd, rdpd);
			break;
		}
		
		// Store the value in the AC into the address 
		case STORE_ADDR:
		{
			int addr = fetch(wtpd, rdpd);
			writeMemory(addr, AC, wtpd);
			break;
		}
		
		//Gets a random int from 1 to 100 into the AC 
		case GET:
			srand(time(NULL));      // Generate the seed
			AC = rand() % 100 + 1;  // Create a random number [1, 100]
			break;
   
		//Put AC to the screen
		// If port = 1, writes AC as an int to the screen 
		// If port = 2, writes AC as a char to the screen 
		case PUT_PORT:
		{
			Boolean flag = fetch(wtpd, rdpd);
			if(flag == 1)
				printf("%d", AC);
			else if(flag == 2)
				printf("%c", AC);
			else
				printf("Parameter error: %d\n", flag);
			break;
		}
			
		// Add the value in X to the AC
		case ADD_X:
			AC += X;
			break;
			
		//Add the value in Y to the AC 
		case ADD_Y:
			AC += Y;
			break;
		
		// Subtract the value in X from the AC 
		case SUB_X:
			AC -= X;
			break;
	
		// Subtract the value in Y from the AC
		case SUB_Y:
			AC -= Y;
			break;
		
		// Copy the value in the AC to X 
		case COPY_TO_X:
			X = AC;
			break;
		
		// Copy the value in X to the AC
		case COPY_FROM_X:
			AC = X;
			break;
		
		// Copy the value in the AC to Y 
		case COPY_TO_Y:
			Y = AC;
			break;
		
		//Copy the value in Y to the AC 
		case COPY_FROM_Y:
			AC = Y;
			break;
		
		//Copy the value in AC to the SP 
		case COPY_TO_SP:
			SP = AC;
			break;
		
		// Copy the value in SP to the AC 
		case COPY_FROM_SP:
			AC = SP;
			break;
		
		// Jump to the address 
		case JUMP_ADDR:
			PC = fetch(wtpd, rdpd);
			break;
			
		//Jump to the address only if the value in the AC is 0 
		case JUMP_IF_EQUAL_ADDR:
		{
			int addr = fetch(wtpd, rdpd);
			if(AC == 0)
				PC = addr;
			break;
		}
		
		//Jump to the address only if the value in the AC is not 0
		case JUMP_IF_NOT_EQUAL_ADDR:
		{
			int addr = fetch(wtpd, rdpd);
			if(AC != 0)
				PC = addr;
			break;
		}
		
		//Function call: push return address onto stack, jump to the address 
		case CALL_ADDR:
		{
			int addr = fetch(wtpd, rdpd);   // Finish the instruction fetch
			SP--;							              // Stack is growing reversely
			writeMemory(SP, PC, wtpd);      // Push PC onto stack
			PC = addr;						          // Jump to address
			break;
		}
		
		// Pop return address from the stack, jump to the address 
		case RET:
			PC = readMemory(SP, wtpd, rdpd); // Read current content of the stack
			SP++;			                 // Stack shrink
			break;
		
		//Increment X 
		case INC_X:
			X++;
			break;
		
		//Decrement X
		case DEC_X:
			X--;
			break;
		
   // Pop from stack into AC 
		case POP:
			AC = readMemory(SP, wtpd, rdpd);
			SP++;
			break;
      
		// Push AC onto stack 
		case PUSH:
			SP--;
			writeMemory(SP, AC, wtpd);
			break;
	
		
		//Perform system call 
		case INT:
			if(mode == USER_MODE)
			{
				mode = KERNEL_MODE;			// Set mode to kernel mode to access interrup handler
				int tmp = SP;				// Record user stack pointer
				SP = SYS_STACK;				// Stack Pointer switch to system stack
				
				SP--;
				writeMemory(SP, tmp, wtpd); // Save user SP, PC into system stack
				SP--;
				writeMemory(SP, PC, wtpd);
				PC = INT_ADDRESS;			// Set PC to timer interrupt handler
				break;				
			}
		
		// Return from system call
		case I_RET:
			PC = readMemory(SP, wtpd, rdpd); // Pop PC, SP
			SP++;
			SP = readMemory(SP, wtpd, rdpd);
			mode = USER_MODE;                // Change mode
			break;
			
		// End execution
		case END:
			write(wtpd, "E", sizeof(char));  // Tell memory program finished so that memory can remove the process
			break;
			
		default:
			printf("Invalid instruction\n");
			write(wtpd, "E", sizeof(char));    // Error occur, exit processes
			exit(-1);
			break;
	}
}