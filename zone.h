#ifndef ZONE_H
#define ZONE_H

#include "queue.h"
#include "probability.h"
#include "hash.h"

typedef struct Zone{
	
	Queue* A;					//'Arrival' queue
	Queue* D;					//'Departure' queue

	double INTERACTION_RATE;    //default set to 1;(optional parameter, not currently in use)

	MarkovChain _mc;			//governs how people move on to new zones after this one
    double L; 					//average rate (per minute) of people leaving location
	

	//Fomite properties
	unsigned int numFomites;	//no. of identical fomites in this zone (e.g. a bank of ATMs) 
	unsigned long* bioburden;   //no. of pathogens (on each fomite)
	

	tgParam fomType;			//type of fomite, defines by a truncGauss distribution
	double fomL;				//rate (per minute) of fomite use
 
							
	double CR;	 				//cleaning rate in zone (applied to each and all fomites if numFomites>1)
								//specified as cleaning rate per minute
}Zone;


bool zone_config(Zone*, Hash_t*, unsigned int, unsigned int);
bool pool_config(Zone* , Hash_t* , unsigned int, unsigned int);

unsigned int zone_mc(Zone*);

void zone_contaminated(Zone* , unsigned long);
void zone_clean(Zone*, unsigned int);

void zone_close(Zone*);

#endif
