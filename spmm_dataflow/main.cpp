#ifdef _WIN32
#include <cstring>
#include <conio.h>
#include <Windows.h>
#else
#include <unistd.h>
#include <sys/time.h>
#include "sds_lib.h"

#endif
#include "math.h"

#include <iostream>

#include "spmm_block.h"

double getTimestamp() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_usec + tv.tv_sec * 1e6;
}
void msleep(u32 c) {
	usleep(c * 1000);
}



void* fpga_malloc(size_t size) {
	void* result = NULL;
	result = (float*)sds_alloc(size);
	return result;
}

void fpga_free(void* ptr) {
	sds_free(ptr);
}



DATA_TYPE *array_values;
u32*       array_colIndices;
u32*       array_rowPtr;
DATA_TYPE_X *array_x;
DATA_TYPE_OUT *array_y;

DATA_TYPE_OUT * array_y_golden;

u32 row_size;
u32 col_size;
u32 nnz;



u32 no_vectors;
float cpu_percentage;
float low_percentage;
float high_percentage;





u32 read_mtx_spmm(u32* row_size, u32* col_size, u32* nnz,FILE *fp_input) {

	printf("read_csr: check point 1\n");

	u32 r;
	u32 c;
	DATA_TYPE v;

	//char* inFilename = "/mnt/weights_byte_000.csr";

	//fp_input = fopen(inFilename, "r");

	u32 nnzeo_false = 0;
	if (fp_input != NULL) {
		printf("read_mtx_spmm: check point 2\n");
		char line[1000];
		while (fgets(line, sizeof line, fp_input) != NULL) {// read a line from a file
			if (line[0] != '%') {
				sscanf(line, "%d %d %d", row_size, col_size, nnz);
				std::cout << "row_size = " <<  *row_size << " col_size = " << *col_size << " nnz = " << *nnz << std::endl;
				printf("read_mtx_spmm: check point 3\n");

				{
					array_values = (DATA_TYPE*)fpga_malloc(*nnz * sizeof(DATA_TYPE));
					if (!array_values) {
						std::cout << "unable to allocate memory array_values" << std::endl;
						exit(1);
					}

					array_colIndices = (u32*)fpga_malloc(*nnz * sizeof(u32));
					if (!array_colIndices) {
						std::cout << "unable to allocate memory array_colIndices" << std::endl;
						exit(1);
					}

					array_rowPtr = (u32*)fpga_malloc((*row_size + 1) * sizeof(u32));
					if (!array_rowPtr) {
						std::cout << "unable to allocate memory array_rowPtr" << std::endl;
						exit(1);
					}

					array_x = (DATA_TYPE_X*)fpga_malloc(*col_size * no_vectors * sizeof(DATA_TYPE_X)/4);
					if (!array_x) {
						std::cout << "unable to allocate memory array_x" << std::endl;
						exit(1);
					}

					array_y = (DATA_TYPE_OUT*)fpga_malloc(*row_size * no_vectors * sizeof(DATA_TYPE_OUT));
					if (!array_y) {
						std::cout << "unable to allocate memory array_y" << std::endl;
						exit(1);
					}

					array_y_golden = (DATA_TYPE_OUT*)malloc(*row_size * no_vectors * sizeof(DATA_TYPE_OUT));
					if (!array_y_golden) {
						std::cout << "unable to allocate memory array_y_golden" << std::endl;
						exit(1);
					}
				}

				u32 line_number = 0;
				while (fgets(line, sizeof line, fp_input) != NULL) {// read a line from a file
					if (line_number < *nnz) {

						sscanf(line, "%d %d", &c, &v);

						//printf("colindices %d val %f\n", c, v);
						//std::cout << "colindices" << c << " val " << v << std::endl;


						*(array_colIndices + line_number) = c;


						*(array_values + line_number) = v;

					}
					else {
						sscanf(line, "%d", &r);

						//printf("rowptr %d \n", r);
						//std::cout << "rowptr " << c << std::endl;

						*(array_rowPtr + (line_number - (*nnz))) = r;
					}
					line_number++;
				}
			}
		}
	}
	else {
		//perror(argv[1]); //print the error message on stderr.
		std::cout << "Error with input file name" << std::endl;
		exit(1);
	}

	return 0;
}


void arg_process(u32 argc, char** argv) {


	if (argc < 2) {
		std::cout << "Usage: " << argv[0] << " a-hight a-width chunk_size " << std::endl;
		exit(0);
	}
	else {

	}
}



