/********************************************************************************
 *                                                                              *
 *  This file is part of Verificarlo.                                           *
 *                                                                              *
 *  Copyright (c) 2015                                                          *
 *     Universite de Versailles St-Quentin-en-Yvelines                          *
 *     CMLA, Ecole Normale Superieure de Cachan                                 *
 *                                                                              *
 *  Verificarlo is free software: you can redistribute it and/or modify         *
 *  it under the terms of the GNU General Public License as published by        *
 *  the Free Software Foundation, either version 3 of the License, or           *
 *  (at your option) any later version.                                         *
 *                                                                              *
 *  Verificarlo is distributed in the hope that it will be useful,              *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of              *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               *
 *  GNU General Public License for more details.                                *
 *                                                                              *
 *  You should have received a copy of the GNU General Public License           *
 *  along with Verificarlo.  If not, see <http://www.gnu.org/licenses/>.        *
 *                                                                              *
 ********************************************************************************/


// Changelog:
//
// 2015-05-20 replace random number generator with TinyMT64. This
// provides a reentrant, independent generator of better quality than
// the one provided in libc.
//
// 2015-10-11 New version based on quad floating point type to replace MPFR until
// required MCA precision is lower than quad mantissa divided by 2, i.e. 56 bits
//
// 2015-16-11 New version using double precision for single precision operation
//
// 2016-07-14 start the new random rounding backend for x86_64 architecture

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "libmca-rdround.h"
#include "../vfcwrapper/vfcwrapper.h"
#include "../common/tinymt64.h"
#include "../common/mca_const.h"


#include <fenv.h>
#pragma STDC FENV_ACCESS ON


static int 	MCALIB_OP_TYPE 		= MCAMODE_IEEE;
static int 	MCALIB_T		    = 53;

//possible op values
#define MCA_ADD 1
#define MCA_SUB 2
#define MCA_MUL 3
#define MCA_DIV 4

#define PLUS_INF_RD 0
#define MINUS_INF_RD 1

static float _mca_sbin(float a, float b, int qop);

static double _mca_dbin(double a, double b, int qop);

/******************** MCA CONTROL FUNCTIONS *******************
* The following functions are used to set virtual precision and
* MCA mode of operation.
***************************************************************/

static int _set_mca_mode(int mode){
	//useless for rd_round
	MCALIB_OP_TYPE = mode;
	return 0;
}

static int _set_mca_precision(int precision){
	//useless for rd_round
	MCALIB_T = precision;
	return 0;
}

/******************** MCA RANDOM FUNCTIONS ********************
* The following functions are used to calculate the random
* perturbations used for MCA
***************************************************************/

/* random generator internal state */
tinymt64_t random_state;

static double _mca_rand(void) {
	/* Returns a random double in the (0,1) open interval */
	return tinymt64_generate_doubleOO(&random_state);
}



static bool set_random_rounding() {

//	if (MCALIB_OP_TYPE == MCAMODE_IEEE) {
//		fprintf(stderr, "IEEE is not a valid mode for random rounding\n");
//	}
	double d_rand=_mca_rand();
	uint64_t u_rand = *((uint64_t*) &(d_rand));
	return (bool) (u_rand&&0xFFFFFFFE);
	
}

static void _mca_seed(void) {
	const int key_length = 3;
	uint64_t init_key[key_length];
	struct timeval t1;
	gettimeofday(&t1, NULL);

	/* Hopefully the following seed is good enough for Montercarlo */
	init_key[0] = t1.tv_sec;
	init_key[1] = t1.tv_usec;
	init_key[2] = getpid();

	tinymt64_init_by_array(&random_state, init_key, key_length);
	
	//set the rounding mode toward +inf
	fesetround(FE_UPWARD);
}

/******************** MCA ARITHMETIC FUNCTIONS ********************
* The following set of functions perform the MCA operation. Operands
* are first converted to quad  format (GCC), inbound and outbound
* perturbations are applied using the _mca_inexact function, and the
* result converted to the original format for return
*******************************************************************/

// perform_bin_op: applies the binary operator (op) to (a) and (b)
// and stores the result in (res)
#define perform_bin_op_plus_inf(op, res, a, b)                               \
    switch (op){                                                    \
    case MCA_ADD: res=(a)+(b); break;                               \
    case MCA_MUL: res=(a)*(b); break;                               \
    case MCA_SUB: res=(a)-(b); break;                               \
    case MCA_DIV: res=(a)/(b); break;                               \
    default: perror("invalid operator in mcaquad.\n"); abort();     \
	};

#define perform_bin_op_minus_inf(op, res, a, b)                               \
    switch (op){                                                    \
    case MCA_ADD: res=-((-a)+(-b)); break;                               \
    case MCA_MUL: res=-((a)*(-b)); break;                               \
    case MCA_SUB: res=-((-a)-(-b)); break;                               \
    case MCA_DIV: res=-((a)/(-b)); break;                               \
    default: perror("invalid operator in mcaquad.\n"); abort();     \
        };




static inline float _mca_sbin(float a, float b,const int  sop) {

    float res=0;
    bool round_switch=set_random_rounding();
    if (round_switch==PLUS_INF_RD){
    	 perform_bin_op_plus_inf(sop, res, a, b);
    }else{
    	 perform_bin_op_minus_inf(sop, res, a, b);
    }
    return res;
}

static inline double _mca_dbin(double a, double b, const int dop) {
	
    double res=0;
    bool round_switch=set_random_rounding();
    if (round_switch==PLUS_INF_RD){
    	perform_bin_op_plus_inf(dop, res, a, b);
    }else{
    	 perform_bin_op_minus_inf(dop, res, a, b);
    }
    return res;

}

/************************* FPHOOKS FUNCTIONS *************************
* These functions correspond to those inserted into the source code
* during source to source compilation and are replacement to floating
* point operators
**********************************************************************/

static float _floatadd(float a, float b) {
	//return a + b
	return _mca_sbin(a, b, MCA_ADD);
}

static float _floatsub(float a, float b) {
	//return a - b
	return _mca_sbin(a, b, MCA_SUB);
}

static float _floatmul(float a, float b) {
	//return a * b
	return _mca_sbin(a, b, MCA_MUL);
}

static float _floatdiv(float a, float b) {
	//return a / b
	return _mca_sbin(a, b, MCA_DIV);
}


static double _doubleadd(double a, double b) {
	//return a + b
	return _mca_dbin(a, b, MCA_ADD);
}

static double _doublesub(double a, double b) {
	//return a - b
	return _mca_dbin(a, b, MCA_SUB);
}

static double _doublemul(double a, double b) {
	//return a * b
	return _mca_dbin(a, b, MCA_MUL);
}

static double _doublediv(double a, double b) {
	//return a / b
	return _mca_dbin(a, b, MCA_DIV);
}


struct mca_interface_t rdround_mca_interface = {
	_floatadd,
	_floatsub,
	_floatmul,
	_floatdiv,
	_doubleadd,
	_doublesub,
	_doublemul,
	_doublediv,
	_mca_seed,
	_set_mca_mode,
	_set_mca_precision
};
