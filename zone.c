#include <stdio.h>
#include <stdlib.h>

#include "utilities.h"
#include "zone.h"
#include "person.h"

bool zone_config(Zone* zone, Hash_t* H, unsigned int n, unsigned int N )
{
	char type[40];
	char buff[40];
	int i=0;
	
	//init all pointers to null ...
	zone->_mc.prb = NULL;
	zone->bioburden = NULL;
	
	zone->A = NULL;
	zone->D = NULL;
	
	zone->INTERACTION_RATE = 1;  //default (not really in use)  or option to change using config file..?
	
	
	//Fomite 'type' is specified by trunGauss + a rate parameter
	if(!hash_lookup_csv(H,"FOMITE_TYPE",n,type))
	 return errorMsg("location config: FOMITE_TYPE not specified");
	 
	
		if(!hash_lookup_csv(H,type,0,buff)) 
			return errorMsg("location config: FOMITE_TYPE[0] not specified");
			
		zone->fomType.md = atof(buff);
		
		if(!hash_lookup_csv(H,type,1,buff)) 
			return errorMsg("location config: FOMITE_TYPE[1] not specified");
						
		zone->fomType.sd = atof(buff);
		
		
		if(!hash_lookup_csv(H,type,2,buff)) 
			return errorMsg("location config: FOMITE_TYPE[2] not specified");
						
		zone->fomType.mn = atof(buff);
		
		
		if(!hash_lookup_csv(H,type,3,buff)) 
			return errorMsg("location config: FOMITE_TYPE[3] not specified");
					
		zone->fomType.mx = atof(buff);
		
		
		if(!hash_lookup_csv(H,type,4,buff)) 
			return errorMsg("location config: FOMITE_TYPE[4] not specified");
					
		zone->fomL = atof(buff);
		
		
	//Number of identical fomites in this zone
	if(!hash_lookup_csv(H,"FOMITES",n,buff)) 
		return errorMsg("location config: FOMITES not specified");
			
	zone->numFomites = atoi(buff);
	
	//init bioburden counters
	zone->bioburden = (unsigned long*)calloc(zone->numFomites,sizeof(unsigned long));
	
		
	
	//Rate at which people leave this zone/ how long do they stick around
	if(!hash_lookup_csv(H,"ZONE_L",n,buff)) 
		return errorMsg("location config: ZONE_L not specified");
				
	zone->L = atof(buff);
	
	
	//Fomite cleaning/disinfecting rate
	if(!hash_lookup_csv(H,"CLEAN_RATE_HR",n,buff)) 
		return errorMsg("location config: CLEAN_RATE_HR not specified");
		
	zone->CR = atof(buff)/60;  //Note: config file=per hour, but utilized as per min in sim
	
	
	
	//Markov chain init
	zone->_mc.num_states = N;
	
	zone->_mc.prb = malloc(N*sizeof(double));
	
	
	if(!hash_lookup_csv(H,"ZONE_MC",n,type))
	 return errorMsg("location config: ZONE_MC not specified");
	 
	for(i=0;i<N;i++)
	{
		if(!hash_lookup_csv(H,type,i,buff)) 
		return false;
			
		zone->_mc.prb[i] = atof(buff);
	}
	
	
	
	zone->A = new_queue(sizeof(Person));
	zone->D = new_queue(sizeof(Person));
	
	return true;
}

bool pool_config(Zone* pool, Hash_t* H, unsigned int n,unsigned int N)
{
	
	//Pools and Zones are essentially the same (but different params in use)
	
	
	char buff[40];
	char type[40];
	
	int i=0;
		
	unsigned int num_people,num_jumps;
		
	//init all pointers to null ...
	pool->_mc.prb = NULL;
	pool->bioburden = NULL;   //pools have no fomites
	
	pool->A = NULL;
	pool->D = NULL;
	
	
	//Rate of emptying pool
	if(!hash_lookup_csv(H,"POOL_L",n,buff)) 
		return errorMsg("location config: POOL_L not specified");
			
	pool->L = atof(buff);
	
	
	//Number of people in pool (make use of numFomites parameter to store value)
	if(!hash_lookup_csv(H,"PEOPLE",n,buff)) 
		return errorMsg("location config: PEOPLE not specified");
			
	num_people = atoi(buff);
	
	pool->numFomites = num_people;  
	
	
	//Markov chain: Number of jumps i.e. total number of zones that will be visited by individuals in sim	
	if(!hash_lookup_csv(H,"JUMPS",n,buff)) 
		return errorMsg("location config: JUMPS not specified");
			
	num_jumps = atoi(buff);
	
		
	pool->_mc.num_states = N;
	pool->_mc.prb = malloc(N*sizeof(double));
	
	
	if(!hash_lookup_csv(H,"POOL_MC",n,type))
	 return errorMsg("location config: POOL_MC not specified");
	 
	for(i=0;i<N;i++)
	{
		if(!hash_lookup_csv(H,type,i,buff)) 
		return errorMsg("pool config: POOL_MC[i] not specified");
		
		pool->_mc.prb[i] = atof(buff);
		
	}
	
	pool->A = new_queue(sizeof(Person));
	pool->D = new_queue(sizeof(Person));
	Person dummy;
	
	//load pool with (default) people
	for(i=0;i<num_people;i++)
	{
	
		dummy.status = SUSCEPTIBLE;
		dummy.fdose = 0;
		dummy.mdose = 0.0;
		
		
		dummy.pool = n;
		dummy.jumps = num_jumps; 
		
		enqueue(pool->D,&dummy);
	}

	
	return true;
}


void zone_close(Zone* zn)
{
	//delete queues and mc
	free(zn->_mc.prb);
	free(zn->bioburden);
	
	
	delete_queue(zn->A);
	delete_queue(zn->D);
	
}


//Markov Chains: given a person at Zone[current], where to go next?
unsigned int zone_mc(Zone* zone){

	double r,sum;
	unsigned int i,states;

	states = zone->_mc.num_states;  // >0 (may be ==1)

	//jump to a new location/zone according to MC parameters?
	r = uniform();
	sum = 0;
	for(i=0; i<states;i++)
	{
		sum=sum+(zone->_mc.prb[i]);

		if(r<sum)
		return i;

	}

	errorMsg("zone_mc error: should never reach this point! Check your MC values!");
	return 0; 

}

/*seeds a zone's fomites with initial contamination level;
num_pathogens will be generated from pathogen_shed function*/
void zone_contaminated(Zone* zone, unsigned long num_pathogens)
{
	int i;
		
	for (i=0;i<zone->numFomites;i++)
		zone->bioburden[i] = num_pathogens;
}	

//clean particular fomites at that zone
void zone_clean(Zone* zone, unsigned int cfom)
{
	/*from ref paper: geo mean reduction of 94.1 95%CI = [71.4 ,  98.8]
	* therefore fraction left over 0.059  CI [0.012 , 0.286]
	* equals a logNorm:  mu = -3.0187  sd = 0.614
	*/
	double fraction_alive = samplelogNorm(-3.0187,0.614,1);
	
	zone->bioburden[cfom]  =  (unsigned long) ( (zone->bioburden[cfom])*fraction_alive );   //implicit rounding; use cast anyways

}
