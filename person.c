#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "utilities.h"
#include "probability.h"

#include "person.h"


/*At each time step, compute pathogen decay on hands and face touching rate and 
 * transfer to mucosal membranes. Mucosal dose is time weighted over INOC_PERIOD;
 * at each time step, compute probability that this dose has 'infected' the person*/

bool person_selfInoc(Person* person, Pathogen* pathogen,tgParam* face_params,tgParam* envr_params, unsigned int step_m)
{
	
	//double alpha = (double)step_m/INOC_PERIOD_MIN;   //exponential time-average
	double alpha = (double)step_m/(pathogen->inoc_min);   //exponential time-average
	double p = 0;

	//Deposit rate (face/ envr)
	double  dep;
	//number of touch event
	unsigned int nt; 
	//convert touch rates (/hr) to rate per step_m
	double den = 60 * 10;  //60 minutes per hr and x10 factor reduction to allow for just one dominant finger?
	
	
	//Face touching
	dep  = sampleNormT(face_params);  //transfer (i,e, deposited) onto face ~30%
	
	nt  = numEvents((double)(FACE_TOUCH_RATE_HR*step_m)/den);
	unsigned long  cmdose = (unsigned long) ( (person->fdose)*(1 - pow( (1-dep), nt) ) ); //dose to mucosal membranes
	
		//time weighted cumulative dose 
		person->mdose = cmdose + ( (1-alpha)*(person->mdose) );   
			
			
		//remove mdose from hands
		person->fdose -= cmdose;
		person->fdose = ( (person->fdose) <0) ? 0:(person->fdose);
	
	
	
	//Loss to environment
	dep  = sampleNormT(envr_params);  //transfer (i,e, deposited) onto envr ~5%%
	
	nt  = numEvents((double)(ENVR_TOUCH_RATE_HR*step_m)/den);
	unsigned long  codose = (unsigned long) ( (person->fdose)*(1 - pow( (1-dep), nt) ) ); //dose to environment

		//remove codose from hands
		person->fdose -= codose;
		person->fdose = ( (person->fdose) <0) ? 0:(person->fdose);
		
		
		//decay
		person->fdose = (unsigned long) ( (person->fdose)*(pathogen->decay_h) );
			
			
			
	//probability of infection mdoses are in ID50 so each contributes a half chance
	p = 1 - exp(-0.7*(person->mdose)/(pathogen->_ID50) );  

	return accept(p);
}
