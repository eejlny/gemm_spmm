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

#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <fstream>
#include <sstream> // std::stringstream

#include <sds_lib.h>
#include <hls_stream.h>
#include "ap_int.h"
#include "matrix_mult.h"
#include "spmm.h"

// note that  BLOCK shoudl be less than B_WIDTH_BLOCK
const int BLOCK=B_WIDTH_BLOCK;   //BLOCK should be less than B_WIDTH_BLOCK
const int PARALLEL_ROW = B_BLOCK_PARALLEL;
const int A_WIDTH_FIFO =  A_WIDTH;



void dsp_kernel_sw(DTYPE a_value,DTYPE b_block[B_HEIGHT][B_WIDTH_BLOCK],ap_int<32> b_row,ap_int<8> zero_point_lhs,ap_int<8> zero_point_rhs,DTYPE_OUT acc[B_WIDTH_BLOCK])
{
	//#pragma HLS ALLOCATION instances=mul limit=64 operation
	//#pragma HLS INLINE

	//DTYPE_OUT acc[B_WIDTH_BLOCK];
	//#pragma HLS ARRAY_PARTITION variable=acc complete

	for (int j = 0; j < B_WIDTH_BLOCK; j++) {

			#pragma HLS UNROLL
		
				acc[j] = 0;
        }



	for (int j = 0; j < B_WIDTH_BLOCK; j++) {
            #pragma HLS UNROLL
	    #pragma HLS PIPELINE
		for(int z = 0; z < DTYPE_LENGTH; z+=8) {
 	  		ap_int<8> A_val = a_value.range(z+7,z);
	  		ap_int<8> B_val = b_block[b_row][j].range(z+7,z);
			//acc[j] += (A_val-zero_point_lhs)*(B_val-zero_point_rhs);
			acc[j] += A_val*(B_val-zero_point_rhs);

		}
	} // j loop




}

int writec_sw(int N,int P, hls::stream<DTYPE_OUT> write_fifo[C_WIDTH_BLOCK], DTYPE* C,int array_c_adjust,int B_index, int B_index_loop,int tail)
{
		int B_WIDTH_INT;
		if (B_index < (B_index_loop-1))
			B_WIDTH_INT = B_WIDTH_BLOCK;
		else
			B_WIDTH_INT = tail;

		LOOP_WRITE1:    for (int i = 0; i < (N>>2); i++) {
			DTYPE C_out;
	  		LOOP_WRITE2: for (int j = 0; j < B_WIDTH_INT; j++) {
					#pragma HLS PIPELINE
					#pragma HLS loop_tripcount min=1 max=1 avg=1
					C_out =  write_fifo[j].read();
					#ifdef ENABLE_TRANSPOSE
						C[i+(j+B_index*B_WIDTH_BLOCK)*(array_c_adjust>>2)] = C_out;
						//printf("Wrote address %x\n", (int)(i+(j+B_index*B_WIDTH_BLOCK)*(array_c_adjust>>2))); 
					#else
						C[i*P+j+B_index*B_WIDTH_BLOCK] = C_out;
					#endif
				}	
			}
					
}


