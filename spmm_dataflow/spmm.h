//Copyright (c) [2022] [Jose Nunez-Yanez //(eejlny@bristol.ac.uk)]


#include "ap_int.h"


/*
 * for clock frequency 100 MHZ   II=4
 * for clock frequency 200 MHZ   II=8
 * for clock frequency 300 MHZ   II=10
 */
//const static int II = 4;

const static int II = 1;


const static int NO_HW_THREAD = 4;


#define DTYPE_LENGTH_X 32
#define DTYPE_LENGTH 8//32//8//32
typedef ap_int<DTYPE_LENGTH> DATA_TYPE;
typedef ap_int<DTYPE_LENGTH_X> DATA_TYPE_X;
typedef ap_int<16> DATA_TYPE_OUT;

typedef unsigned int u32;
//const static int ROW_SIZE_MAX        = (1024*64);
//const static int ROW_SIZE_THREAD_MAX = (1024*16);
const static int ROW_SIZE_MAX        = (4096);
const static int ROW_SIZE_THREAD_MAX = (4096);
const static int COL_SIZE_MAX        = (4096);


//const static int ROW_SIZE_MAX        = (1024);
//const static int ROW_SIZE_THREAD_MAX = (1024);
//const static int COL_SIZE_MAX        = (1024);

void spmm_wrapper(
		ap_uint<2> ternary,
		u32 *rowPtr,

		u32 *columnIndex_0,
		u32 *columnIndex_1,

		u32 *columnIndex_2,
		u32 *columnIndex_3,





		DATA_TYPE *values_0,
		DATA_TYPE *values_1,

		DATA_TYPE *values_2,
		DATA_TYPE *values_3,





		DATA_TYPE_OUT *y_0,
		DATA_TYPE_OUT *y_1,

		DATA_TYPE_OUT *y_2,
		DATA_TYPE_OUT *y_3,



		DATA_TYPE_X *x,
		u32        no_vectors,

		u32 col_size,
		u32 row_size,
		u32 nnz,

		u32 begin,
		u32 end,


		u32 first_rowPrt_value
		);



void spmm(
		ap_uint<2> ternary,
		u32 *rowPtr,

		u32 *columnIndex_0,
		u32 *columnIndex_1,

		u32 *columnIndex_2,
		u32 *columnIndex_3,





		DATA_TYPE *values_0,
		DATA_TYPE *values_1,

		DATA_TYPE *values_2,
		DATA_TYPE *values_3,





		DATA_TYPE_OUT *y_0,
		DATA_TYPE_OUT *y_1,

		DATA_TYPE_OUT *y_2,
		DATA_TYPE_OUT *y_3,



		DATA_TYPE_X *x,
		u32        nv,

		u32 col_size,
		u32 row_size,
		u32 nnz,

		u32 begin,
		u32 end,


		u32 first_rowPrt_value
		);



#endif
