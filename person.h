#ifndef PERSON_H
#define PERSON_H

//person status macros
#define INFECTED 0
#define SUSCEPTIBLE 1

//Hardcoded touch rates (see ref papers)
#define FACE_TOUCH_RATE_HR 15
#define ENVR_TOUCH_RATE_HR 204

#include "pathogen.h"

typedef struct Person{
	int status;		
	
	unsigned long fdose;   		//number of pathogens (dose) on finger
	double mdose;   			//pathogens in/on mucosal membranes (cumulative exponential average)
	
	
	unsigned int pool;   		//the pool this person originated from
		
	unsigned int jumps; 		//controls the number of new locations person can visit in MC

}Person;

bool person_selfInoc(Person* person, Pathogen* pathogen,tgParam* face,tgParam* env, unsigned int step_m);


#endif