u32 golden_spmm_ternary(DATA_TYPE * values, u32 *row_ptr, u32* col_indices, DATA_TYPE_X * x, u32 no_vectors, DATA_TYPE_OUT *y, u32 row_size, u32 col_size) {

	printf("gold_spmm_ternary: check point 1\n");
	u32 nvc = 0, i = 0, j = 0, rowStart = 0, rowEnd = row_size;

	DATA_TYPE_OUT y0 = 0;
	u32 last_j = 0;
	for (nvc = 0; nvc < no_vectors; nvc++) {
		for (i = rowStart; i < rowEnd; ++i) {
			y0 = 0;
			for (j = row_ptr[i]; j < row_ptr[i + 1]; ++j) {
				//y0 += values[j] * x[nvc*col_size+col_indices[j]];
				for(int z = 0; z < DTYPE_LENGTH; z+=2) {
					            DATA_TYPE values_val1 = values[j];
					        	ap_int<2> values_val = values_val1.range(z+1,z);
					        	int x_value = nvc*col_size+col_indices[j];
					        	int x_up = x_value >> 4;
					        	int x_down = (x_value & 0xF);
								DATA_TYPE values_val_temp = values_val;
						       	ap_int<2> x_temp;
								switch(values_val_temp)
								{
									 case 0:
										 //std::cout << "C is" << C[j] << std::endl;
										 break;
									 case 1:
										 x_temp = x[x_up].range(x_down*2+1,x_down*2);
										 y0 += x_temp;
										 //std::cout << "B is" << b[k][j].range(z+1,z) << std::endl;
										 //std::cout << "C is" << C[j] << std::endl;
										 break;
									 case -1:
										 x_temp = x[x_up].range(x_down*2+1,x_down*2);
										 y0 -= x_temp;
										 //std::cout << "B is" << b[k][j].range(z+1,z) << std::endl;
										 //std::cout << "C is" << C[j] << std::endl;
										 break;
								}

				}
				//std::cout << "y0 is " << y0 << std::endl;
			}
			y[nvc*row_size+i] = y0;
		}
	}

	return 0;
}



u32 golden_spmm_byte(DATA_TYPE * values, u32 *row_ptr, u32* col_indices, DATA_TYPE_X * x, u32 no_vectors, DATA_TYPE_OUT *y, u32 row_size, u32 col_size) {

	printf("gold_spmm_byte: check point 1\n");
	u32 nvc = 0, i = 0, j = 0, rowStart = 0, rowEnd = row_size;

	DATA_TYPE_OUT y0 = 0;
	u32 last_j = 0;
	for (nvc = 0; nvc < no_vectors; nvc++) {
		for (i = rowStart; i < rowEnd; ++i) {
			y0 = 0;
			for (j = row_ptr[i]; j < row_ptr[i + 1]; ++j) {
				//y0 += values[j] * x[nvc*col_size+col_indices[j]];
				for(int z = 0; z < DTYPE_LENGTH; z+=8) {
					            DATA_TYPE values_val1 = values[j];
								ap_int<8> values_val = values_val1.range(z+7,z);
								int x_value = nvc*col_size+col_indices[j];
								int x_up = x_value >> 2;
								int x_down = (x_value & 0x3);
						       	y0 += values_val * x[x_up].range(x_down*8+7,x_down*8);
						       	//std::cout << "y0 " << y0 << std::endl;

				}
				//std::cout << "y0 is " << y0 << std::endl;
			}
			//std::cout << "y0 is " << y0 << std::endl;
			y[nvc*row_size+i] = y0;
		}
	}

	return 0;
}


u32 golden_spmm_quad(DATA_TYPE * values, u32 *row_ptr, u32* col_indices, DATA_TYPE_X * x, u32 no_vectors, DATA_TYPE_OUT *y, u32 row_size, u32 col_size) {

	printf("gold_spmm_quad: check point 1\n");
	u32 nvc = 0, i = 0, j = 0, rowStart = 0, rowEnd = row_size;

	DATA_TYPE_OUT y0 = 0;
	u32 last_j = 0;
	for (nvc = 0; nvc < no_vectors; nvc++) {
		for (i = rowStart; i < rowEnd; ++i) {
			y0 = 0;
			for (j = row_ptr[i]; j < row_ptr[i + 1]; ++j) {
				//y0 += values[j] * x[nvc*col_size+col_indices[j]];
				for(int z = 0; z < DTYPE_LENGTH; z+=4) {
					            DATA_TYPE values_val1 = values[j];
								ap_int<4> values_val = values_val1.range(z+3,z);
								int x_value = nvc*col_size+col_indices[j];
								int x_up = x_value >> 3;
								int x_down = (x_value & 0x7);
						       	y0 += values_val * x[x_up].range(x_down*4+3,x_down*4);
						       	//std::cout << "y0 " << y0 << std::endl;

				}
				//std::cout << "y0 is " << y0 << std::endl;
			}
			//std::cout << "y0 is " << y0 << std::endl;
			y[nvc*row_size+i] = y0;
		}
	}

	return 0;
}



