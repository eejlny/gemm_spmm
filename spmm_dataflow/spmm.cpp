//Copyright (c) [2022] [Jose Nunez-Yanez //(eejlny@bristol.ac.uk)]

#include <hls_stream.h>
#include "spmm.h"



void spmm_kernel(
		ap_uint<2> ternary,
		u32 *rowSize_local_rs,
		u32 *rowSize_local_nrs,
		u32 *columnIndex,
		DATA_TYPE *values,
		DATA_TYPE_OUT *y,
		DATA_TYPE *x_local,
		u32 row_size,
		u32 nnz,
		u32 new_nnz
) {

#pragma HLS DATAFLOW
	u32 row_size_tmp=0;
	u32 j = 0;

	DATA_TYPE_OUT y_tmp = 0;
	u32 row_counter = 0;

hls::stream<DATA_TYPE>       values_fifo;
#pragma HLS STREAM variable=values_fifo depth=4 dim=1
hls::stream<u32>             col_indices_fifo;
#pragma HLS STREAM variable=col_indices_fifo depth=4 dim=1
hls::stream<DATA_TYPE_OUT>       y_fifo;
#pragma HLS STREAM variable=y_fifo depth=4 dim=1


	R_LOOP: for (u32 i = 0; i < nnz; i+=1) {
#pragma HLS loop_tripcount min=409 max=409
#pragma HLS pipeline
		values_fifo << values[i];
	}
	for (u32 i = 0; i < nnz; i+=1) {
#pragma HLS loop_tripcount min=409 max=409
#pragma HLS pipeline
		col_indices_fifo << columnIndex[i];
	}



	u32 row_size_remains = 0;



	 P_LOOP: for (u32 i = 0; i < new_nnz; i+=II) {
#pragma HLS loop_tripcount min=100 max=100
#pragma HLS pipeline
		if (row_size_tmp == 0) {
			row_size_tmp = rowSize_local_nrs[j];
			row_size_remains = 0;
			y_tmp = 0;
			row_counter	= rowSize_local_rs[j++];
		}

		DATA_TYPE_OUT y_local = 0;
		for (u32 p = 0; p < II; p++) {
			row_size_remains++;
			if (row_size_remains > row_counter) {
				y_local +=  0;
			} else {
				DATA_TYPE v = values_fifo.read();
				u32   ci = col_indices_fifo.read();
				//y_local +=  v*x_local[ci];
				 if(ternary == 0)
				 {
					 	 for(int z = 0; z < DTYPE_LENGTH; z+=8) {
					 		 	 ap_int<8> v_val = v.range(z+7,z);
					 		 	 	y_local +=  v_val*x_local[ci].range(z+7,z);
					 		 	 	//std::cout << "y_local  " << y_local << std::endl;
					 	 }
				 }
				 else if (ternary == 1)
				 {
						for(int z = 0; z < DTYPE_LENGTH; z+=2) {

							    ap_int<2> v_val = v.range(z+1,z);
							    ap_int<2> x_temp = x_local[ci].range(z+1,z);
		    			  			ap_int<2> C_val;
					  		    //ap_int<8> v_val = v.range(z+1,z);
					  		    //ap_int<8> x_temp = x_local[ci].range(z+1,z);
					  			//ap_int<8> C_val;
							    C_val = v_val*x_temp;
								y_local += C_val;



		    				}
				 }
				 else
				 {
						for(int z = 0; z < DTYPE_LENGTH; z+=4) {

							    ap_int<4> v_val = v.range(z+3,z);
							    ap_int<4> x_temp = x_local[ci].range(z+3,z);
					  			ap_int<4> C_val;
					  		    //ap_int<8> v_val = v.range(z+1,z);
					  		    //ap_int<8> x_temp = x_local[ci].range(z+1,z);
					  			//ap_int<8> C_val;
							    C_val = v_val*x_temp;
								y_local += C_val;
						}
				 }

			}
		} //p loop

		y_tmp += y_local;
		row_size_tmp-=II;
		//std::cout << "y_tmp  " << y_tmp << std::endl;

		if (row_size_tmp == 0) {
			y_fifo << y_tmp;

		}
	}

	 W_LOOP: for (u32 i = 0; i < row_size; i+=1) {
#pragma HLS loop_tripcount min=6 max=6
#pragma HLS pipeline
		y[i] = y_fifo.read();
	}
}