int scale_sw(ap_int<32> *quantized_multiplier, ap_int<32> *shift, ap_int<32> *bias, ap_int<8> zero_point_dst, ap_int<8> clamp_max,ap_int<8> clamp_min,int N, int M, int P, hls::stream<DTYPE_OUT> C_fifo[C_WIDTH_BLOCK],int B_index, int B_index_loop,int tail,hls::stream<DTYPE_OUT> write_fifo[C_WIDTH_BLOCK])
{


			int B_WIDTH_INT;
			if (B_index < (B_index_loop-1))
				B_WIDTH_INT = B_WIDTH_BLOCK;
			else
				B_WIDTH_INT = tail;

			LOOP_CH1:    for (int i = 0; i < N; i+=4) {
				ap_int<32> bias_val[4];
				ap_int<32> shift_val[4];
				ap_int<32> mult_val[4];
				bias_val[0] =  bias[i];
				bias_val[1] =  bias[i+1];
				bias_val[2] =  bias[i+2];
				bias_val[3] =  bias[i+3];
				shift_val[0] = shift[i];
				shift_val[1] = shift[i+1];
				shift_val[2] = shift[i+2];
				shift_val[3] = shift[i+3];
				mult_val[0] = quantized_multiplier[i];
				mult_val[1] = quantized_multiplier[i+1];
				mult_val[2] = quantized_multiplier[i+2];
				mult_val[3] = quantized_multiplier[i+3];
	  			//LOOP_CW1: for (int j = 0; j < B_WIDTH_INT; j++) {
				LOOP_CW1: for (int j = 0; j < B_WIDTH_BLOCK; j++) {
					#pragma HLS PIPELINE II=4
					//#pragma HLS UNROLL factor=2
					DTYPE C_out;
					LOOP_CH3:    for (int z = 0; z < 4; z++) {
					    //#pragma HLS PIPELINE
					    #pragma HLS loop_tripcount min=1 max=1 avg=1
						if (j<B_WIDTH_INT)
						{
							#ifdef ENABLE_SCALING
							ap_int<64> C_temp1 =  C_fifo[j].read() + bias_val[z];
							ap_int<32> total_shift1 = 31 - shift_val[z];
 		             				ap_int<64> round1 = (ap_int<64>)1 << (total_shift1 - 1);
							C_temp1 = C_temp1*mult_val[z] + round1;
							C_temp1 = (C_temp1 >> total_shift1) + zero_point_dst;
							#else
							ap_int<64> C_temp1 =  C_fifo[j].read()+ bias_val[z];
							#endif
							ap_int<8> C_temp5 = C_temp1;
		                         		if (C_temp1 < clamp_min) C_temp5 = clamp_min;
							if (C_temp1 > clamp_max) C_temp5 = clamp_max; 
	
							C_out = ((C_out >> 8) | ((int)C_temp5 << 24));

							if (z==3)
							{
								write_fifo[j] << C_out;
							}
						}
					}
								
				    }
			    }     
}

