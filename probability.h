#ifndef PROBABILITY_H
#define PROBABILITY_H

#include <stdbool.h>

#define PI 3.141592653589793
#define SQRT2PI 2.5006

#define uniform() ( (double)rand()/RAND_MAX )


typedef struct MarkovChain{
	//unsigned int* state;
	double* prb;
	unsigned int num_states;
}MarkovChain;


//Truncated Gaussian Parameters
typedef struct tgParam{

double md; //mode
double sd; //standard deviation
double mn; //min
double mx; //max

}tgParam;

void print_tgParam(tgParam*);

//double erf(double );

bool accept(double );

double sampleNormT(tgParam*);
double samplelogNorm(double, double,double);


unsigned int numEvents(double );


double onlineMean(double,double, unsigned int);
double onlineSSD(double,double,double,double);

#endif
