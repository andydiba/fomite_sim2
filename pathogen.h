#ifndef PATHOGEN_H
#define PATHOGEN_H

#include "hash.h"

//define 'pathogen' structure
typedef struct Pathogen{

	double _shed[2]; 			//pathogen shedding sampled from uniform distribution: U(shed[0],shed[1])
  	double _ID50; 				//Infectious Dose 50%      ( could store as exponent i.e. 10^ID50?)
  	
   	
  	double decay_f; 			//pathogen %survival/tStep (on fomite)
  	double decay_h;				//pathogen %survival/tStep (on skin)
  	
  	double inoc_min;        	//pathogen inoc period  (effectively ~ survival/step  in mucus membranes)
  	
  	
}Pathogen;


bool pathogen_config(Pathogen*, Hash_t*);

unsigned long pathogen_shed(Pathogen*);

void pathogen_print(Pathogen*);

#endif
