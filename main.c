#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>

#define MAXSTACKSIZE 64
#define BYTES_IN_WORD 4
#define _CRT_SECURE_NO_WARNINGS

/********************** Memory Bank ***************************/


int Memory[1024];
int stack[MAXSTACKSIZE];
int stackTop;

/******************** Registers and flags********************/

typedef uint32_t registerType; //representing registers as uint32_t

registerType regFile[15] = { 0, 0, 2, 2, 3, 4, 5, 16, 22, 8, 9, 10, 11, 12, 13 }; // reg[8-14] are used as temporary registers
registerType *programCounter = Memory;
registerType MAR, MDR;
registerType FLAGS[9];

/*********************Six Sets for Binary Search***************/

int set1[6] = { 1, 2, 3, 4, 5, 6 };	//Natural numbers
int set2[6] = { 0, 2, 4, 6, 8, 10 };	//Even numbers
int set3[6] = { 1, 3, 5, 7, 9, 11 };	//Odd numbers
int set4[6] = { 10, 20, 30, 40, 50 };
int set5[6] = { 4, 7, 11, 15, 17 };
int set6[6] = { 15, 20, 25, 30, 35 };

/********************Label Declaration**********************/

typedef struct{
	char label_name[15];
	int label_index;
}label;

//Forward declaration
int pushStack(int);
int popStack(int);
void display_stack(void);
void initializeMemory();
void display();
void display_flags();
void instructionDecode(char *);
void instructionExecution(char *, int, int);
void memoryFetch(char *, int, uint32_t *);
void BEN(char*, int);
void LEN(char*, int);


void initializeMemory()
{
	int i;
	for (i = 0; i < 1024; i++)
	{
		Memory[i] = i * 2;
	}
}


int check_Memory(int mem_index)
{

	if ((mem_index >= 0 && mem_index <= 100))
	{
		printf(" Reserved memory \n");
		system("pause");
		exit(0);
	}

	else
		return 1;
}

int check_Registers(int reg_index)
{

	if (!(reg_index > 0 && reg_index <= 7))
	{
		printf("Register does not exist\n");
		system("pause");
		exit(0);
	}

	else
		return 1;
}

pushStack(int x)
{
	if (stackTop > MAXSTACKSIZE)
	{
		printf("The stack is full\n");
		return 0;
	}
	else
	{
		printf("The number pushed onto stack is %d\n", regFile[x]);
		stackTop++;
		stack[stackTop] = regFile[x];
		display_stack();
		return 1;
	}
}

popStack(int x)
{
	if (stackTop < 0)
	{
		printf("The stack is empty\n");

	}
	else
	{
		printf("The size of stack is %u\n", MAXSTACKSIZE);
		printf("The stack_top is at %u\n", stackTop);
		printf("The popped element is again stored in the register given\n");
		regFile[x] = stack[stackTop];
		printf("The popped element of stack is %d\n", regFile[x]);
		stackTop--;
		display_stack();
	}
}

void display_stack(void)
{

	if (stackTop < 0)
	{
		printf("The stack is empty\n");
		return;
	}
	else
	{
		printf("The elements in stack are \n");
		int i;
		for (i = stackTop; i > 0; i--)
		{
			printf("%d\n", stack[i]);

		}
		printf("\n");
	}
}

//Definitions of the operations in ALU

ADD(int x, int y, int z)
{

	uint32_t a = regFile[y];
	uint32_t b = regFile[z];
	uint32_t carry;

	while (b != 0) //iterated until no carry
	{
		carry = a & b;

		a = a ^ b; // this is the sum 

		b = carry << 1;
		//enabling the flags 
		if (carry != 0 && carry != 2147483648) //to set carry flag,
		{
			FLAGS[2] = 1;
		}
	}
	regFile[x] = a;
	if (a == 0)
		FLAGS[0] = 1;  //to set zero flag
	if (a >= 2147483648)
		FLAGS[1] = 1; //to set overflow flag, if the sum results in an overflow

	//BEN((char*)&regFile[x], sizeof(regFile[x]));
	//LEN((char*)&regFile[x], sizeof(regFile[x]));


}

SUB(int x, int y, int z)
{

	regFile[9] = ~regFile[z];
	regFile[8] = 1;
	ADD(x, 9, 8);
	regFile[10] = regFile[x];
	return ADD(x, y, 10);


}

MUL(int x, int y, int z)
{

	int a = regFile[y];
	int b = regFile[z];
	regFile[8] = a;
	regFile[10] = 0;

	while (b != 0)
	{
		if (b & 1)                	// checking wheather b is odd or not
		{
			ADD(x, 10, 8);     		// Add a to result if b is odd .
			regFile[10] = regFile[x];
		}
		regFile[8] <<= 1;                    // a is left shifted by 1 which multiplies a by 2

		b >>= 1;

	}

	regFile[x] = regFile[10];   //storing the result in the destination register
	if (regFile[10] == 0)
		FLAGS[0] = 1;

}


int Add_ALU(int a, int b)
{
	while (b != 0) //iterated until no carry
	{
		uint32_t carry = a & b;


		a = a ^ b; // this is the sum 

		b = carry << 1;
	}
	return a;
}

int Sub_ALU(int a, int b)
{
	return Add_ALU(a, Add_ALU(~b, 1));
}


int Mul_ALU(int a, int b)