int main(int argc, char** argv) {


	std::cout << "spmm running" << std::endl;

    if(argc<3)
    {
	std::cout << "Error not enough arguments " << std::endl;
	exit(1);
    }



	int ireartion_no    = 1;
	no_vectors      = 512; //64;


	FILE *fp_input;

	fp_input = fopen(argv[1], "r");

    int ternary = atoi(argv[2]);

	read_mtx_spmm(&row_size, &col_size, &nnz,fp_input);

	if(ternary==0)
	{
		for (u32 i = 0; i < no_vectors; i++) {
			for (u32 j = 0; j < (col_size>>2); j++) {
				//DATA_TYPE t = (1.0*rand()+1.0)/RAND_MAX;
				//DATA_TYPE t = DATA_TYPE(rand() % (col_size * no_vectors));
				//DATA_TYPE t = DATA_TYPE(col_size * no_vectors);
				//array_x[i*col_size+j] = t;
				array_x[i*(col_size>>2)+j] = 0x01010101;
			}
		}
	}
	else if (ternary==1)
	{
		for (u32 i = 0; i < no_vectors; i++) {
			for (u32 j = 0; j < (col_size>>2); j++) {
				//DATA_TYPE t = (1.0*rand()+1.0)/RAND_MAX;
				//DATA_TYPE t = DATA_TYPE(rand() % (col_size * no_vectors));
				//DATA_TYPE t = DATA_TYPE(col_size * no_vectors);
				//array_x[i*col_size+j] = t;
				array_x[i*(col_size>>2)+j] = 0x55555555;
			}
		}
	}
	else
	{
		for (u32 i = 0; i < no_vectors; i++) {
			for (u32 j = 0; j < (col_size>>2); j++) {
				//DATA_TYPE t = (1.0*rand()+1.0)/RAND_MAX;
				//DATA_TYPE t = DATA_TYPE(rand() % (col_size * no_vectors));
				//DATA_TYPE t = DATA_TYPE(col_size * no_vectors);
				//array_x[i*col_size+j] = t;
				array_x[i*(col_size>>2)+j] = 0x11111111;
			}
		}
	}
	std::cout << "main check point 1 " << std::endl;

	double start_time, end_time, execution_time;



	//======================ONLY FPGA ==========================================


	start_time = getTimestamp();
	//for (u32 i = 0; i < ireartion_no; i++) {
		spmm_block(
		    ternary,
			array_values,
			array_colIndices,
			array_rowPtr,
			array_x,
			no_vectors,
			array_y,
			row_size,
			col_size,
			nnz,
			0,
			row_size);
	//}
	end_time = getTimestamp();
	execution_time = (end_time - start_time) / (1000);
	std::cout << "FPGA " << " Total execution_time = " << execution_time << " msec" << std::endl;



	start_time = getTimestamp();

	if (ternary==0)
          {
		    golden_spmm_byte(
			array_values,
			array_rowPtr,
			array_colIndices,
			array_x,
			no_vectors,
			array_y_golden,
			row_size,
			col_size);
		  }
	else if (ternary==1)
		  {

		    golden_spmm_ternary(
			array_values,
			array_rowPtr,
			array_colIndices,
			array_x,
			no_vectors,
			array_y_golden,
			row_size,
			col_size);

		  }
	else
	  {
	    golden_spmm_quad(
		array_values,
		array_rowPtr,
		array_colIndices,
		array_x,
		no_vectors,
		array_y_golden,
		row_size,
		col_size);

	  }





	end_time = getTimestamp();

	execution_time = (end_time - start_time) / (1000);
	std::cout << "CPU " << " Total execution time = " << execution_time << " msec" << std::endl;




    int status = 0;

    for (u32 i = 0; i < row_size*no_vectors; i++) {
        //float diff = array_y[i]- array_y_golden[i];
		//std::cout << "Data at  " << i << "array_y[" << i << "] = " << array_y[i] << " != " << "array_y_golden[" << i << "] = " << array_y_golden[i] << std::endl;
        if (array_y[i] != array_y_golden[i]) {
            std::cout << "Error at  " << i << " array_y["<<i << "] = " << array_y[i] << " != " << " array_y_golden["<<i << "] = " << array_y_golden[i] << std::endl;
            status = -1;
            break;
        }
    }


    if (status == 0) {
        std::cout << "spmm SUCCESS!" << std::endl;
    } else {
        std::cout << "spmm FAILED!" << std::endl;
    }

	free(array_y_golden);

	fpga_free(array_values);
	fpga_free(array_colIndices);
	fpga_free(array_rowPtr);
	fpga_free(array_x);



}
