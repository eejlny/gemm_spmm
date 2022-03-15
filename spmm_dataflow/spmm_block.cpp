/*Copyright (c) [2021] [Jose Nunez-Yanez (eejlny@bristol.ac.uk)]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
===============================================================================
* This file has been written at University of Bristol
* for the HOPWARE/MINET project
*
* 
* author    : Jose Nunez-Yanez eejlny@bristol.ac.uk
* date      : 1 October 2021
*/

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