{
	int Result = 0;
	int count = 0;
	if (a < 0)
	{
		a = Sub_ALU(0, a);
		count++;
	}

	if (b < 0)
	{
		b = Sub_ALU(0, b);
		count++;
	}
	while (b != 0)
	{
		if (b & 01)
		{
			Result = Add_ALU(Result, a); // Add op1 to result if op2 is odd .
		}
		a <<= 1; // Left shifting the value contained in op1 by 1
		b >>= 1; // Right shifting the value contained in op2 by 1.
	}



	if (count == 1)
	{
		return Sub_ALU(0, Result);
	}
	else
	{
		return Result;
	}
}

int Division_ALU(int a, int b)
{

	int Quotient = 1, Remainder;

	//convert to positive
	int dividend = (a < 0) ? -a : a;
	int divisor = (b < 0) ? -b : b;

	if (b == 0)
	{
		FLAGS[3] = 1; //set divide by zero flag
		printf("The divisor is zero, divide by zero flag is set\n");
		return 0;
	}
	if (dividend == 0)
		FLAGS[0] = 1;
	if (divisor == dividend)
	{
		Remainder = 0;
		return Quotient;
	}
	else if (dividend < divisor)
	{
		Remainder = dividend;
		return 0;
	}
	while (divisor << 1 <= dividend)
	{

		divisor = divisor << 1;
		Quotient = Quotient << 1;

	}

	Quotient = Add_ALU(Quotient, Division_ALU(Sub_ALU(dividend, divisor), b)); //quotient = quotient + division (dividend-divisor,divisor)
	return Quotient;



}

DIV(int x, int y, int z)
{
	if ((regFile[y] < 0 && regFile[z] > 0) || (regFile[y] > 0 && regFile[z] < 0))
		regFile[x] = Division_ALU(regFile[y], regFile[z]);
	else
		regFile[x] = Sub_ALU(0, Division_ALU(regFile[y], regFile[z]));

}

MOD(int x, int y, int z)
{
	int Remainder;

	if (regFile[z] == 0)
	{
		FLAGS[0] = 1;
		printf("The divisor is zero, zero flag is set\n");
		return 0;
	}

	if (regFile[y] == regFile[z])
	{
		Remainder = 0;
	}

	if (regFile[y] < regFile[z])
	{
		Remainder = regFile[y]; //this is done automatically
	}


	regFile[12] = Division_ALU(regFile[y], regFile[z]);
	MUL(x, z, 12);
	regFile[13] = regFile[x];
	Remainder = Sub_ALU(regFile[y], regFile[13]);

	regFile[x] = Remainder;

	if (Remainder == 0)
		FLAGS[0] = 1;

}

int JLT(int operand1, int operand2, int operand3)
{
	int x = regFile[operand1];
	int y = regFile[operand2];
	int index = operand3;
	int a = Sub_ALU(x, y);
	if (a<0)
		programCounter = &(Memory[index]);
	else
		printf("Jump cannot happen, program counter will remain same\n");
	return;
}

int JGT(int operand1, int operand2, int operand3)
{
	int x = regFile[operand1];
	int y = regFile[operand2];
	int index = operand3;
	int a = Sub_ALU(x, y);
	if (a>0)
		programCounter = &(Memory[index]);
	else
		printf("Jump cannot happen, program counter will remain same\n");
	return;
}

int JNE(int operand1, int operand2, int operand3)
{
	int x = regFile[operand1];
	int y = regFile[operand2];
	int index = operand3;
	int a = Sub_ALU(x, y);
	if (a != 0)
		programCounter = &(Memory[index]);
	else
		printf("Jump cannot happen, program counter will remain same\n");
	return;
}

int JE(int operand1, int operand2, int operand3)
{


	int x = regFile[operand1];
	int y = regFile[operand2];
	int index = operand3;
	int a = Sub_ALU(x, y);
	if (a == 0){
		programCounter = &(Memory[index]);
		return 1;
	}
	else
	{
		//printf("Jump cannot happen, program counter will remain same\n");
		//else if (a == 0 && flag == 1)
		return 0;
	}


}

int JNZ(int operand1, int operand2)
{
	int x = regFile[operand1];
	int index = operand2;

	int a = Sub_ALU(x, 0);
	if (a == 0)
	{
		FLAGS[0] = 1; //set zero flag
	}
	if (FLAGS[0] != 1)
		programCounter = &(Memory[index]);
	else
		printf("Jump cannot happen, program counter will remain same\n");
	return;
}

int JZ(int operand1, int operand2)
{
	int x = regFile[operand1];
	int index = operand2;
	int a = Sub_ALU(x, 0);
	if (a == 0)
	{
		FLAGS[0] = 1; //set zero flag
	}
	if (FLAGS[0] == 1)
		programCounter = &(Memory[index]);
	else
		printf("Jump cannot happen, program counter will remain same\n");
	return;
}

int JMP(int operand1)
{
	int index = operand1;
	programCounter = &(Memory[index]);
	return;
}



int blt(int reg1, int reg2)
{
	int x = regFile[reg1];
	int y = regFile[reg2];

	if (x < y){
		return 1;
	}
	else{
		return 0;
	}
}


int bgt(int reg1, int reg2){
	int x = regFile[reg1];
	int y = regFile[reg2];

	if (x > y){
		return 1;
	}
	else
		return 0;
	
}

int LEA(int operand1, int operand2)
{

	int x = &(Memory[operand2]);
	//printf("value of x = 02x%d", x);
	regFile[operand1] = x;
	return;
}

