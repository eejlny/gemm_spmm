#ifndef __SPMV_BLOCK_H__
#define __SPMV_BLOCK_H__


#include <spmm.h>


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
		);


#endif//__SPMV_BLOCK_H__
