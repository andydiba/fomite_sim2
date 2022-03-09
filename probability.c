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
#include <stdlib.h>
#include <math.h>


#include "probability.h"


//returns TRUE with probability 'p'
bool accept(double p){

	double pb = uniform(); //[0,1]
  
  	if(pb<p)
  	 return true;//accept

	return false;//reject
}
/*

double logNorm(double x, double mu, double sd)
{
	
	double A,B;
	
	if(x<=0 || sd<=0)
	return 0.0;
	
	A = 1/( (x*sd)*SQRT2PI );
	B = log(x) -mu;
	B=-(B*B)/(2*sd*sd);
	
	return A*exp(B);
	
	
	
}	*/


//Use box-muller + exponential transformation
double samplelogNorm(double mu, double sd, double mx){
	
	if(sd<=0)
	{
		printf("\nError: sampleLogNorm sd value <=0\n");
		return -1;
	}
	
	double u1,u2,z,X;
	
	u1 = uniform();
	u2 = uniform();
	
	// ~N(0,1)
	z = sqrt(-2*log(u1))*cos(2*PI*u2);  
	
	X = exp(mu+sd*z);
	X = (X>mx) ? mx:X ;  //truncate if required
	
	return X;
}

/*
double samplelogNorm(double mu, double sd, double mx)
{
	
    unsigned int max_count = 1000;
    unsigned int count=0;

	double mode = exp(mu-(sd*sd));
        
	double maxPDF,x,y,gx,r1,r2;

	if(sd<=0)
	{
		printf("\nError: sampleLogNorm sd value <=0\n");
		return -1;
	}


    maxPDF = logNorm(mode,mu,sd);
        
        
    double xdom = mx-0;
        
	while(count<max_count)
	{

	  r1 = uniform();
	  r2 = uniform();

	  x = 0+xdom*r1;
	  y = maxPDF*r2;

	  gx = logNorm(x,mu,sd); 
	  
	  if(y <= gx)
		  return x;
					 
	   count++;
	}
  
	if(count==max_count)
	   printf("\nOops! max count reached!\n");

	return -1;
	
}*/


/*
//Error function : needed for truncated gaussian computation
double erf(double x){

	double a1 =  0.254829592;
   	double a2 = -0.284496736;
   	double a3 =  1.421413741;
   	double a4 = -1.453152027;
   	double a5 =  1.061405429;
   	double p  =  0.3275911;
  
  	//Save the sign of x
    	int sgn = 1;
    	if (x < 0)
        	sgn = -1;
   
        x = fabs(x);
  
  
  	//A&S formula 7.1.26
   	double t = 1.0/(1.0 + p*x);
   	double y = 1.0 - (((((a5*t + a4)*t) + a3)*t + a2)*t + a1)*t*exp(-x*x);

    	return sgn*y;

}

*/

void print_tgParam(tgParam* tg){
	
	printf("\n tgParam \n\t (%.3f,%.3f,%.3f,%.3f)\n",
		tg->md,tg->sd,tg->mn,tg->mx);
}

/*
//Truncated (Gaussian) Normal Distribution
double normTrunc(double x,tgParam* P){


	   double md = P->md;
	   double sd = P->sd;
	   double mn = P->mn;
	   double mx = P->mx;


    
            if(x<mn || x>mx)
               return 0;
            else if (sd == 0){
             	
		return (x==md);
		
            }else{  
              
              double X1 = (x-md)/sd;
              double Xb = (mx-md)/sd;
              double Xa = (mn-md)/sd;
              
              double PHI1 = (1/sqrt(2*PI))*exp(-0.5*pow(X1,2));
              double PHIb = 0.5*(1+ erf(Xb/sqrt(2)) );
              double PHIa = 0.5*(1+ erf(Xa/sqrt(2)) );
              
              return (1/sd)*PHI1/(PHIb - PHIa);
            }  
            
}*/

//Box-Muller algorithm + truncation:
//->involves adding a proportion of uniform data 'underneath' the Gaussian to assure proper relative likelihoods
double sampleNormT(tgParam* P){
	
	double md = P->md;
	double sd = P->sd;
	double mn = P->mn;
	double mx = P->mx;
	
	if (sd == 0){
        if (md<=mx && md>=mn)
		{
			return md;
		}else{
			
			return 0;
		}	
     }  
	
	double a = (mn-md)/sd;
	double b = (mx-md)/sd;
	
	double u1,u2,z;
	
	u1 = uniform();
	u2 = uniform();
	
	// ~N(0,1)
	z = sqrt(-2*log(u1))*cos(2*PI*u2);  
	
	//Outside range? Convert to ~U(a,b)
	if(z>b || z<a){  
		u1 = uniform();  //generate new uniform (or recycle previous...?)
		z = u1*(b-a) + a;
	}
	
	z = z*sd + md;
	
	return z;
	
}


//Sample from trunc norm distribution using 'rejection' sampling method


/*double sampleNormT(tgParam* P){
  
        bool done = false;
        unsigned int max_count = 1000;
        unsigned int count=0;


	double md = P->md;
	double sd = P->sd;
	double mn = P->mn;
	double mx = P->mx;
        
	double maxPDF,x,y,gx,r1,r2;


	if (sd == 0){
              if (md<=mx && md>=mn)
		{	return md;}

		return 0;	
        }  


        if (md>mx)
             maxPDF = normTrunc(mx,P);
        else if (md<mn)
             maxPDF = normTrunc(mn,P);
        else
             maxPDF = normTrunc(md,P);
        
        
        double xdom = mx-mn;
        
        while(!done && count<max_count)
        {
	
	  r1 = uniform();
	  r2 = uniform();

          x = mn+xdom*r1;
          y = maxPDF*r2;

          gx = normTrunc(x,P); 
          
          if(y <= gx)
              done = true;
                         
           count++;
        }
  
        if(count==max_count)
           printf("\nOops! max count reached!\n");

	return x;
        
}
*/

//generates samples from Poisson distribution using invCDF method.
//Note: very slow for large L
unsigned int numEvents(double L){
	
	unsigned int k;
	double t,r;	

	 if(L==0)
         { 
		k = 0;
    	 }else{
  
              t=0; //time of event
              k=0; //number of events (event index)

              //S=[]; //arrival times
	
 	         while(t<1) //number of event in '1 unit' of time e.g. called every simulation step
		  {
 	            r = uniform();
 	            t = t - log(r)/L;
 	            k++;
 	           //S=[S; t];
 	         }  
                 
 	         //cut off last iteration
 	         //S=S(1:end-1);
 	         k=k-1;
	 }
	return k;

}


//n>0
double onlineMean(double xnm1,double x, unsigned int n)
{
	return ((n-1)*xnm1 + x)/n;
	
}
//n>0
//Welford's online algorithm
double onlineSSD(double M2nm1, double xn,double xnm1,double x)
{
	
	return M2nm1 + ( (x-xnm1)*(x-xn) );
	
	
}