int LEA_BYTES(int operand1, int operand2, int operand3)
{
	int index = operand2;
	int bytes = operand3;
	int x = &(Memory[index + bytes]);
	regFile[operand1] = x;
	return;
}

int LEA_WORD(int operand1, int operand2, int operand3)
{
	int index = operand2;
	int word = operand3;
	int words = Mul_ALU(word, BYTES_IN_WORD);
	int x = &(Memory[index + words]);
	regFile[operand1] = x;
	return;
}

int LI(int operand1, int value)
{
	regFile[operand1] = value;
}


//do while loop implementation using labels as in assembly language
//used add, sub and load immediate instructions from our instruction set 
int dowhile()
{
	LI(7, 7);
	LI(3, 0);
	int x;

Loop:
	LI(2, 1);
	regFile[3] = Add_ALU(regFile[3], regFile[2]);
	x = Sub_ALU(regFile[7], regFile[3]);
	if (x == 0)
		FLAGS[0] = 1;
	if (FLAGS[0] == 1)
		return;
	//printf("Exiting loop\n");
	goto Loop;


}

//for loop implementation using labels as in assembly language
//used add, sub and load immediate instructions from our instruction set 
int for_loop()
{
	LI(7, 7);
	LI(3, 0);
	int x;
Loop:
	x = Sub_ALU(regFile[7], regFile[3]);
	if (x == 0)
		FLAGS[0] = 1;
	if (FLAGS[0] == 1)
		//if(regFile[7] == regFile[3])
		goto end;
	LI(2, 1);
	regFile[3] = Add_ALU(regFile[3], regFile[2]);
	goto Loop;

end:
	return;

}


//while do loop implementation
int whiledo()
{
	LI(7, 8);
	LI(3, 0);
	LI(2, 1); //initializing the register before entering the loop
	int z;
Loop:
	z = Sub_ALU(regFile[7], regFile[3]);
	if (z == 0)
		FLAGS[0] = 1;
	if (FLAGS[0] == 1)
		//if(regFile[7] == regFile[3])
		goto end;

	regFile[3] = Add_ALU(regFile[3], regFile[2]);
	goto Loop;

end:
	return;

}


//Binary Search
int bSearch(int set[], int lowval, int highval, int item)
{
	int mid;
	mid = (lowval + highval) / 2;

	if (lowval>highval)
	{
		printf("\n key not found");
		return -1;
	}

	if (set[mid] == item)
	{
		printf("\nSearch item found in the list\n");
		return mid;	//index
	}

	else if (item < set[mid])
	{
		bSearch(set, lowval, mid - 1, item);
	}

	else
	{
		bSearch(set, mid + 1, highval, item);
	}
}



int jmpTarget(char *jT, label l[]) //to find jump target
{
	int i = 0;
	for (i = 0; i < 10; i++)
	{
		//printf("Label:%s\n", l[i].labelName);
		if (strcmp(l[i].label_name, "") != 0){
			if (!(strcmp(l[i].label_name, jT))){
				return l[i].label_index;
			}
		}

	}
	return -1;	//jump target not found
}

void multipleInstructionDecode(char **inst, int instIndex)
{
	label Label[50];
	int i = 0, j = 0;

	for (i = 0; i<instIndex; i++)
	{
		if (strstr(inst[i], ":") != NULL)
		{
			//printf("Instruction: %s", inst[i]);
			char lbel[30];
			char *lname;
			strcpy(lbel, inst[i]);

			lname = strtok(lbel, " :\t\n");
			strncpy(Label[j].label_name, lname, sizeof(Label[j].label_name));
			Label[j].label_index = i;
			j++;
		}
	}


	for (i = 0; i<instIndex; i++)
	{
		if (strstr(inst[i], ":") != NULL)
		{
			if (i<instIndex)
			{
				//continue;
			}
		}

		if ((strstr(inst[i], "JE") != NULL) || (strstr(inst[i], "je") != NULL))
		{
			//printf("Instruction: %s", inst[i]);
			char instCopy[25];
			char *jumpTarget;
			char *op1, *op2;
			int oprnd1, oprnd2;
			strcpy(instCopy, inst[i]);
			jumpTarget = strtok(instCopy, " ,,\t\n");
			jumpTarget = strtok(NULL, " ,,\t\n");
			op1 = strtok(NULL, "$ ,,\t\n");
			op2 = strtok(NULL, "$ ,,\t\n");
			oprnd1 = atoi(op1);
			oprnd2 = atoi(op2);

			int index = jmpTarget(jumpTarget, Label);
			//printf("Jump index: %d\n", jIdx);
			if (JE(oprnd1, oprnd2,0))
			{
				i = index;
				printf("\n After jumping the instruction is : %s ", inst[i]);
				
			}
			else {
				continue;
			}

		}


		if ((strstr(inst[i], "BGT") != NULL) || (strstr(inst[i], "bgt") != NULL))
		{
			printf("Instruction1: %s", inst[i]);
			char instCopy[25];
			char *jumpTarget;
			char *op1, *op2;
			int oprnd1, oprnd2;
			strcpy(instCopy, inst[i]);
			jumpTarget = strtok(instCopy, " ,,\t\n");
			jumpTarget = strtok(NULL, " ,,\t\n");
			op1 = strtok(NULL, "$ ,,\t\n");
			op2 = strtok(NULL, "$ ,,\t\n");
			oprnd1 = atoi(op1);
			oprnd2 = atoi(op2);

			int index = jmpTarget(jumpTarget, Label);
			
			if (bgt(oprnd1, oprnd2)){
				i = index;
				
			}
			else{
				continue;
			}
		}

		if ((strstr(inst[i], "BLT") != NULL) || (strstr(inst[i], "blt") != NULL))
		{
			printf("Instruction1: %s", inst[i]);
			char instCopy[25];
			char *jumpTarget;
			char *op1, *op2;
			int oprnd1, oprnd2;
			strcpy(instCopy, inst[i]);
			jumpTarget = strtok(instCopy, " ,,\t\n");
			jumpTarget = strtok(NULL, " ,,\t\n");
			op1 = strtok(NULL, "$,,\t\n");
			op2 = strtok(NULL, "$,,\t\n");
			oprnd1 = atoi(op1);
			oprnd2 = atoi(op2);

			int index = jmpTarget(jumpTarget, Label);
			
			if (blt(oprnd1, oprnd2)){
				i = index;
				
			}
			else{
				continue;
			}
		}

		printf("\nExecuting the instruction: %s\n", inst[i]);

		if (inst[i][0] != '\0'){
			
			if (strstr(inst[i], ":") != NULL){
				char *a = strtok(inst[i], ":\t ");
				
				char *b = strtok(NULL, "\n");
				b = ++b;
				instructionDecode(++b);
			}
			else
				instructionDecode(inst[i]);
		}
	}
	
}


