//#######################################
//## Author: Eric Petit
//## Mail eric.petit@intel.com
//#######################################

#include "all_header.h"


double NaiveSumIn(double*, unsigned int );

double NaiveSum(double *p, unsigned int n) {
  double *q=NULL, r=0;

  q= (double *)malloc(sizeof(double)*n);
  memcpy(q, p, sizeof(double)*n);
  r = NaiveSumIn(q, n);
  free(q);
  return r;
}

 
double NaiveSumIn(double *p, unsigned int n) {

  double res=0;
  unsigned int i=0;
  for(i=0; i<n; i++)
	res+=p[i];

  return res;
}


    