int compute_sw(ap_uint<2> mode, ap_int<8> zero_point_lhs,  ap_int<8> zero_point_rhs, int N, int M, int P, DTYPE* A, DTYPE* B, hls::stream<DTYPE_OUT> C_fifo[B_WIDTH_BLOCK],int B_index, int B_index_loop, int tail,int *rowPtr,int *columnIndex,DTYPE *values)
{


       //#pragma HLS allocation function instances=dsp_kernel limit=1


	DTYPE B_accel[B_HEIGHT][B_WIDTH_BLOCK];
        #pragma HLS array_partition variable=B_accel block factor= BLOCK/2 dim=2

        //hls::stream<DTYPE>       A_accel;
        //#pragma HLS STREAM variable=A_accel depth=A_WIDTH_FIFO dim=1

	DTYPE A_accel[A_WIDTH];
        //#pragma HLS array_partition variable=A_accel cyclic factor=


	DTYPE_OUT acc[B_WIDTH_BLOCK];
	#pragma HLS ARRAY_PARTITION variable=acc complete


	DTYPE_OUT acc2[B_WIDTH_BLOCK];
	#pragma HLS ARRAY_PARTITION variable=acc2 complete


	//hls::stream<int>             col_indices_fifo;
	//#pragma HLS STREAM variable=col_indices_fifo depth=1024 dim=1

	int col_indices[A_WIDTH];

        #pragma HLS DATAFLOW	

	int B_WIDTH_INT,rnnz;

	if (B_index < (B_index_loop-1))
		B_WIDTH_INT = B_WIDTH_BLOCK;
	else
		B_WIDTH_INT = tail;


         for (int j = 0; j < B_WIDTH_INT; j++) {
		        LOOP_BLOCK1 : for (int i = 0; i < M; i++) {
					    #pragma HLS loop_tripcount min=84 max=84 avg=84
						#pragma HLS PIPELINE
						#pragma HLS loop_tripcount min=16 max=16 avg=16
						B_accel[i][j] = B[i+j*M+B_index*B_WIDTH_BLOCK*M];
						//std::cout << i << " " << j << std::endl;
					}
		       }
    
     
  	

	for (int A_index = 0; A_index < N; A_index++) {
               #pragma HLS loop_tripcount min=6 max=6 avg=6

		//std::cout << "A_index is " << A_index << " out of " << N/A_HEIGHT_BLOCK << std::endl;

			if (mode == 0) //gemm load A row
			{

				#ifdef ENABLE_GEMM
				LOOP_A_ROW_GEMM : for (int j = 0; j < M; j++) {
					#pragma HLS PIPELINE
					//A_accel <<  A[A_index*M*A_HEIGHT_BLOCK+j];
					A_accel[j] =  A[A_index*M*A_HEIGHT_BLOCK+j];
					//std::cout << "A is " << A_accel[i][j] << std::endl;
				}
				#endif
			}
			else //spmm load A row
			{

				#ifdef ENABLE_SPMM
				int current_index= rowPtr[A_index];
				int next_index=rowPtr[A_index+1];
				rnnz = next_index-current_index;
				//LOOP_A_ROW_SPMM : for (int j = current_index; j < next_index; j++) {
				LOOP_A_ROW_SPMM : for (int j = 0; j < rnnz; j++) {
					#pragma HLS PIPELINE
					//A_accel <<  values[j];
					//col_indices_fifo << columnIndex[j];
					A_accel[j] =  values[current_index+j];
					col_indices[j] = columnIndex[current_index+j];
					//A_accel[z] =  current_index+j;
					//col_indices[z] = current_index+j;

				}
				#endif

			}
		

		//computing
	
		for (int j = 0; j < B_WIDTH_BLOCK; j++) {

			#pragma HLS UNROLL
		
				acc2[j] = 0;
		}

		if (mode == 0) //gemm
		{
			#ifdef ENABLE_GEMM

	   		DSP_LOOP_GEMM: for(int k = 0; k < M; k+=1) {
				#pragma HLS loop_tripcount min=84 max=84 avg=84
	        		#pragma HLS PIPELINE
	        		#pragma HLS UNROLL factor=PARALLEL_ROW

		   		//how many rows of B block are computed in parallel in multiplication loop
		   		//for example a couple of B block rows are multiplied for A 1 row in each loop iteration
		   		//it basically reduces how the loop iterations by 2 if it is 2.

				//DTYPE v = A_accel.read();
				DTYPE v = A_accel[k];
				dsp_kernel_sw(v,B_accel,k,zero_point_lhs,zero_point_rhs,acc);

				for (int j = 0; j < B_WIDTH_BLOCK; j++) {

					#pragma HLS UNROLL
		
					acc2[j] += acc[j];
        			}



			} // k loop
     			for (int j = 0; j < B_WIDTH_BLOCK; j++) {
				//#pragma HLS loop_tripcount min=16 max=16 avg=16
	                	#pragma HLS UNROLL
				if (j < B_WIDTH_INT)
				{
					C_fifo[j] << acc2[j];
				}
			}
			#endif
	        } //mode spmm
	        else
	        {
			#ifdef ENABLE_SPMM

 			DSP_LOOP_SPMM: for (int i = 0; i < rnnz; i+=1) {
		 	 #pragma HLS PIPELINE
	        	 #pragma HLS UNROLL factor=PARALLEL_ROW
    				//DTYPE v = A_accel.read();
    				//int   ci = col_indices_fifo.read();
    				DTYPE v = A_accel[i];
    				int   ci = col_indices[i];

				dsp_kernel_sw(v,B_accel,ci,zero_point_lhs,zero_point_rhs,acc);

				for (int j = 0; j < B_WIDTH_BLOCK; j++) {

					#pragma HLS UNROLL
		
					acc2[j] += acc[j];
        			}


		     	} //i loop

            		for (int j = 0; j < B_WIDTH_BLOCK; j++) {
	                	#pragma HLS UNROLL
				if (j < B_WIDTH_INT)
				{
					C_fifo[j] << acc2[j];
				}
			}
			#endif

		 } //else

          } // A_index loop

}