// ALU operations include ADD/SUB/MUL/DIV/MOD
ALU(int operand1, int operand2, int operand3, int index)
{
	switch (index)
	{
	case 1:
		ADD(operand1, operand2, operand3);
		break;
	case 2:
		SUB(operand1, operand2, operand3);
		break;
	case 3:
		MUL(operand1, operand2, operand3);
		break;
	case 4:
		DIV(operand1, operand2, operand3);
		break;
	case 5:
		MOD(operand1, operand2, operand3);
		break;


	default: printf("ALU can perform only ADD/SUB/MUL/DIV/MOD !! please enter those operations only");
	}

}



void BEN(char *x, int num)
{
	int i;
	//unsigned char *byte = (unsigned char*) &num;
	printf("\nBEN layout: 0x");
	for (i = num - 1; i >= 0; i--)
	{
		printf("%.2x", x[i]);
	}
	printf("\n");
}


void LEN(char *x, int num)
{
	int i;
	//unsigned char *byte = (unsigned char*) &num;
	printf("\nLEN layout: 0x");
	for (i = 0; i < num; i++)
	{
		printf("%.2x", x[i]);
	}
	printf("\n");
}

void instructionDecode(char* instruction) //decoding the instructions using string token
{
	char *instType, *op1, *op2, *op3;
	int operand1, operand2, operand3;

	instType = strtok(instruction, " , "); //using " ," as delimiters




	if (!strcmp(instType, "add") || !strcmp(instType, "ADD"))
	{


		op1 = strtok(NULL, " ,$ ");	//using " ,$" as delimiters
		op2 = strtok(NULL, " ,$");
		op3 = strtok(NULL, " ,$");

		operand1 = atoi(op1);
		operand2 = atoi(op2);
		operand3 = atoi(op3);
		if (check_Registers(operand1) && check_Registers(operand2) && check_Registers(operand3))
		{

			ALU(operand1, operand2, operand3, 1);
			BEN((char*)&regFile[operand1], sizeof(regFile[operand1]));
			LEN((char*)&regFile[operand1], sizeof(regFile[operand1]));

		}
	}

	else if (!strcmp(instType, "sub") || !strcmp(instType, "SUB"))
	{

		op1 = strtok(NULL, " ,$ ");	//using " ,$" as delimiters
		op2 = strtok(NULL, " ,$");
		op3 = strtok(NULL, " ,$");

		operand1 = atoi(op1);
		operand2 = atoi(op2);
		operand3 = atoi(op3);

		if (check_Registers(operand1) && check_Registers(operand2) && check_Registers(operand3))
		{
			ALU(operand1, operand2, operand3, 2);
			BEN((char*)&regFile[operand1], sizeof(regFile[operand1]));
			LEN((char*)&regFile[operand1], sizeof(regFile[operand1]));
		}
	}


	else if (!strcmp(instType, "mul") || !strcmp(instType, "MUL"))
	{
		op1 = strtok(NULL, " ,$ ");	//using " ,$" as delimiters
		op2 = strtok(NULL, " ,$");
		op3 = strtok(NULL, " ,$");

		operand1 = atoi(op1);
		operand2 = atoi(op2);
		operand3 = atoi(op3);

		if (check_Registers(operand1) && check_Registers(operand2) && check_Registers(operand3))
		{
			ALU(operand1, operand2, operand3, 3);
			BEN((char*)&regFile[operand1], sizeof(regFile[operand1]));
			LEN((char*)&regFile[operand1], sizeof(regFile[operand1]));
		}
	}

	else if (!strcmp(instType, "div") || !strcmp(instType, "DIV"))
	{
		op1 = strtok(NULL, " ,$ ");	//using " ,$" as delimiters
		op2 = strtok(NULL, " ,$");
		op3 = strtok(NULL, " ,$");

		operand1 = atoi(op1);
		operand2 = atoi(op2);
		operand3 = atoi(op3);

		if (check_Registers(operand1) && check_Registers(operand2) && check_Registers(operand3))

		{
			ALU(operand1, operand2, operand3, 4);
			BEN((char*)&regFile[operand1], sizeof(regFile[operand1]));
			LEN((char*)&regFile[operand1], sizeof(regFile[operand1]));
		}
	}

	else if (!strcmp(instType, "mod") || !strcmp(instType, "MOD"))
	{

		op1 = strtok(NULL, " ,$ ");	//using " ,$" as delimiters
		op2 = strtok(NULL, " ,$");
		op3 = strtok(NULL, " ,$");

		operand1 = atoi(op1);
		operand2 = atoi(op2);
		operand3 = atoi(op3);

		if (check_Registers(operand1) && check_Registers(operand2) && check_Registers(operand3))
		{
			ALU(operand1, operand2, operand3, 5);
			BEN((char*)&regFile[operand1], sizeof(regFile[operand1]));
			LEN((char*)&regFile[operand1], sizeof(regFile[operand1]));
		}
	}

	else if (!strcmp(instType, "push") || !strcmp(instType, "PUSH"))
	{
		op1 = strtok(NULL, " ,$");
		operand1 = atoi(op1);
		pushStack(operand1);
		BEN((char*)&regFile[operand1], sizeof(regFile[operand1]));
		LEN((char*)&regFile[operand1], sizeof(regFile[operand1]));

	}

	else if ((!strcmp(instType, "pop")) || (!strcmp(instType, "POP")))
	{
		op1 = strtok(NULL, " ,$");
		operand1 = atoi(op1);
		popStack(operand1);
		BEN((char*)&regFile[operand1], sizeof(regFile[operand1]));
		LEN((char*)&regFile[operand1], sizeof(regFile[operand1]));

	}

	else if (!strcmp(instType, "lw") || !strcmp(instType, "sw"))
	{


		op1 = strtok(NULL, " ,$");	//using " ,$" as delimiters
		op2 = strtok(NULL, " ,$\n");

		operand1 = atoi(op1);
		operand2 = atoi(op2);
		if (check_Registers(operand1) && check_Memory(operand2))
		{
			instructionExecution(instruction, operand1, operand2);
			if (instType == " lw")
			{
				BEN((char*)&regFile[operand1], sizeof(regFile[operand1]));
				LEN((char*)&regFile[operand1], sizeof(regFile[operand1]));
			}
			else
			{
				BEN((char*)&Memory[operand2], sizeof(Memory[operand2]));
				LEN((char*)&Memory[operand2], sizeof(Memory[operand2]));
			}
		}

	}

	else if ((!strcmp(instType, "JLT")) || (!strcmp(instType, "jlt")))
	{
		op1 = strtok(NULL, " ,$ ");	//using " ,$" as delimiters
		op2 = strtok(NULL, " ,$");
		op3 = strtok(NULL, " ,$");

		operand1 = atoi(op1);
		operand2 = atoi(op2);
		operand3 = atoi(op3);

		if (check_Registers(operand1) && check_Registers(operand2) && check_Memory(operand3))
		{
			MAR = operand3;
			printf("\n *** JUMP_LESS_THAN*** \n");
			printf("\n Value of PC is  = 0x%p\n\n", programCounter);
			JLT(operand1, operand2, operand3);
			printf("\n Value of PC after JUMP is  = 0x%p \n\n", programCounter);

		}

	}

	else if ((!strcmp(instType, "JGT")) || (!strcmp(instType, "jgt")))
	{
		op1 = strtok(NULL, " ,$ ");	//using " ,$" as delimiters
		op2 = strtok(NULL, " ,$");
		op3 = strtok(NULL, " ,$");

		operand1 = atoi(op1);
		operand2 = atoi(op2);
		operand3 = atoi(op3);

		if (check_Registers(operand1) && check_Registers(operand2) && check_Memory(operand3))
		{
			MAR = operand3;
			printf("\n***JUMP_GREATER_THAN***\n");
			printf("\n Value of PC is  = 0x%p\n\n", programCounter);
			JGT(operand1, operand2, operand3);
			printf("\n Value of PC after JUMP is  = 0x%p \n\n", programCounter);

		}

	}

	else if ((!strcmp(instType, "JNE")) || (!strcmp(instType, "jne")))

	{
		op1 = strtok(NULL, " ,$ ");	//using " ,$" as delimiters
		op2 = strtok(NULL, " ,$");
		op3 = strtok(NULL, " ,$");

		operand1 = atoi(op1);
		operand2 = atoi(op2);
		operand3 = atoi(op3);

		if (check_Registers(operand1) && check_Registers(operand2) && check_Memory(operand3))
		{
			MAR = operand3;
			printf("\n***JUMP_NOT_EQUAL***\n");
			printf("\n Value of PC is  = 0x%p\n\n", programCounter);
			JNE(operand1, operand2, operand3);
			printf("\n Value of PC after JUMP is  = 0x%p \n\n", programCounter);

		}

	}

	else if ((!strcmp(instType, "JE")) || (!strcmp(instType, "je")))
	{
		op1 = strtok(NULL, " ,$ ");	//using " ,$" as delimiters
		op2 = strtok(NULL, " ,$");
		op3 = strtok(NULL, " ,$");

		operand1 = atoi(op1);
		operand2 = atoi(op2);
		operand3 = atoi(op3);

		if (check_Registers(operand1) && check_Registers(operand2) && check_Memory(operand3))
		{
			MAR = operand3;
			printf("\n***JUMP_EQUAL***\n");
			printf("\n Value of PC is  = 0x%p\n\n", programCounter);
			JE(operand1, operand2, operand3);
			printf("\n Value of PC after JUMP is  = 0x%p \n\n", programCounter);

		}

	}

	else if ((!strcmp(instType, "JNZ")) || (!strcmp(instType, "jnz")))
	{
		op1 = strtok(NULL, " ,$ ");	//using " ,$" as delimiters
		op2 = strtok(NULL, " ,$");
		//op3 = strtok (NULL, " ,$");

		operand1 = atoi(op1);
		operand2 = atoi(op2);

		if (check_Registers(operand1) && check_Memory(operand2))
		{
			MAR = operand2;
			printf("\n***JUMP_NON_ZERO***\n");
			printf("\n Value of PC is  = 0x%p\n\n", programCounter);
			JNZ(operand1, operand2);
			printf("\n Value of PC after JUMP is  = 0x%p \n\n", programCounter);

		}

	}

	else if ((!strcmp(instType, "JZ")) || (!strcmp(instType, "jz")))
	{
		op1 = strtok(NULL, " ,$ ");	//using " ,$" as delimiters
		op2 = strtok(NULL, " ,$");
		//op3 = strtok (NULL, " ,$");

		operand1 = atoi(op1);
		operand2 = atoi(op2);

		if (check_Registers(operand1) && check_Memory(operand2))
		{
			MAR = operand2;
			printf("\n***JUMP_ZERO***\n");
			printf("\n Value of PC is  = 0x%p\n\n", programCounter);
			JZ(operand1, operand2);
			printf("\n Value of PC after JUMP is  = 0x%p \n\n", programCounter);

		}
	}

	else if ((!strcmp(instType, "JMP")) || (!strcmp(instType, "jmp")))
	{

		op1 = strtok(NULL, " ,$ ");	//using " ,$" as delimiters
		operand1 = atoi(op1);

		if (check_Memory(operand1))
		{
			MAR = operand1;
			printf("\n***UNCONDITIONAL_JUMP***\n");

			printf("\n Value of PC is  = 0x%p\n\n", programCounter);
			JMP(operand1);
			printf("\n Value of PC after JUMP is  = 0x%p \n\n", programCounter);

		}

	}


	else if ((!strcmp(instType, "LEA")) || (!strcmp(instType, "lea")))
	{
		op1 = strtok(NULL, " ,$ ");	//using " ,$" as delimiters
		op2 = strtok(NULL, " ,$");
		//op3 = strtok (NULL, " ,$");

		operand1 = atoi(op1);
		operand2 = atoi(op2);

		if (check_Registers(operand1) && check_Memory(operand2))
		{
			MAR = operand2;
			printf("\n***LOAD_EFFECTIVE_ADDRESS***\n");
			printf("\n Value of effective address is  = 0x%d\n\n", &Memory[operand2]);
			LEA(operand1, operand2);
			printf("\n Value of register after load is  = 0x%d \n\n", regFile[operand1]);

		}
	}

	else if ((!strcmp(instType, "LEA_BYTES")) || (!strcmp(instType, "lea_bytes")))
	{
		op1 = strtok(NULL, " ,$ ");	//using " ,$" as delimiters
		op2 = strtok(NULL, " ,$");
		op3 = strtok(NULL, " ,$");

		operand1 = atoi(op1);
		operand2 = atoi(op2);
		operand3 = atoi(op3);

		if (check_Registers(operand1) && check_Memory(operand2))
		{
			MAR = Add_ALU(operand2, operand3);
			printf("\n***LOAD_EFFECTIVE_ADDRESS_BYTES***\n");
			printf("\n Value of effective address is  = 0x%d\n\n", &(Memory[operand2 + operand3]));
			LEA_BYTES(operand1, operand2, operand3);
			printf("\n Value of register after load is  = 0x%d \n\n", regFile[operand1]);

		}
	}

	else if ((!strcmp(instType, "LEA_WORD")) || (!strcmp(instType, "lea_word")))
	{
		op1 = strtok(NULL, " ,$ ");	//using " ,$" as delimiters
		op2 = strtok(NULL, " ,$");
		op3 = strtok(NULL, " ,$");

		operand1 = atoi(op1);
		operand2 = atoi(op2);
		operand3 = atoi(op3);

		if (check_Registers(operand1) && check_Memory(operand2))
		{
			int x = Mul_ALU(operand3, BYTES_IN_WORD);
			MAR = Add_ALU(operand2, x);
			printf("\n***LOAD_EFFECTIVE_ADDRESS_WORD***\n");
			printf("\n Value of effective address is  = 0x%d\n\n", &(Memory[operand2 + x]));
			LEA_WORD(operand1, operand2, operand3);
			printf("\n Value of register after load is  = 0x%d \n\n", regFile[operand1]);

		}
	}

	else if ((!strcmp(instType, "LI")) || (!strcmp(instType, "li")))
	{
		op1 = strtok(NULL, " ,$ ");	//using " ,$" as delimiters
		op2 = strtok(NULL, " ,$");
		//op3 = strtok (NULL, " ,$");

		operand1 = atoi(op1);
		operand2 = atoi(op2);
		//operand3 = atoi(op3);

		if (check_Registers(operand1))
		{

			printf("\n***LOAD_IMMEDIATE***\n");
			LI(operand1, operand2);
			printf("\n Value of register after load is  = 0x%d \n\n", regFile[operand1]);

		}
	}

	else if (!strcmp(instType, "DOWHILE") || (!strcmp(instType, "dowhile")))
	{
		printf("\nImplementing the do while loop\n");
		dowhile();
		printf("\nExiting loop\n");
		printf("\n Value of registers are regFile[7]= %d and regFile[3]=%d\n\n", regFile[7], regFile[3]);

	}


	else if ((!strcmp(instType, "FOR_LOOP")) || (!strcmp(instType, "for_loop")))
	{
		printf("\nImplementing the for loop\n");
		for_loop();
		printf("\nExiting loop\n");
		printf("\n Value of registers are regFile[7]= %d and regFile[3]=%d\n\n", regFile[7], regFile[3]);

	}

	else if ((!strcmp(instType, "WHILEDO")) || (!strcmp(instType, "whiledo")))
	{
		printf("\nImplementing the while do loop\n");
		whiledo();
		printf("\nExiting loop\n");
		printf("\n Value of registers are regFile[7]= %d and regFile[3]=%d\n\n", regFile[7], regFile[3]);

	}

	else if ((!strcmp(instType, "BSearch")) || (!strcmp(instType, "bsearch"))) //enter bsearch following a space for tokenizing 
	{

		int ch = 0, key = 0, low = 0, high = 0;
		char *choice = (char *)malloc(sizeof(60));
		char *keyval = (char *)malloc(sizeof(60));
		printf("\n Choose from the following sets :");
		printf("\n 1. Natural Numbers Set");
		printf("\n 2. Even Set");
		printf("\n 3. Odd Set");
		printf("\n 4. Set 4");
		printf("\n 5. Set 5");
		printf("\n 6. Set 6\n");
		printf("\nEnter a choice: ");
		fgets(choice, 25, stdin);
		ch = atoi(strtok(choice, " ,\t\n"));

		switch (ch)
		{
		case 1:
			printf("\n Displaying the contents from set 1: ");
			high = (sizeof(set1) / sizeof(set1[0])) - 1; //to find the size of array
			int i;
			for (i = 0; i <= high; i++)
				printf("%d \t ", set1[i]);
			printf("\nPlease enter the search item:");
			fgets(keyval, 25, stdin);
			key = atoi(strtok(keyval, " ,\t\n"));
			regFile[1] = key; 	//Register 1 holds the key to be searched 
			printf("Register1 holds the value of key: %d\n", regFile[1]);
			regFile[2] = bSearch(set1, low, high, regFile[1]); //Register 2 holds the position of key
			printf("Register2 holds the index of the key value: %d\n", regFile[2]);
			break;

		case 2:
			printf("\n Displaying the contents from set 2: ");
			high = (sizeof(set2) / sizeof(set2[0])) - 1; //to find the size of array
			//int i;
			for (i = 0; i <= high; i++)
				printf("%d \t ", set2[i]);
			printf("\nPlease enter the search item:");
			fgets(keyval, 25, stdin);
			key = atoi(strtok(keyval, " ,\t\n"));
			regFile[1] = key; 	//Register 1 holds the key to be searched 
			printf("Register1 holds the value of key: %d\n", regFile[1]);
			regFile[2] = bSearch(set2, low, high, regFile[1]);
			printf("Register2 holds the index of the key value: %d\n", regFile[2]);
			break;

		case 3:
			printf("\n Displaying the contents from set 3: ");
			high = (sizeof(set3) / sizeof(set3[0])) - 1; //to find the size of array
			//int i;
			for (i = 0; i <= high; i++)
				printf("%d \t ", set3[i]);
			printf("\nPlease enter the search item:");
			fgets(keyval, 25, stdin);
			key = atoi(strtok(keyval, " ,\t\n"));
			regFile[1] = key; 	//Register 1 holds the key to be searched 
			printf("Register1 holds the value of key: %d\n", regFile[1]);
			regFile[2] = bSearch(set3, low, high, regFile[1]);
			printf("Register2 holds the index of the key value: %d\n", regFile[2]);
			break;

		case 4:
			printf("\n Displaying the contents from set 4: ");
			high = (sizeof(set4) / sizeof(set4[0])) - 1; //to find the size of array
			//int i;
			for (i = 0; i <= high; i++)
				printf("%d \t ", set4[i]);
			printf("\nPlease enter the search item:");
			fgets(keyval, 25, stdin);
			key = atoi(strtok(keyval, " ,\t\n"));
			regFile[1] = key; 	//Register 1 holds the key to be searched 
			printf("Register1 holds the value of key: %d\n", regFile[1]);
			regFile[2] = bSearch(set4, low, high, regFile[1]);
			printf("Register2 holds the index of the key value: %d\n", regFile[2]);
			break;

		case 5:
			printf("\n Displaying the contents from set 5: ");
			high = (sizeof(set5) / sizeof(set5[0])) - 1; //to find the size of array
			//int i;
			for (i = 0; i <= high; i++)
				printf("%d \t ", set5[i]);
			printf("\nPlease enter the search item:");
			fgets(keyval, 25, stdin);
			key = atoi(strtok(keyval, " ,\t\n"));
			regFile[1] = key; 	//Register 1 holds the key to be searched 
			printf("Register1 holds the value of key: %d\n", regFile[1]);
			regFile[2] = bSearch(set5, low, high, regFile[1]);
			printf("Register2 holds the index of the key value: %d\n", regFile[2]);
			break;


		case 6:
			printf("\n Displaying the contents from set 6: ");
			high = (sizeof(set6) / sizeof(set6[0])) - 1; //to find the size of array
			//int i;
			for (i = 0; i <= high; i++)
				printf("%d \t ", set6[i]);
			printf("\n Please enter the search item:");
			fgets(keyval, 25, stdin);
			key = atoi(strtok(keyval, " ,\t\n"));
			regFile[1] = key; 	//Register 1 holds the key to be searched 
			printf("Register1 holds the value of key: %d\n", regFile[1]);
			regFile[2] = bSearch(set6, low, high, regFile[1]);
			printf("Register2 holds the index of the key value: %d\n", regFile[2]);
			break;

		default:
			printf("\n Invalid entry. Please choose the correct option");
		}
	}

	else if ((!strcmp(instType, "MULT")) || (!strcmp(instType, "mult")))	//enter mult following space to allow for tokenizing 
	{

		int x, i;
		printf("\n****Multiple Instructions for jumps using labels****\n");
		printf("\nEnter the number of Instructions:"); //when entering the third instruction with label, give as label(space):(space)instruction
		scanf("%d", &x);
		getchar();

		
		char **instruction = (char**)malloc(sizeof(char *)*x);

		if (instruction == NULL)
		{
			fprintf(stderr, "Memory Not Allocated\n");
			return 1;
		}
		else
		{
			for (i = 0; i < x; i++)
			{
				instruction[i] = (char *)malloc(sizeof(char)* 30);
				if (instruction[i] == NULL)
				{
					fprintf(stderr, "Memory not allocated\n");
					return 1;
				}
				printf("\nEnter an instruction: ");
				fgets(instruction[i], 25, stdin);
				printf("The entered instruction is: %s", instruction[i]);

			}
		}

		multipleInstructionDecode(instruction, x);

		for (i = 0; i<x; i++)
		{
			free(instruction[i]);
		}

		free(instruction);

	}

	else if ((!strcmp(instType, "exit")) || (!strcmp(instType, "EXIT")))
	{
		printf("Shutting Down !!!\n");
		exit(0);
	}

	else
		printf("Enter the instruction in correct format\n");


}


