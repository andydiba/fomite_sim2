#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "probability.h"
#include "utilities.h"

#include "pathogen.h"


void pathogen_print(Pathogen* P)
{
	updateMsg("Pathogen specifications:");
	printf("U(%.0f,%.0f), id50:%.0f, decay/min:%.3f,%.3f, inoc_min:%.3f\n",
	P->_shed[0],P->_shed[1],P->_ID50,P->decay_f,P->decay_h,P->inoc_min);
		
}


//Pathogen shedding based on uniform distribution
unsigned long pathogen_shed(Pathogen* P)
{
	double dose;  
	double r = uniform();

	dose = P->_shed[0]+ (P->_shed[1] - P->_shed[0])*r;
	
	//dose = dose/(P->_ID50); //normalised
	//dose = 0.014*dose; //finger tip proportion ~1.4or1.5%    //multiply by a certain number of touches??

	return (unsigned long)dose;
	
}


//Initialisation
bool pathogen_config(Pathogen* P, Hash_t* H)
{
	char* temp;
	char buff[40];
	
	unsigned int tStep_m;
	double tStep_hr;
	
	//Get resolution of simulation to pre-calculate decay fractions
	temp = hash_lookup(H,"RES_MIN");
	if(!temp)
		{errorMsg("Pathogen configuration: RES_MIN not found");
			return false;
		}
	
	tStep_m = atoi(temp);
	
	tStep_hr = (double)tStep_m /60;
	
	
	
	//Infectious dose
	temp = hash_lookup(H,"PATHOGEN_ID50");
	if(!temp)
		{errorMsg("Pathogen configuration: PATHOGEN_ID50 not found");
			return false;
		}
	
	P->_ID50 = atof(temp);
	
	
	//Inoculation period  (~decay rate in mucosal membranes)
	temp = hash_lookup(H,"PATHOGEN_INOC_PERIOD_MIN");
	if(!temp)
		{errorMsg("Pathogen configuration: PATHOGEN_INOC_PERIOD_MIN not found");
			return false;
		}
	
	P->inoc_min = atof(temp);
	
	
	//Decay rate on fomites and hands
	temp = hash_lookup(H,"PATHOGEN_DECAY_FOMITE");
	if(!temp)
		return errorMsg("Pathogen configuration: PATHOGEN_DECAY_FOMITE not found");
		
	
	//P->_muf = atof(temp);
	P->decay_f =  pow(atof(temp),tStep_hr);   //convert hr rate to tStep
  	
  	
  	temp = hash_lookup(H,"PATHOGEN_DECAY_HANDS");
	if(!temp)
		return errorMsg("Pathogen configuration: PATHOGEN_DECAY_HANDS not found");
		
	//P->_muh = atof(temp);
	P->decay_h =  pow(atof(temp),tStep_hr); 
  	
  	
  	
  	//Pathogen shedding ~U[shed[0],shed[1])
	
	if(!hash_lookup_csv(H,"PATHOGEN_SHED",0,buff))
		return errorMsg("Pathogen configuration: PATHOGEN_SHED[0] not found");
		
	
	P->_shed[0] = atof(buff);
	
	
	if(!hash_lookup_csv(H,"PATHOGEN_SHED",1,buff))
		return errorMsg("Pathogen configuration: PATHOGEN_SHED[1] not found");
			
	
	P->_shed[1] = atof(buff);
	
	return true;
  	
}



