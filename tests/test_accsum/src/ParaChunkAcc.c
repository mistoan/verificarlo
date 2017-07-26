//#######################################
//## Author: Eric Petit
//## Mail eric.petit@intel.com
//#######################################

#include "all_header.h"

//rd robin accumulator usage is fair since the input vector are unformly randomly generated
// CHUNK_SIZE: the vector to sum are split in chunk of CHUNK_SIZE
unsigned int CHUNK_SIZE=100;
// NB_ACCUMULATOR: the number of intermediate accumulator collecting the sum; in a parallel implementation it correspond to each thread local result.
unsigned int NB_ACCUMULATOR=16;
// ACCUMULATE_ALG: the algorithm used to do the accumulation in all chunks
double (*ACCUMULATE_ALG)(double*, unsigned int)=AccSumIn;
// REDUCT_ALG: the reduction algorithm used to sum the accumulator contribution to the final sum
double (*REDUCT_ALG)(double*, unsigned int)=AccSumIn;



int set_ACCUMULATE_ALG(double (*f_pointer)(double*, unsigned int)){
return 0;
}
int set_REDUCT_ALG(double (*f_pointer)(double*, unsigned int)){
return 0;
}
int set_NB_ACCUMULATOR(int nb_acc){
return 0;
}
int set_CHUNK_SIZE(int chunck_size){
return 0;
}

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
  assert(NB_ACCUMULATOR>=1);

  double res=0;
  double *Acc=(double *)malloc(NB_ACCUMULATOR*sizeof(double));
  
  int i=0, j=0;//, k=0;
  unsigned int tail_size=0;

for(i=0;i<NB_ACCUMULATOR;i++){
	Acc[i]=0;
}

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


    