#pragma SDS data zero_copy(rowPtr[0:(row_size+1)])
//#pragma SDS data sys_port(rowPtr:ps_e_S_AXI_HPC0_FPD)
//----------------------------------------------------------------
#pragma SDS data zero_copy(columnIndex_0[0:(nnz)])
#pragma SDS data sys_port(columnIndex_0:ps7_S_AXI_HP0)
//#pragma SDS data sys_port(columnIndex_0:ps_e_S_AXI_HPC0_FPD)

#pragma SDS data zero_copy(columnIndex_1[0:(nnz)])
#pragma SDS data sys_port(columnIndex_1:ps7_S_AXI_HP1)
//#pragma SDS data sys_port(columnIndex_1:ps_e_S_AXI_HPC0_FPD)

#pragma SDS data zero_copy(columnIndex_2[0:(nnz)])
#pragma SDS data sys_port(columnIndex_2:ps7_S_AXI_HP2)
//#pragma SDS data sys_port(columnIndex_2:ps_e_S_AXI_HPC1_FPD)

#pragma SDS data zero_copy(columnIndex_3[0:(nnz)])
#pragma SDS data sys_port(columnIndex_3:ps7_S_AXI_HP3)
//#pragma SDS data sys_port(columnIndex_3:ps_e_S_AXI_HPC1_FPD)



//----------------------------------------------------------------
#pragma SDS data zero_copy(values_0[0:(nnz)])
#pragma SDS data sys_port(values_0:ps7_S_AXI_HP0)

#pragma SDS data zero_copy(values_1[0:(nnz)])
#pragma SDS data sys_port(values_1:ps7_S_AXI_HP1)
//#pragma SDS data sys_port(values_1:ps_e_S_AXI_HPC0_FPD)

#pragma SDS data zero_copy(values_2[0:(nnz)])
#pragma SDS data sys_port(values_2:ps7_S_AXI_HP2)
//#pragma SDS data sys_port(values_2:ps_e_S_AXI_HPC1_FPD)

#pragma SDS data zero_copy(values_3[0:(nnz)])
#pragma SDS data sys_port(values_3:ps7_S_AXI_HP3)
//#pragma SDS data sys_port(values_3:ps_e_S_AXI_HPC1_FPD)



//----------------------------------------------------------------
#pragma SDS data zero_copy(y_0[0:(row_size)])
#pragma SDS data sys_port(y_0:ps7_S_AXI_HP0)
//#pragma SDS data sys_port(y_0:ps_e_S_AXI_HPC0_FPD)

#pragma SDS data zero_copy(y_1[0:(row_size)])
#pragma SDS data sys_port(y_1:ps7_S_AXI_HP1)
//#pragma SDS data sys_port(y_1:ps_e_S_AXI_HPC0_FPD)

#pragma SDS data zero_copy(y_2[0:(row_size)])
#pragma SDS data sys_port(y_2:ps7_S_AXI_HP2)
//#pragma SDS data sys_port(y_2:ps_e_S_AXI_HPC1_FPD)

#pragma SDS data zero_copy(y_3[0:(row_size)])
#pragma SDS data sys_port(y_3:ps7_S_AXI_HP3)
//#pragma SDS data sys_port(y_3:ps_e_S_AXI_HPC1_FPD)


