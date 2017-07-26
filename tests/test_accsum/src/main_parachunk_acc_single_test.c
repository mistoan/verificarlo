//######################################
//## Author: Eric Petit
//## Mail eric.petit@intel.com
//#####################################

	
#include "bench.conf"
#include "all_header.h"
#include "test_macro.h"

int main(int argc, char *argv[])
{
	double c;
	//double X=1.1;
	double *Pinit,*P[TEST];
	int i,i_c,i_n;
	int SIZE=0,SIZEb=0;
	double C[NB_MAX_C];
	int C_EXP[NB_MAX_C];
	unsigned long int N[NB_MAX_N];
	


	initTabC();
	initTabN();	
	
	assert((NB_N-1)*NB_N_STRIDE+NB_N_MIN<=NB_MAX_N);	
	for(i_n=NB_N_MIN;i_n<NB_N*NB_N_STRIDE+NB_N_MIN;i_n+=NB_N_STRIDE) {	
		SIZE=N[i_n];
		
		for(i_c=0;i_c<NB_C;i_c++){	
			if (VERBOSE_TEST>0) printf("------------------------------------------------\n");
		
			//init P as array of 0
			init(&Pinit, SIZE);
		
			//init rand from libc needed by gensum
			if (VERBOSE_TEST>0) printf("---- Generating %lu data with C==%.4e \n",N[i_n],C[i_c]);
		
			if (SIZE<100)
			{
				srand(3);
				GenSum(Pinit, &c, SIZE, C[i_c]);
			}
			else	
				genSum_fromFile(Pinit, &c, SIZE, C_EXP[i_c]);
		
			if (VERBOSE_TEST>0) printf("---- Test done with C=%.4e \n",c);
			if (VERBOSE_TEST>0) printf("------------------------------------------------\n\n");
		
			//------------------------------
                	//----------ParaChunkAcc test --------
                	//------------------------------
                
			double tmp_res=-1010101;

                        Test_macro_WarmUp(AccSumIn,SIZE,tmp_res);
                        printf("=>AccSum=%.17f\n",tmp_res);

			tmp_res=-1010101;
                        Test_macro_WarmUp(NaiveSumIn,SIZE,tmp_res);
                        printf("=>NaiveSum=%.17f\n",tmp_res);
			
			set_NB_ACCUMULATOR(16);
			set_CHUNK_SIZE(100);
                        set_ACCUMULATE_ALG(AccSumIn);
                        set_REDUCT_ALG(AccSumIn);
                	Test_macro_WarmUp(ParaChunkAccIn,SIZE,tmp_res);
			printf("=>ParaChunkAcc_AccSum=%.17f\n",tmp_res);
			
			tmp_res=-1010101;
			set_NB_ACCUMULATOR(16);
			set_CHUNK_SIZE(100);
			set_ACCUMULATE_ALG(NaiveSumIn);
			set_REDUCT_ALG(NaiveSumIn);
		
			Test_macro_WarmUp(ParaChunkAccIn,SIZE,tmp_res);
			printf("   ParaChunkAcc_NaiveSum=%.17f\n",tmp_res);


			tmp_res=-1010101;
                        set_NB_ACCUMULATOR(128);
			set_CHUNK_SIZE(100);
                        set_ACCUMULATE_ALG(NaiveSumIn);
                        set_REDUCT_ALG(NaiveSumIn);
                
                        Test_macro_WarmUp(ParaChunkAccIn,SIZE,tmp_res);
                        printf("   ParaChunkAcc_NaiveSum=%.17f\n",tmp_res);


			tmp_res=-1010101;
			set_NB_ACCUMULATOR(2);
			set_CHUNK_SIZE(1000);
                        set_ACCUMULATE_ALG(NaiveSumIn);
                        set_REDUCT_ALG(NaiveSumIn);

                        Test_macro_WarmUp(ParaChunkAccIn,SIZE,tmp_res);
                        printf("=>ParaChunkAcc_NaiveSum=%.17f\n",tmp_res);

			tmp_res=-1010101;
                        set_NB_ACCUMULATOR(4);
                        set_CHUNK_SIZE(500);
                        set_ACCUMULATE_ALG(NaiveSumIn);
                        set_REDUCT_ALG(NaiveSumIn);

                        Test_macro_WarmUp(ParaChunkAccIn,SIZE,tmp_res);
                        printf("=>ParaChunkAcc_NaiveSum=%.17f\n",tmp_res);

			
			free(Pinit);
		}//end cond loop
	}
	
		
return 0;
}



