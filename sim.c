/*
Copyright 2020 Andrew Di Battista andrew.di.battista@ultraleap.com

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>


#include "queue.h"
#include "probability.h"
#include "hash.h"
#include "utilities.h"

#include "pathogen.h"
#include "person.h"
#include "zone.h"



struct stat{

	double mu;
	double sd;
	unsigned long val;


};

//main simulation loop
void fomite_sim(
	Zone* pool,
	unsigned int numPools,
	Zone* zone,
	unsigned int numZones,
	tgParam* DEPOSIT,tgParam* PICKUP,
	tgParam* FACE,tgParam* ENVR,
	Pathogen* pathogen,
	unsigned int tStep_m,
	unsigned int duration_min,
	char* outfile,unsigned int LOG_B
)
{
	unsigned int i,j,m,n,p, num_fom, nl, num_touch;
	double deposit,pickup,chooseF;
	unsigned long cf,cs,dcfs;

	unsigned int nSteps = duration_min/tStep_m;


	unsigned int cfom; //index of current fomite
	Person dummy; 		//buffer to store current person
	Zone* czone;		//pointer to a current zone


	FILE *fp;
	char biofile[80];
	sprintf(biofile,"bio_%s",outfile);

	if(LOG_B)
		 fp = fopen(biofile, "a");


for(n=0;n<nSteps;n++)
{



	//Log bio-burdens on each fomite at certain time intervals e.g tStep_m
	if(LOG_B)
	{
		//if((n*tStep_m)%5==0 )
		//{  //or whatever resolution...e.g. every 5min

			//write timestamp,
			fprintf(fp,"%d",n*tStep_m);

			for(p=0;p<numZones;p++)
			{
				czone = &zone[p];
				num_fom = czone->numFomites;

				for (cfom=0;cfom<num_fom;cfom++) //swap for queue to fom...
				{
						if(czone->bioburden[cfom] < 1)
							fprintf(fp,",%d",0); // leading with comma!
						else
							fprintf(fp,",%lu",czone->bioburden[cfom]); // leading with comma!
				}
			}
			//write newline
			 fprintf(fp,"\n");


		//} //timestamp
	}//log_b





	for(i=0;i<numPools;i++)
	{
		//at each time step, get 'm' people from initial 'pool' and send them to first 'zone(s)'
		m = numEvents(tStep_m*(pool[i].L));

		p=0;
		while(!isEmpty_queue(pool[i].D) && p<m)
		{
				dequeue(pool[i].D,&dummy);
				nl= zone_mc(&pool[i]);  //new location

				if(dummy.jumps!=0) //it won't be 0 if configured properly
				{
					dummy.jumps = dummy.jumps -1;
					enqueue(zone[nl].A,&dummy);
				}
			p++;
		}
	}//end of pool dequeuing

	for(p=0;p<numZones;p++)
	{
		/*At each location with num_fom available touchscreens and grab 'm' people (if available)
		 out of 'arrival' queue to interact with them*/

		czone = &zone[p];

		num_fom = czone->numFomites;

		double INTERACTION_RATE = czone->INTERACTION_RATE;

		//rate at which queue depletes given # of foms and rate of use
		double QUEUE_RATE = (czone->fomL)*tStep_m*num_fom;

		if(!isEmpty_queue(czone->A))  //initial check, if queue already empty no need to compute the rest!
		{
			m = numEvents(QUEUE_RATE); //# of fomite interactions

			i=0;
			while(!isEmpty_queue(czone->A)  && i<m)
			{
				//grab ith person out of queue, they may or may not interact...
				dequeue(czone->A,&dummy);

				if(accept(INTERACTION_RATE))
				{
					//choose one of the available fomites to interact with at random
					chooseF = uniform();
					chooseF= ceil(chooseF*num_fom)-1;
					cfom=(unsigned int)chooseF; //index of current fom in zone

					//number of touch events for interaction with fomite
					num_touch = (unsigned int)round(sampleNormT(&(czone->fomType)));
					//printf("\n nt: %d",num_touch);

					//compute transfers to and from using gradient model
					//transfer  = samplelogNorm(TRANSFER->md, TRANSFER->sd, TRANSFER->mx);
					deposit  = sampleNormT(DEPOSIT);
					pickup   = sampleNormT(PICKUP);


					cf = dummy.fdose;
					cs = czone->bioburden[cfom];

					for(j=0;j<num_touch;j++)
					{
						dcfs = (unsigned long) ( (deposit*cf) - (pickup*cs) );  //implicit rounding off
						cf = cf - dcfs;
						cs = cs + dcfs;
					}

					dummy.fdose = cf;
					czone->bioburden[cfom]= cs;


				}//interaction rate


				enqueue(czone->D,&dummy); //arrivals go to departure

				i++;

			}//while loop
		}//empty queue check


		//for each person..die off plus inoculation...
		//in this zone check both queues and run self inoc to update finger pathogen levels, change state if true
		m = queue_count(czone->A);
		for(j=0;j<m;j++)
		{
			peek_queue(czone->A,&dummy,j); //copies queued person into dummy

			if( person_selfInoc(&dummy,pathogen,FACE,ENVR,tStep_m))  //updates finger pathogen overall levels
				dummy.status = INFECTED;

			insert_queue(czone->A,&dummy,j);  //replace queue element with updated dummy

		}

		m = queue_count(czone->D);
		for(j=0;j<m;j++)
		{
			peek_queue(czone->D,&dummy,j);

			if( person_selfInoc(&dummy,pathogen,FACE,ENVR,tStep_m))
				dummy.status = INFECTED;

			insert_queue(czone->D,&dummy,j);

		}



		//Cleaning and pathogen half-life die-off
		for (cfom=0;cfom<num_fom;cfom++) //swap for queue to fom...
		{
			//decay rates (specified per hour)
			czone->bioburden[cfom] = (unsigned long) ( (czone->bioburden[cfom])*(pathogen->decay_f) );

			//Cleaning rates are specified /hr in config but converted to /min on initialisation
			if( accept((czone->CR)*tStep_m) )
				zone_clean(czone, cfom);

		}//for each fomite cleaning

	}//for each zone


	//Having gone through ALL zones and updates A,D queues, now let's think about:
	//moving to new zones: Departures from one location can go to another's arrival queue

	for(p=0;p<numZones;p++)
	{
		czone = &zone[p];

		m = numEvents(tStep_m*(czone->L)); //max number of people to leave zone

		i=0;
		while(!isEmpty_queue(czone->D) && i<m )
		{
			dequeue(czone->D,&dummy);

			if(dummy.jumps==0)//leave simualtion-> go to source pool arrivals!
			{
				enqueue(pool[dummy.pool].A,&dummy);

			}else{ //go to new zone (or possibly back to same one in 'arrivals'

				nl= zone_mc(czone);
				dummy.jumps = dummy.jumps -1;
				enqueue(zone[nl].A,&dummy);
			}

			i++;

		} //while
	}//for each zone


}//time