//----------------------------------------------------------------
#pragma SDS data zero_copy(x[0:(col_size/4)])
#pragma SDS data sys_port(x:ps7_S_AXI_ACP)


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


		) {
#pragma HLS DATAFLOW

	u32 rowSizeNew_local_rs[NO_HW_THREAD][ROW_SIZE_THREAD_MAX];
	u32 rowSizeNew_local_nrs[NO_HW_THREAD][ROW_SIZE_THREAD_MAX];
    #pragma HLS ARRAY_PARTITION variable=rowSizeNew_local_rs complete dim=1
    #pragma HLS ARRAY_PARTITION variable=rowSizeNew_local_rs cyclic  factor=4 dim=2

    #pragma HLS ARRAY_PARTITION variable=rowSizeNew_local_nrs complete dim=1
    #pragma HLS ARRAY_PARTITION variable=rowSizeNew_local_nrs cyclic factor=4 dim=2

	//================================================

	DATA_TYPE x_local[NO_HW_THREAD][COL_SIZE_MAX];
#pragma HLS ARRAY_PARTITION variable=x_local complete dim=1
#pragma HLS ARRAY_PARTITION variable=x_local cyclic factor=4 dim=2
	//================================================

	u32 row_size_threads[NO_HW_THREAD];
	u32 values_offset_threads[NO_HW_THREAD];
	u32 row_offset_threads[NO_HW_THREAD];
	u32 nnz_threads[NO_HW_THREAD];
	u32 new_nnz_threads[NO_HW_THREAD];
#pragma HLS ARRAY_PARTITION variable=row_size_threads complete dim=1
#pragma HLS ARRAY_PARTITION variable=values_offset_threads complete dim=1
#pragma HLS ARRAY_PARTITION variable=row_offset_threads complete dim=1
#pragma HLS ARRAY_PARTITION variable=nnz_threads complete dim=1
#pragma HLS ARRAY_PARTITION variable=new_nnz_threads complete dim=1


	//=======================================================

	{



		u32 ideal_nnz = nnz/NO_HW_THREAD;

		for ( u32 i = 0; i < NO_HW_THREAD; i++) {
	#pragma HLS UNROLL
			row_size_threads[i] = 0;
			nnz_threads[i] = 0;
			new_nnz_threads[i] = 0;
		}

		u32 nrs = 0;
		u32 new_nnz = 0;
		u32 j = 0;
		u32 prev_index = first_rowPrt_value;
		u32 k = 0;

	for (u32 i = 0; i < end-begin; i++) {
    #pragma HLS loop_tripcount min=6 max=6
	#pragma HLS PIPELINE

			u32 current_index= rowPtr[i+begin+1];
			u32 rs = (current_index-prev_index);

			if (rs  == 0) {
				nrs = II;
				new_nnz = II;
			} else if (rs%II == 0) {
				nrs = rs;
				new_nnz = 0;
			} else {
				nrs = rs + (II-rs%II);
				new_nnz = (II-rs%II);
			}

			u32 t = nnz_threads[j] + rs;
			prev_index = current_index;

			if (t < ideal_nnz) {
				nnz_threads[j] = t;
			} else {
				if (j+1 < NO_HW_THREAD) {
					j++;
					k=0;
					nnz_threads[j] = rs;

				} else {

					nnz_threads[j] = t;

				}
			}
			row_size_threads[j]++;
			new_nnz_threads[j] += new_nnz;
			rowSizeNew_local_rs[j][k]  = rs;
			rowSizeNew_local_nrs[j][k] = nrs;
			k++;
		}


		for ( u32 i = 0; i < NO_HW_THREAD; i++) {
	#pragma HLS UNROLL

			new_nnz_threads[i] += nnz_threads[i];
		}
		values_offset_threads[0] = 0;
		row_offset_threads[0] = 0;
		for ( u32 i = 1; i < NO_HW_THREAD; i++) {
	#pragma HLS UNROLL
			values_offset_threads[i] = values_offset_threads[i-1]+nnz_threads[i-1];
			row_offset_threads[i] = row_offset_threads[i-1] + row_size_threads[i-1];
		}
	}

	//for (u32 i=0; i<(col_size); i++){
	//#pragma HLS pipeline
	//		for (u32 j=0; j<(NO_HW_THREAD); j++){
	//			x_local[j][i] = x[i];
	//		}
	//	}


//=======================================================================================

//LOOP_C:	for (u32 nv=0; nv < no_vectors; nv++){
//#pragma HLS loop_tripcount min=1 max=1
//u32 nv=0;
		//for (u32 i=0; i<col_size; i+=4){
		//#pragma HLS pipeline
		//		for (u32 j=0; j<(NO_HW_THREAD); j++){
		//			DATA_TYPE_X x_wide = x[nv*(col_size>>2) + (i>>2)];
		//			x_local[j][i] = x_wide.range(7,0);
		//			x_local[j][i+1] = x_wide.range(15,8);
		//			x_local[j][i+2] = x_wide.range(23,16);
		//			x_local[j][i+3] = x_wide.range(31,24);
		//		}
		//	}


		//if(!ternary)
		//{
			for (u32 i=0; i<(col_size>>2); i++){
#pragma HLS loop_tripcount min=168 max=168
				#pragma HLS pipeline
						for (u32 j=0; j<(NO_HW_THREAD); j++){
							DATA_TYPE_X x_wide = x[nv*(col_size>>2) + i];
							x_local[j][i*4] = x_wide.range(7,0);
							x_local[j][i*4+1] = x_wide.range(15,8);
							x_local[j][i*4+2] = x_wide.range(23,16);
							x_local[j][i*4+3] = x_wide.range(31,24);
						}
					}
		//}
		//else
		//{
		//	for (u32 i=0; i<(col_size>>2); i++){
		//		#pragma HLS pipeline
		//				for (u32 j=0; j<(NO_HW_THREAD); j++){
		//					DATA_TYPE_X x_wide = x[nv*(col_size>>2) + i];
		//					x_local[j][i*4] = x_wide.range(7,0);
		//					x_local[j][i*4+1] = x_wide.range(15,8);
		//					x_local[j][i*4+2] = x_wide.range(23,16);
		//					x_local[j][i*4+3] = x_wide.range(31,24);
		//				}
		//			}

		//}



//=======================================================================================

			u32 i;

			i = 0;
			spmm_kernel(
					ternary,
					rowSizeNew_local_rs[i],
					rowSizeNew_local_nrs[i],
					columnIndex_0 + first_rowPrt_value + values_offset_threads[i],
					values_0 + first_rowPrt_value + values_offset_threads[i],
					y_0 + begin + nv*row_size + row_offset_threads[i],
					x_local[i],
					row_size_threads[i],
					nnz_threads[i],
					new_nnz_threads[i]
			);

			i = 1;
			spmm_kernel(
					ternary,
					rowSizeNew_local_rs[i],
					rowSizeNew_local_nrs[i],
					columnIndex_1 + first_rowPrt_value + values_offset_threads[i],
					values_1 + first_rowPrt_value + values_offset_threads[i],
					y_1 + begin + nv*row_size + row_offset_threads[i],
					x_local[i],
					row_size_threads[i],
					nnz_threads[i],
					new_nnz_threads[i]
			);

			i = 2;
			spmm_kernel(
					ternary,
					rowSizeNew_local_rs[i],
					rowSizeNew_local_nrs[i],
					columnIndex_2 + first_rowPrt_value + values_offset_threads[i],
					values_2 + first_rowPrt_value + values_offset_threads[i],
					y_2 + begin + nv*row_size + row_offset_threads[i],
					x_local[i],
					row_size_threads[i],
					nnz_threads[i],
					new_nnz_threads[i]
			);

			i = 3;
			spmm_kernel(
					ternary,
					rowSizeNew_local_rs[i],
					rowSizeNew_local_nrs[i],
					columnIndex_3 + first_rowPrt_value + values_offset_threads[i],
					values_3 + first_rowPrt_value + values_offset_threads[i],
					y_3 + begin + nv*row_size + row_offset_threads[i],
					x_local[i],
					row_size_threads[i],
					nnz_threads[i],
					new_nnz_threads[i]
			);
//		}


}