int mmult_wrapper_sw(ap_uint<2> mode, ap_int<32> *quantized_multiplier, ap_int<32> *shift, ap_int<32> *bias,  ap_int<32> bias_count, ap_int<8> zero_point_lhs,  ap_int<8> zero_point_rhs, ap_int<8> zero_point_dst, ap_int<8> clamp_max,ap_int<8> clamp_min,int N, int M, int P, DTYPE* A, DTYPE* B, DTYPE* C, int array_c_adjust, int B_index, int B_index_loop, int tail,int *rowPtr,int *columnIndex,DTYPE *values)
{

      hls::stream<DTYPE_OUT>       C_fifo[C_WIDTH_BLOCK];
      #pragma HLS STREAM variable=C_fifo depth=8 dim=1

      //hls::stream<DTYPE_OUT>       write_fifo[C_WIDTH_BLOCK];
      //#pragma HLS STREAM variable=write_fifo depth=16 dim=1

      hls::stream<DTYPE_OUT>       write_fifo[C_WIDTH_BLOCK];
      #pragma HLS STREAM variable=write_fifo depth=8 dim=1


      #pragma HLS DATAFLOW	

          compute_sw(mode, zero_point_lhs,  zero_point_rhs, N, M, P, A, B,C_fifo, B_index, B_index_loop, tail,rowPtr,columnIndex,values);
	
	  scale_sw(quantized_multiplier, shift, bias, zero_point_dst, clamp_max,clamp_min,N, M, P,C_fifo, B_index, B_index_loop, tail,write_fifo);
    
          writec_sw(N,P, write_fifo, C,array_c_adjust,B_index, B_index_loop, tail);
	

}


typedef unsigned long u32;

/*
 The amount of data saved in the FPGA is B_HEIGHT*B_WIDTH_BLOCK+A_WIDTH+B_WIDTH_BLOCK which should be less than FPGA BRAM size
*/

#pragma SDS data zero_copy(rowPtr[0:(N+1)])
#pragma SDS data sys_port(rowPtr:ps_e_S_AXI_HPC0_FPD)
//----------------------------------------------------------------
#pragma SDS data zero_copy(columnIndex[0:(nnz)])
//#pragma SDS data sys_port(columnIndex_0:ps_e_S_AXI_HPC0_FPD)

//----------------------------------------------------------------
#pragma SDS data zero_copy(values[0:(nnz)])
#pragma SDS data sys_port(values:ps_e_S_AXI_HPC0_FPD)


#pragma SDS data zero_copy(bias[0:(bias_count)])
#pragma SDS data zero_copy(quantized_multiplier[0:(bias_count)])
#pragma SDS data zero_copy(shift[0:(bias_count)])

#pragma SDS data zero_copy(A[0:N*M],B[0:M*P], C[0:N*P])


void mmult_top_sw(ap_uint<2> mode, ap_int<32> *quantized_multiplier, ap_int<32> *shift, ap_int<32> *bias,  ap_int<32> bias_count, ap_int<8> zero_point_lhs,  ap_int<8> zero_point_rhs, ap_int<8> zero_point_dst, ap_int<8> clamp_max,ap_int<8> clamp_min,int N, int M, int P, DTYPE* A, DTYPE* B, DTYPE* C,int array_c_adjust,int *rowPtr,int *columnIndex,DTYPE *values,int nnz)
{


	
	 //c_fifo_stream_t       C_fifo[B_WIDTH_BLOCK];
	 //#pragma HLS STREAM variable=C_fifo depth=1024 dim=1


	 ap_int<32> bias_data[1024]; 
	 ap_int<32> shift_data[1024];
	 ap_int<32> quantized_multiplier_data[1024];
	 


	 //load bias
         //preloading bias and param data seems to be a good idea but in practice performance is the same and we save preloading overhead
         //param data is loaded in demand in this case
         //preloading is important for certain matrix configurations with small A and large B so I am going to leave it
         //if (bias_count > 0) 
         {
	 	for(int bias_index=0;bias_index<bias_count;bias_index++)
	 	{
			#pragma HLS PIPELINE
			 bias_data[bias_index]=bias[bias_index];
			 shift_data[bias_index]=shift[bias_index];
			 quantized_multiplier_data[bias_index]=quantized_multiplier[bias_index];
	 	}
	 }
         //else
	 {

	 ap_int<32> tail = P%B_WIDTH_BLOCK;
	 ap_int<32> B_index_loop = P/B_WIDTH_BLOCK + 1;
	  

         for (int B_index = 0; B_index < B_index_loop; B_index++) {

        //#pragma HLS DATAFLOW

          //mmult_wrapper(mode, quantized_multiplier_data, shift_data, bias_data, bias_count, zero_point_lhs, zero_point_rhs, zero_point_dst, clamp_max,clamp_min,N, M, P, A, B, C,  B_index, B_index_loop, tail,rowPtr,columnIndex,values );
          mmult_wrapper_sw(mode, quantized_multiplier, shift, bias, bias_count, zero_point_lhs, zero_point_rhs, zero_point_dst, clamp_max,clamp_min,N, M, P, A, B, C, array_c_adjust, B_index, B_index_loop, tail,rowPtr,columnIndex,values );
 
      }
 
     }
}