if(LOG_B)
 fclose(fp);

}//eof function



/*MAIN SIMULATION Starting point*/

int main(int argc, char* argv[])
{

	/*File I/O: write to a .csv file*/
	FILE* fp;
	char filename[60];

	bool new_output_flg = true;
	unsigned int i,j,k;

	Hash_t* config;
	config = hash_new(50);
	char buff[MAX_VALUE_SIZE];

	char* key;
	char* value;


	srand(time(0)); //seed random number generator

	if(!(argc==7 || argc==5))
		return errorMsg("4 (or 6) arguments expected (config file, output(.csv), #realisations, logb_flg, {key,value})");



	unsigned int NUM_REALISATIONS = atoi(argv[3]);
	printf("\nNUM_REALISATIONS: %d\n",NUM_REALISATIONS);


	unsigned int LOG_B = atoi(argv[4]);
	printf("\nLOG_BIO: %d\n",LOG_B);


	updateMsg("Reading config file...");

		sprintf(filename,"%s",argv[1]);

		if(!readConfigurationFile(filename, config))
			return 0;


		if(argc==7)
		{
			key = argv[5];
			value = argv[6];

			hash_insert(config,key,value);
		}
		hash_print(config);

	updateMsg("Opening output csv file...");

		sprintf(filename,"%s",argv[2]);

		if( access( filename, F_OK ) == 0 )  //file already exists
				new_output_flg = false;


		fp = fopen(filename,"a");
		if (fp == NULL)
		 return errorMsg("Could not open file for writing output");

		updateMsg(filename);



	Pathogen pathogen;
	if(!pathogen_config(&pathogen,config))
		return errorMsg("Pathogen configuration error");

	pathogen_print(&pathogen);


	updateMsg("Loading zones...");
	if(!hash_lookup_csv(config,"ZONES",0,buff))
		return errorMsg("Number of Zones - error");

	unsigned int NUM_ZONES = atoi(buff);


	updateMsg("Loading pools...");
	if(!hash_lookup_csv(config,"POOLS",0,buff))
		return errorMsg("Number of Pools - error");

	unsigned int NUM_POOLS = atoi(buff);

	//get some simulation parameters
	if(!hash_lookup_csv(config ,"RES_MIN" , 0 ,buff ))
			return errorMsg("RES_MIN configuration error");

	unsigned int resolution_min = atof(buff);


	//get some simulation parameters
	if(!hash_lookup_csv(config ,"DURATION_MIN" , 0 ,buff ))
			return errorMsg("DURATION_MIN configuration error");

	unsigned int duration_min = atof(buff);


	if(duration_min<resolution_min)
		return errorMsg("Simulation duration less than resolution step!");


	float params[4];


	tgParam DEPOSIT;

	for(i=0;i<4;i++)
	{
		if(!hash_lookup_csv(config ,"DEPOSIT" , i ,buff ))
			return errorMsg("DEPOSIT configuration error");

		params[i]= atof(buff);
	}


	DEPOSIT.md = params[0];
	DEPOSIT.sd = params[1];
	DEPOSIT.mn = params[2];
	DEPOSIT.mx = params[3];


	updateMsg("DEPOSIT");
	print_tgParam(&DEPOSIT);



	tgParam PICKUP;

	for(i=0;i<4;i++)
	{
		if(!hash_lookup_csv(config ,"PICKUP" , i ,buff ))
			return errorMsg("PICKUP configuration error");

		params[i]= atof(buff);
	}


	PICKUP.md = params[0];
	PICKUP.sd = params[1];
	PICKUP.mn = params[2];
	PICKUP.mx = params[3];


	updateMsg("PICKUP");
	print_tgParam(&PICKUP);







	tgParam FACE;

	for(i=0;i<4;i++)
	{
		if(!hash_lookup_csv(config ,"FACE" , i ,buff ))
			return errorMsg("FACE configuration error");

		params[i]= atof(buff);
	}


	FACE.md = params[0];
	FACE.sd = params[1];
	FACE.mn = params[2];
	FACE.mx = params[3];


	updateMsg("FACE");
	print_tgParam(&FACE);



	tgParam ENVR;

	for(i=0;i<4;i++)
	{
		if(!hash_lookup_csv(config ,"ENVR" , i ,buff ))
			return errorMsg("ENVR configuration error");

		params[i]= atof(buff);
	}


	ENVR.md = params[0];
	ENVR.sd = params[1];
	ENVR.mn = params[2];
	ENVR.mx = params[3];


	updateMsg("ENVR");
	print_tgParam(&ENVR);



	//Initialise simualtion statistics output

	//averaged over j- realisations..
	struct stat* mystats = (struct stat*)calloc(NUM_POOLS,sizeof(struct stat));



	//Compute algorithm

	updateMsg("Simulating...");
	if(argc==7)
	printf("Parameters: key:%s \t value:%s\n", key,value);

	Person dummy;

	for(j=0;j<NUM_REALISATIONS;j++)
	{
		//for each realisation loop, reload and clear pool/zones...
		//Zone pool[1];

		Zone* pool = malloc(NUM_POOLS*sizeof(Zone));
		Zone* zone = malloc(NUM_ZONES*sizeof(Zone));

		for(i=0;i<NUM_POOLS;i++)
		{
			if(!pool_config(&pool[i], config , i,NUM_ZONES))
			return errorMsg("Pool configuration error");
		}

		for(i=0;i<NUM_ZONES;i++)
		{
			if(!zone_config(&zone[i], config , i ,NUM_ZONES))
				return errorMsg("Zone configuration error");

			if(!hash_lookup_csv(config,"CONTAMINATED",i,buff))
				return errorMsg("location config: CONTAMINATED not specified");

			unsigned int contaminated = atoi(buff);  //0(no) or 1(yes)

			if(contaminated)
				zone_contaminated(&zone[i],pathogen_shed(&pathogen));

		}


		//DATA analysis...
		unsigned long count; //infectious I and newly infected E
		double avg; //temp
		unsigned int N;



	  //  clock_t begin = clock();


		fomite_sim(
				pool,
				NUM_POOLS,
				zone,
				NUM_ZONES,
				&DEPOSIT,&PICKUP,
				&FACE, &ENVR,
				&pathogen,
				resolution_min,
				duration_min,
				filename,LOG_B
		);

				/*	clock_t end = clock();
					double time_spent =0.0;
					time_spent +=(double)(end-begin)/CLOCKS_PER_SEC;
					printf("\n Iteration j:%d \t time elaspsed: is %f seconds", j,time_spent);
				*/

		printf("\r\tProgress:%.0f%%",(double)((j+1)*100)/NUM_REALISATIONS);
		fflush( stdout );


		//done: check each pool

	for(i=0;i<NUM_POOLS;i++)
	{
		//Check that everyone made it out of the initial pool
		if(queue_count(pool[i].D)!=0){
		  warningMsg("sim did not get through entire pool");
			printf("pool:%d",i);

		}

				//updateMsg("Pool/Zone [A,D] queue stats:");
				//stats_queue(pool[i].A);
				//stats_queue(pool[i].D);
	}


		// round everybody up left in different zones to their original pool A
		for(k=0;k<NUM_ZONES;k++)
		{
				Zone* czone = &zone[k];

						//printf("\nZone %d:",k);
						//stats_queue(czone->A);
						//stats_queue(czone->D);

				unsigned long c;
				//check departures queues
				c = queue_count(czone->D);
				for(i=0;i<c;i++)
				{
				    dequeue(czone->D,&dummy);
				    enqueue(pool[dummy.pool].A,&dummy);
				}
				//check arrival queues
				c = queue_count(czone->A);
				for(i=0;i<c;i++)
				{
				    dequeue(czone->A,&dummy);
				    enqueue(pool[dummy.pool].A,&dummy);
				}

		}


		//compute infection rates in each pool

		for(k=0;k<NUM_POOLS;k++)
		{
			N = pool[k].numFomites;  //note: people==fomites in the pool context

			//should all be in A queue of pool!
			count = queue_count(pool[k].A);  //

			if(count!=N)
			errorMsg("Missing people! Some never left the initial pool!");

			mystats[k].val = 0;

			unsigned int tot_jumps_remaining=0;

			for(i=0;i<count;i++)
			{
					peek_queue(pool[k].A,&dummy,i);

					tot_jumps_remaining += dummy.jumps;

					if(dummy.status==INFECTED)
						mystats[k].val++;

			}

			if(tot_jumps_remaining !=0)
			{
			 warningMsg("Not all people made all their jumps!");
			 printf("\tAverage # jumps performed (per person):%f\n",(double)tot_jumps_remaining/N);
			}


		//running averages
			avg = (double)(mystats[k].val)/N;   				//infection rate in the k^th pool
			mystats[k].mu = onlineMean(mystats[k].mu,avg,1+j);  //updated mean (note mystats[k].mu still contain previous)

		}


		//jth-realisation complete - free memory/reset pool/locations
		for(i=0;i<NUM_POOLS;i++)
			zone_close(&pool[i]);

		for(i=0;i<NUM_ZONES;i++)
			zone_close(&zone[i]);

		free(zone);
		free(pool);

	}	//j-realisations



	//Display/write stats to output file
	updateMsg("Final statistics:");


	if(new_output_flg)
	{
		//header
		fprintf(fp,"#Simulation_Output: n= %d\n",NUM_REALISATIONS);

		fprintf(fp,"key,value");

		for(k=0;k<NUM_POOLS;k++)
			fprintf(fp,",CI%d",k);

		fprintf(fp,"\n");
	}


	if(argc==7)
		fprintf(fp,"%s,%s",key,value);  //+ other outputs...
	else
		fprintf(fp,"key,value");

	//normalised onlineSSD to get variance
	for(k=0;k<NUM_POOLS;k++)
	{
			fprintf(fp,",%f",mystats[k].mu);
			printf("risk: %f%%\n",100*mystats[k].mu);
	}

	fprintf(fp,"\n");


	//Clean up heap
	free(mystats);
	hash_delete(config);
	fclose(fp);

	updateMsg("\nProgram complete");
	return 0;
}