#pragma SDS data zero_copy(rowPtr[0:(row_size+1)])
//#pragma SDS data sys_port(rowPtr:ps_e_S_AXI_HPC0_FPD)
//----------------------------------------------------------------
#pragma SDS data zero_copy(columnIndex_0[0:(nnz)])
#pragma SDS data sys_port(columnIndex_0:ps7_S_AXI_HP0)
//#pragma SDS data sys_port(columnIndex_0:ps_e_S_AXI_HPC0_FPD)

#pragma SDS data zero_copy(columnIndex_1[0:(nnz)])
#pragma SDS data sys_port(columnIndex_1:ps7_S_AXI_HP1)
//#pragma SDS data sys_port(columnIndex_1:ps_e_S_AXI_HPC0_FPD)

#pragma SDS data zero_copy(columnIndex_2[0:(nnz)])
#pragma SDS data sys_port(columnIndex_2:ps7_S_AXI_HP2)
//#pragma SDS data sys_port(columnIndex_2:ps_e_S_AXI_HPC1_FPD)

#pragma SDS data zero_copy(columnIndex_3[0:(nnz)])
#pragma SDS data sys_port(columnIndex_3:ps7_S_AXI_HP3)
//#pragma SDS data sys_port(columnIndex_3:ps_e_S_AXI_HPC1_FPD)