//Executing the given instruction

void instructionExecution(char *inst, int regOffset, int memOffset)
{
	uint32_t *regFileAddress = &regFile[regOffset];
	memoryFetch(inst, memOffset, regFileAddress);
}

//Performing load and store 

void memoryFetch(char *instruction, int memoryAddress, uint32_t *regAddress)
{
	MAR = memoryAddress;
	MDR = (!strcmp(instruction, "lw")) ? Memory[MAR] : *regAddress;
	if (!strcmp(instruction, "lw"))
	{
		*regAddress = MDR;
	}
	else
	{
		Memory[MAR] = *regAddress;
	}
}


void display_flags()
{
	printf("*** FLAGS ***\n");
	printf("The following flags are used to implement flags\n");
	printf("Zero Flag: 0\n");
	printf("Overflow Flag: 1\n");
	printf("Carry flag: 2\n");
	printf("Divide by zero flag: 3\n");

	printf("The contents of flags are:\n");
	int j;
	for (j = 0; j < 9; j++)
		printf("||Flag%d = %d ", j, FLAGS[j]);
}
// printing the contents of CPU 

void display()
{
	printf("\nThe contents of CPU \n");
	printf("--------------------------------\n");
	printf("MDR = %d\n", MDR);
	printf("MAR = 0x%x\n", MAR);
	printf("PC = 0x%p\n", programCounter);
	programCounter++;
	printf("The contents of registers are:\n");
	printf("$1 = %d\n$2 = %d\n$3 = %d\n$4 = %d\n$5 = %d\n$6 = %d\n$7 = %d\n$8 = %d\n$9 = %d\n$10 = %d\n$11 = %d\n$12 = %d\n$13 = %d\n$14 = %d\n\n",
		regFile[1], regFile[2], regFile[3], regFile[4], regFile[5], regFile[6], regFile[7], regFile[8], regFile[9], regFile[10], regFile[11], regFile[12], regFile[13], regFile[14]);
	display_flags();

}


