#include <stdio.h>
#include "utilities.h"


bool errorMsg(char* msg)
{	
	printf(RED_B);
	printf("\nERROR: %s\n",msg);
	printf(RESET);
	return false;
}

void warningMsg(char* msg)
{	
	printf(YELLOW_B);
	printf("\nWARNING: %s\n",msg);
	printf(RESET);
	
}

void updateMsg(char* msg)
{	
	printf(GREEN);
	printf("\n%s\n",msg);
	printf(RESET);
	
}