int kernelmult1_sw(
int cores,
ap_uint<2> mode,
ap_int<32> *quantized_multiplier,
ap_int<32> *shift,
ap_int<32> *bias,
ap_int<32> bias_count,
ap_int<8> zero_point_lhs,
ap_int<8> zero_point_rhs,
ap_int<8> zero_point_dst,
ap_int<8> clamp_max,
ap_int<8> clamp_min,
DTYPE *array_a,
DTYPE *array_b,
DTYPE *array_c,
DTYPE *values,
int *colIndices,
int *rowPtr,
int nnz,
int N,
int M,
int P
)
{

  DTYPE *array_a_block;
  DTYPE *array_b_block;
  DTYPE *array_c_block;

  int N_block,P_block;

  int nnz_block, nnz_done;
  int *rowPtr_block;
  int *colIndices_block;
  DTYPE *values_block;

  
  int P_tail = P%cores;
  int P_tail1 =0;
  int P_tail2 =0;
  int P_tail3 =0;
  int P_tail4 =0;
  int nnz_block1,nnz_block2,nnz_block3,nnz_block4;

  //this value is needed with multiple cores and spliting A matrix to adjust where each core writes output C matrix

  int array_c_adjust=N;

 

  int A_split = ((cores >> 3)&0x1);
  int core_count = (cores&0x7);
  int bias_offset = 0;


  //0 => B split, 1 => A split

  if(A_split)
  {

       N_block = N/core_count;
       P_block = P;
       P_tail = 0;
       P_tail1 =0;
       P_tail2 =0;
       P_tail3 =0;
       P_tail4 =0;
       bias_count = bias_count/core_count;
       bias_offset = N_block;
       nnz_block1 = *(rowPtr + N_block) - *(rowPtr);
       nnz_block2 = *(rowPtr + 2*N_block) - *(rowPtr + N_block);
       nnz_block3 = *(rowPtr + 3*N_block) - *(rowPtr + 2*N_block);
       nnz_block4 = *(rowPtr + 4*N_block) - *(rowPtr + 3*N_block);
       printf("nnz block 1 %d nnz block 2 %d\n",nnz_block1,nnz_block2);
       printf("rowPtr 0 %x \n",*(rowPtr+0));


  }
  else
  {
      N_block = N;
      P_block = P/core_count;
      P_tail = P%core_count;
      P_tail1 =0;
      P_tail2 =0;
      P_tail3 =0;
      P_tail4 =0;
      nnz_block1 = nnz;
      nnz_block2 = nnz;
      nnz_block3 = nnz;
      nnz_block4 = nnz;
  
 }
	
  array_a_block = (DTYPE*)(array_a);
  array_b_block = (DTYPE*)(array_b);
  array_c_block = (DTYPE*)(array_c);


  rowPtr_block = rowPtr;
  colIndices_block = colIndices;
  values_block = values;

 printf("core 1 array a %x array b %x array c %x\n", (int*)array_a_block,(int*)array_b_block,(int*)array_c_block);
 printf("core 1 rowPtr_block  %x nnz done %d colIndices_block %x values_block %x \n",rowPtr_block,nnz_done,colIndices_block,values_block);



  #pragma SDS resource(1)
  #pragma SDS async(1)



  //lower two bits how many cores

  if(!A_split)
  {
  	switch(core_count) {
  	case 1:
	  P_tail1 = P_tail;
       	 break;
        case 2:
	P_tail2 = P_tail;
        break;
        case 3: 
	P_tail3 = P_tail;
        break;
        case 4:
	P_tail4 = P_tail;
        break;
    }
  }



  
    #pragma SDS resource(1)
    #pragma SDS async(1)

    mmult_top_sw(mode,quantized_multiplier,shift,bias,bias_count,zero_point_lhs,zero_point_rhs, zero_point_dst,clamp_max,clamp_min,N_block, M, P_block+P_tail1, array_a_block, array_b_block, array_c_block,array_c_adjust,rowPtr_block,colIndices_block,values_block,nnz_block1);

    for(int i=0;i<256;i++)
    {
      printf("Core 1 C value at %d is %x\n",i,(int)*(array_c+i));
    }

   if (core_count > 1)
   {
        
        if(A_split)
	{
       	        array_a_block = (DTYPE*)(array_a+N_block*M);
        	array_b_block = (DTYPE*)(array_b);
        	array_c_block = (DTYPE*)(array_c+N_block/4);

                rowPtr_block = rowPtr + N_block;
	        nnz_done = *(rowPtr + N_block);
	        colIndices_block = colIndices;
	        values_block = values;

		printf("core 2 array a %x array b %x array c %x\n", (int*)array_a_block,(int*)array_b_block,(int*)array_c_block);
		printf("core 2 rowPtr_block  %x nnz done %d colIndices_block %x values_block %x \n",rowPtr_block,nnz_done,colIndices_block,values_block);


        }
	else
	{
      	        array_a_block = (DTYPE*)(array_a);
        	array_b_block = (DTYPE*)(array_b+P_block*M);
        	array_c_block = (DTYPE*)(array_c+P_block*N/4);
	}

  	#pragma SDS resource(2)
  	#pragma SDS async(2)

      
  	mmult_top_sw(mode,quantized_multiplier+bias_offset,shift+bias_offset,bias+bias_offset,bias_count,zero_point_lhs,zero_point_rhs, zero_point_dst,clamp_max,clamp_min,N_block, M, P_block+P_tail2, array_a_block, array_b_block, array_c_block,array_c_adjust,rowPtr_block,colIndices_block,values_block,nnz_block2);
        
    	for(int i=0;i<256;i++)
    	{
      		printf("Core 2 C value at %d is %x\n",i,(int)*(array_c+i));
    	}

   

        if (core_count > 2)
	{
        	 if(A_split)
		 {
       	       	        array_a_block = (DTYPE*)(array_a+2*N_block*M);
        		array_b_block = (DTYPE*)(array_b);
        	        array_c_block = (DTYPE*)(array_c+2*N_block/4);

                        rowPtr_block = rowPtr_block + N_block;
	                nnz_done = *(rowPtr_block + N_block);
	                colIndices_block = colIndices_block;
	                values_block = values_block;

                 }
	         else
	         {
      	                array_a_block = (DTYPE*)(array_a);
        	        array_b_block = (DTYPE*)(array_b+2*P_block*M);
        	        array_c_block = (DTYPE*)(array_c+2*P_block*N/4);
	         }

  		#pragma SDS resource(3)
  		#pragma SDS async(3)

  		mmult_top_sw(mode,quantized_multiplier+2*bias_offset,shift+2*bias_offset,bias+2*bias_offset,bias_count,zero_point_lhs,zero_point_rhs, zero_point_dst,clamp_max,clamp_min,N_block, M, P_block+P_tail3, array_a_block, array_b_block, array_c_block,array_c_adjust,rowPtr_block,colIndices_block,values_block,nnz_block3);
 
		if (core_count > 3)
		{
        	        if(A_split)
		        {
       	       	            array_a_block = (DTYPE*)(array_a+3*N_block*M);
        		    array_b_block = (DTYPE*)(array_b);
        	            array_c_block = (DTYPE*)(array_c+3*N_block/4);

                 	    rowPtr_block = rowPtr_block + N_block;
	                    nnz_done = *(rowPtr_block + N_block);
	                    colIndices_block = colIndices_block;
	                    values_block = values_block;

                        }
	                else
	                {
      	                    array_a_block = (DTYPE*)(array_a);
        	            array_b_block = (DTYPE*)(array_b+3*P_block*M);
        	            array_c_block = (DTYPE*)(array_c+3*P_block*N/4);
	                }

  			#pragma SDS resource(4)
  			#pragma SDS async(4)

	

  			mmult_top_sw(mode,quantized_multiplier+3*bias_offset,shift+3*bias_offset,bias+3*bias_offset,bias_count,zero_point_lhs,zero_point_rhs, zero_point_dst,clamp_max,clamp_min,N_block, M, P_block+P_tail4, array_a_block, array_b_block, array_c_block,array_c_adjust,rowPtr_block,colIndices_block,values_block,nnz_block4);
 		 	#pragma SDS wait(4)
		}

  		#pragma SDS wait(3)
	} 
  	#pragma SDS wait(2)
  }
  #pragma SDS wait(1) 

}