//----------------------------------------------------------------
#pragma SDS data zero_copy(values_0[0:(nnz)])
#pragma SDS data sys_port(values_0:ps7_S_AXI_HP0)

#pragma SDS data zero_copy(values_1[0:(nnz)])
#pragma SDS data sys_port(values_1:ps7_S_AXI_HP1)
//#pragma SDS data sys_port(values_1:ps_e_S_AXI_HPC0_FPD)

#pragma SDS data zero_copy(values_2[0:(nnz)])
#pragma SDS data sys_port(values_2:ps7_S_AXI_HP2)
//#pragma SDS data sys_port(values_2:ps_e_S_AXI_HPC1_FPD)

#pragma SDS data zero_copy(values_3[0:(nnz)])
#pragma SDS data sys_port(values_3:ps7_S_AXI_HP3)
//#pragma SDS data sys_port(values_3:ps_e_S_AXI_HPC1_FPD)



//----------------------------------------------------------------
#pragma SDS data zero_copy(y_0[0:(row_size*no_vectors)])
#pragma SDS data sys_port(y_0:ps7_S_AXI_HP0)
//#pragma SDS data sys_port(y_0:ps_e_S_AXI_HPC0_FPD)

#pragma SDS data zero_copy(y_1[0:(row_size*no_vectors)])
#pragma SDS data sys_port(y_1:ps7_S_AXI_HP1)
//#pragma SDS data sys_port(y_1:ps_e_S_AXI_HPC0_FPD)

#pragma SDS data zero_copy(y_2[0:(row_size*no_vectors)])
#pragma SDS data sys_port(y_2:ps7_S_AXI_HP2)
//#pragma SDS data sys_port(y_2:ps_e_S_AXI_HPC1_FPD)

#pragma SDS data zero_copy(y_3[0:(row_size*no_vectors)])
#pragma SDS data sys_port(y_3:ps7_S_AXI_HP3)
//#pragma SDS data sys_port(y_3:ps_e_S_AXI_HPC1_FPD)


//----------------------------------------------------------------
#pragma SDS data zero_copy(x[0:(col_size*no_vectors/4)])
#pragma SDS data sys_port(x:ps7_S_AXI_ACP)

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


		) {



for(u32 nv=0;nv<no_vectors;nv++)
{

   #pragma HLS DATAFLOW
   #pragma HLS loop_tripcount min=512 max=512
	spmm(
			ternary,
			rowPtr,

			columnIndex_0,
			columnIndex_1,
			columnIndex_2,
			columnIndex_3,

			values_0,
			values_1,
			values_2,
			values_3,

			y_0,
			y_1,
			y_2,
			y_3,

			x,
			nv,

			col_size,
			row_size,
			nnz,

			begin,
			end,
			first_rowPrt_value

			);
}

}
