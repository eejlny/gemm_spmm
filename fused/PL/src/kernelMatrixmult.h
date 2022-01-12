#ifndef KERNELMATRIXMULT_H_
#define KERNELMATRIXMULT_H_

int kernelgemm1(
ap_uint<2> ternary,
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
int M,
int P,
int begin,
int end);

int kernelspmm1(
		int ternary,
		DATA_TYPE *values,
		int       *colIndices,
		int       *rowPtr,
		DATA_TYPE_X *x,
		int        no_vectors,
		DATA_TYPE *y,
		int        rows,
		int        cols,
		int        nnz,
    		ap_int<32> *quantized_multiplier, 
                ap_int<32> *shift, 
                ap_int<32> *bias,  
                ap_int<32> bias_count, 
                ap_int<8> zero_point_lhs,  
                ap_int<8> zero_point_rhs, 
                ap_int<8> zero_point_dst,
		ap_int<8> clamp_max,
		ap_int<8> clamp_min,
		int        begin,
		int        end
);



//int kernelMatrixmult2(
//ap_uint<2> ternary,
//ap_int<32> quantized_multiplier,
//ap_int<32> shift,
//ap_int<32> *bias,
//ap_int<32> bias_count,
//ap_uint<8> zero_point_lhs,
//ap_uint<8> zero_point_rhs,
//ap_uint<8> zero_point_dst,
//ap_uint<8> clamp_max,
//ap_uint<8> clamp_min,
//DTYPE *array_a,
//DTYPE *array_b,
//DTYPE *array_c,
//int M,
//int P,
//int begin,
//int end);


#endif 