reset_flags()
{
	int i;
	for (i = 0; i<8; i++)
		FLAGS[i] = 0;
}






int main()
{
	initializeMemory();
	char input[150];
	printf("\n*******************************************\n");
	printf("                                                 \n");
	printf("\t!!!! BEST CPU SETUP !!!!\t\t\n");
	printf("                                                 \n");
	printf("*******************************************\n");
	display();
	while (1) // to exit the loop enter "exit" following space (for delimiting)"
	{
		reset_flags();		// resets the flags to zero
		printf("\n\nEnter Instructions in the following format\n");
		printf("Example format: ADD $1,$2,$3 | SUB $1,$2,$3 | MUL $1,$2,$3 | DIV $1,$2,$3 | MOD $1,$2,$3 | LW $1,120 | SW $2,120 | PUSH $2 | POP $2 | EXIT to shut down\n\n ");
		printf("\nEnter Jump Instructions for JLT, JGT, JNE, JE as Instruction $register, $register, Index | for JNZ, JZ as Instr $register, Index | for jmp as jmp index\n");
		printf("\nEnter LEA instruction as LEA $egister, index\n");
		printf("\nImplement loops by entering dowhile | whiledo | for_loop following space\n\n");
		printf("\nEnter Binary Search as bsearch following space\n");
		printf("\nMultiple instructions:For jumps using labels..enter mult following space\n");
		printf("\nEnter label instruction as in label : lw $2,120\n");




		fgets(input, 50, stdin);
		instructionDecode(input);
		display();
		printf("\n\n");
	}

	return 0;
}



