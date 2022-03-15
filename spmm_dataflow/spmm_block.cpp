//Copyright (c) [2022] [Jose Nunez-Yanez //(eejlny@bristol.ac.uk)]

#include "spmm_block.h"


u32 spmm_block(
		int ternary,
		DATA_TYPE *values,
		u32       *colIndices,
		u32       *rowPtr,
		DATA_TYPE_X *x,
		u32        no_vectors,

		DATA_TYPE_OUT *y,
		u32        rows,
		u32        cols,
		u32        nnz,
		u32        begin,
		u32        end

		) {

    #pragma SDS resource(1)
	spmm_wrapper(
			ternary,
			rowPtr,

			colIndices,
			colIndices,
			colIndices,
			colIndices,

			values,
			values,
			values,
			values,

			y,
			y,
			y,
			y,

			x,
			no_vectors,

			cols,
			rows,
			nnz,

			begin,
			end,
			rowPtr[begin]


			);

 //   #pragma SDS resource(2)
//	spmm(
//			ternary,
//			rowPtr,
//			colIndices,
//			colIndices,
//			colIndices,
//			colIndices,

//			values,
//			values,
//			values,
//			values,

//			y,
//			y,
//			y,
//			y,

//			x,
//			no_vectors,

//			cols,
//			rows,
//			nnz,

//			begin,
//			end,
//			rowPtr[begin]


//			);


	return 0;


}
