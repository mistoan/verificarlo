//#######################################
//## Author: Eric Petit
//## Mail eric.petit@intel.com
//#######################################

//rd robin accumulator usage is fair since the input vector are unformly randomly generated
// CHUNK_SIZE: the vector to sum are split in chunk of CHUNK_SIZE
#define CHUNK_SIZE 100
// NB_ACCUMULATOR: the number of intermediate accumulator collecting the sum; in a parallel implementation it correspond to each thread local result.
#define NB_ACCUMULATOR 16
// ACCUMULATE_ALG: the algorithm used to do the accumulation in all chunks
#define ACCUMULATE_ALG AccSumIn
// REDUCT_ALG: the reduction algorithm used to sum the accumulator contribution to the final sum
#define REDUCT_ALG AccSumIn


#include "all_header.h"


double ParaChunkAccIn(double*, unsigned int );

double ParaChunkAcc(double *p, unsigned int n) {
  double *q=NULL, r=0;

  q= (double *)malloc(sizeof(double)*n);
  memcpy(q, p, sizeof(double)*n);
  r = ParaChunkAccIn(q, n);
  free(q);
  return r;
}

 
double ParaChunkAccIn(double *p, unsigned int n) {
  double res=0, Acc[NB_ACCUMULATOR]={0};
  int i=0, j=0, k=0;
  unsigned int tail_size=0;

if(n>CHUNK_SIZE)
  for(i=0,j=0; i<n ;i+=CHUNK_SIZE, j=(j+1)%NB_ACCUMULATOR)
	Acc[j]=ACCUMULATE_ALG(&p[i], CHUNK_SIZE);
	//for(k=0; k<CHUNK_SIZE; k++)
	//	Acc[j]+=p[i+k];

  tail_size=n%CHUNK_SIZE;
  //j has been updated and point to the correct accumulator

  Acc[j]=ACCUMULATE_ALG(&p[n-tail_size], tail_size);

  res=REDUCT_ALG(Acc, NB_ACCUMULATOR);

  return res;
}


    
