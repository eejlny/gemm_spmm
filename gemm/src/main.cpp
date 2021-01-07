/*
(c) Copyright 2013 - 2016 Xilinx, Inc. All rights reserved. 

This file contains confidential and proprietary information of Xilinx, Inc. and
is protected under U.S. and international copyright and other intellectual
property laws.

DISCLAIMER 
This disclaimer is not a license and does not grant any rights to the materials
distributed herewith. Except as otherwise provided in a valid license issued to
you by Xilinx, and to the maximum extent permitted by applicable law: (1) THESE
MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL FAULTS, AND XILINX HEREBY
DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY,
INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-INFRINGEMENT, OR
FITNESS FOR ANY PARTICULAR PURPOSE; and (2) Xilinx shall not be liable (whether
in contract or tort, including negligence, or under any other theory of
liability) for any loss or damage of any kind or nature related to, arising
under or in connection with these materials, including for any direct, or any
indirect, special, incidental, or consequential loss or damage (including loss
of data, profits, goodwill, or any type of loss or damage suffered as a result
of any action brought by a third party) even if such damage or loss was
reasonably foreseeable or Xilinx had been advised of the possibility of the
same.

CRITICAL APPLICATIONS
Xilinx products are not designed or intended to be fail-safe, or for use in any
application requiring fail-safe performance, such as life-support or safety
devices or systems, Class III medical devices, nuclear facilities, applications
related to the deployment of airbags, or any other applications that could lead
to death, personal injury, or severe property or environmental damage
(individually and collectively, "Critical Applications"). Customer assumes the
sole risk and liability of any use of Xilinx products in Critical Applications,
subject only to applicable laws and regulations governing limitations on product
liability.

THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE AT
ALL TIMES. 
*/

#include <iostream>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>

#include <string>
#include <fstream>
#include <sstream> // std::stringstream

#include "sds_lib.h"
#include "matrix_mult.h"
#include "kernelMatrixmult.h"

#define NUM_TESTS 1

int SM,SN,SP;

double getTimestamp() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_usec + tv.tv_sec * 1e6;
}


class perf_counter
{
public:
     uint64_t tot, cnt, calls;
     perf_counter() : tot(0), cnt(0), calls(0) {};
     inline void reset() { tot = cnt = calls = 0; }
     inline void start() { cnt = sds_clock_counter(); calls++; };
     inline void stop() { tot += (sds_clock_counter() - cnt); };
     inline uint64_t avg_cpu_cycles() { return (tot / calls); };
};

static void init_arrays(DTYPE *B, DTYPE_OUT *C_sw, DTYPE_OUT *C)
{
     for (int i = 0; i < SM; i++) {
          for (int j = 0; j < SP; j++) {
               //B[i * P + j] = DTYPE(rand() % (M * P));
               //B[i * P + j] =  DTYPE(M * P);
        	   B[i * SP + j] =  0x01;
        	  // std::cout << "B "<< i * P + j << " " << B[i * P + j] << std::endl;
          }
     }
     for (int i = 0; i < SN; i++) {
          for (int j = 0; j < SP; j++) {
               C_sw[i * SP + j] = 0;
               C[i * SP + j] = 0;
          }
     }
}

static void load_arrays_quad(DTYPE *A,std::ifstream& myFile)
{

	// Create an input filestream
    //std::ifstream myFile("/mnt/weights_qau.csv");

	// Make sure the file is open
	if(!myFile.is_open()) throw std::runtime_error("Could not open quad file");

	// Helper vars
	std::string line;
	int val;
	int val_count=0;
	DTYPE array_val;

    for (int i = 0; i < SN; i++) {
    	// Read data, line by line
    	std::getline(myFile, line);

	    // Create a stringstream of the current line
	    std::stringstream ss(line);


        for (int j = 0; j < SM; j++) {

	        //fill one array val
        	array_val = 0;
	        for(int z =0; z< DTYPE_LENGTH/4; z++)
	        {
	        	// Extract each integer
	        	ss >> val;

	        	switch(val)
	        	{
	        		case 0:
	        			array_val = array_val << 4;
	        			break;
	        		case 1:
	        			array_val = array_val << 4;
	        			array_val+= 1;
	        			break;
	        		case 2:
	        			array_val = array_val << 4;
	        			array_val+= 2;
	        			break;
	        		case 3:
	        			array_val = array_val << 4;
	        			array_val+= 3;
	        			break;
	        		case 4:
	        			array_val = array_val << 4;
	        			array_val+= 4;
	        			break;
	        		case 5:
	        			array_val = array_val << 4;
	        			array_val+= 5;
	        			break;
	        		case 6:
	        			array_val = array_val << 4;
	        			array_val+= 6;
	        			break;
	        		case 7:
	        			array_val = array_val << 4;
	        			array_val+= 7;
	        			break;
	        		case -1:
	        			array_val = array_val << 4;
	        			array_val+= 15;
	        			break;
	        		case -2:
	        			array_val = array_val << 4;
	        			array_val+= 14;
	        			break;
	        		case -3:
	        			array_val = array_val << 4;
	        			array_val+= 13;
	        			break;
	        		case -4:
	        			array_val = array_val << 4;
	        			array_val+= 12;
	        			break;
	        		case -5:
	        			array_val = array_val << 4;
	        			array_val+= 11;
	        			break;
	        		case -6:
	        			array_val = array_val << 4;
	        			array_val+= 10;
	        			break;
	        		case -7:
	        			array_val = array_val << 4;
	        			array_val+= 9;
	        			break;
	        		case -8:
	        			array_val = array_val << 4;
	        			array_val+= 8;
	        			break;

	        	}
	            // If the next token is a comma, ignore it and move on
	            if(ss.peek() == ',') ss.ignore();
	        }

	        //A[i * N + j] = 16;
	        //std::cout << i <<" "<< j << " " << array_val << " " << A[i * N + j] << std::endl;
	        A[i * SM + j] = array_val;
	        val_count++;
	        //std::cout << i <<" "<< j << " " << array_val << " " << A[i * N + j] << std::endl;

	    }
    }

    std::cout << "Val count " << val_count << std::endl;
}


static void load_arrays_tern(DTYPE *A,std::ifstream& myFile)
{

	// Create an input filestream
    //std::ifstream myFile("/mnt/weights_tern.csv");

	// Make sure the file is open
	if(!myFile.is_open()) throw std::runtime_error("Could not open tern file");

	// Helper vars
	std::string line;
	int val;
	int val_count=0;
	DTYPE array_val;

    for (int i = 0; i < SN; i++) {
    	// Read data, line by line
    	std::getline(myFile, line);

	    // Create a stringstream of the current line
	    std::stringstream ss(line);


        for (int j = 0; j < SM; j++) {

	        //fill one array val
        	array_val = 0;
	        for(int z =0; z< DTYPE_LENGTH/2; z++)
	        {
	        	// Extract each integer
	        	ss >> val;

	        	switch(val)
	        	{
	        		case 0:
	        			array_val = array_val << 2;
	        			break;
	        		case 1:
	        			array_val = array_val << 2;
	        			array_val+= 1;
	        			break;
	        		case -1:
	        			array_val = array_val << 2;
	        			array_val+= 3;
	        			break;
	        	}
	            // If the next token is a comma, ignore it and move on
	            if(ss.peek() == ',') ss.ignore();
	        }

	        //A[i * N + j] = 16;
	        //std::cout << i <<" "<< j << " " << array_val << " " << A[i * N + j] << std::endl;
	        A[i * SM + j] = array_val;
	        val_count++;
	        //std::cout << i <<" "<< j << " " << array_val << " " << A[i * N + j] << std::endl;

	    }
    }

    std::cout << "Val count " << val_count << std::endl;
}


static void load_arrays_byte(DTYPE *A,std::ifstream& myFile)
{

	// Create an input filestream
    //std::ifstream myFile("/mnt/weights_byte.csv");

	// Make sure the file is open
	if(!myFile.is_open()) throw std::runtime_error("Could not open byte file");

	// Helper vars
	std::string line;
	int val;
	int val_count=0;
	DTYPE array_val;

    for (int i = 0; i < SN; i++) {
    	// Read data, line by line
    	std::getline(myFile, line);

	    // Create a stringstream of the current line
	    std::stringstream ss(line);


        for (int j = 0; j < SM; j++) {

	        //fill one array val
        	array_val = 0;
	        for(int z =0; z< DTYPE_LENGTH/8; z++)
	        {
	        	// Extract each integer
	        	ss >> val;

	        	array_val = (array_val << 8) + val;

	            // If the next token is a comma, ignore it and move on
	            if(ss.peek() == ',') ss.ignore();
	        }

	        //A[i * N + j] = 16;
	        //std::cout << i <<" "<< j << " " << array_val << " " << A[i * N + j] << std::endl;
	        A[i * SM + j] = array_val;
	        val_count++;
	        //std::cout << i <<" "<< j << " " << array_val << " " << A[i * N + j] << std::endl;

	    }
    }

    std::cout << "Val count " << val_count << std::endl;

}

void mmult_golden_ternary(DTYPE *A,  DTYPE *B, DTYPE_OUT *C)
{
     for (int row = 0; row < SN; row++) {
          for (int col = 0; col < SP; col++) {
        	   DTYPE_OUT result = 0;
               for (int k = 0; k < SM; k++) {
       			for(int z = 0; z < DTYPE_LENGTH; z+=2) {
       				DTYPE A_temp1 = A[row*SM+k];
       				ap_int<2> A_val = A_temp1.range(z+1,z);
       				DTYPE A_temp = A_val;
       				ap_int<2> B_temp;
       				switch(A_temp)
       				{
       				 case 0:
       					 //std::cout << "C golden is" << result << std::endl;
       					 break;
       				 case 1:
       					 B_temp = B[k*SP+col].range(z+1,z);
       					 result += B_temp;
       					 //std::cout << "B is" << b[k][j].range(z+1,z) << std::endl;
       					 //std::cout << "C golden is" << result << std::endl;
       					 break;
       				 case -1:
       					 B_temp = B[k*SP+col].range(z+1,z);
       					 result -= B_temp;
       					 //std::cout << "B is" << b[k][j].range(z+1,z) << std::endl;
       					 //std::cout << "C golden is" << result << std::endl;
       					 break;
       				}
       			}
       			//std::cout << "result is " << result << std::endl;
               }
               C[row*SP+col] = result;
          }
     }
}

void mmult_golden_quad(DTYPE *A,  DTYPE *B, DTYPE_OUT *C)
{
     for (int row = 0; row < SN; row++) {
          for (int col = 0; col < SP; col++) {
        	   DTYPE_OUT result = 0;
               for (int k = 0; k < SM; k++) {
       			for(int z = 0; z < DTYPE_LENGTH; z+=4) {
       				DTYPE A_temp1 = A[row*SM+k];
       				ap_int<4> A_val = A_temp1.range(z+3,z);
       				DTYPE A_temp = A_val;
       				ap_int<4> B_temp;
  					B_temp = B[k*SP+col].range(z+3,z);
     				result+=A_val*B_temp;
       			}
       			//std::cout << "result is " << result << std::endl;
               }
               C[row*SP+col] = result;
          }
     }
}

void mmult_golden_byte(DTYPE *A,  DTYPE *B, DTYPE_OUT *C)
{
     for (int row = 0; row < SN; row++) {
          for (int col = 0; col < SP; col++) {
        	   DTYPE_OUT result = 0;
               for (int k = 0; k < SM; k++) {
       			for(int z = 0; z < DTYPE_LENGTH; z+=8) {
       				DTYPE A_temp1 = A[row*SM+k];
       				ap_int<8> A_val = A_temp1.range(z+7,z);
       				result+=A_val*B[k*SP+col];
       			}
               }
               C[row*SP+col] = result;
               //std::cout << row << " " << col << " result is " << result << std::endl;
          }
          //std::cout << row << " " << col << " result is " << result << std::endl;
     }
}

static int result_check(DTYPE_OUT *C, DTYPE_OUT *C_sw)
{
     for (int i = 0; i < SN * SP; i++) {
          if (C_sw[i] != C[i]) {
               std::cout << "Mismatch: data index= " << i << " golden = " << C_sw[i]
                         << ", kernel = " << C[i] << std::endl;
               return 1;
          }
          //else
        	//  std::cout << "out :data index= " << i << " golden = " << C_sw[i] << std::endl;
     }
     return 0;
}


void printVector(const vi& V, char* msg)
{

	std::cout << msg << "[ ";
	for_each(V.begin(), V.end(), [](int a) {
		std::cout << a << " ";
	});
	std::cout << "]" << std::endl;
}


// Generate the three vectors A, IA, JA
void arraytocsr(DTYPE *V)
{
	int i, j;
	vi A;
	vi IA = { 0 }; // IA matrix has N+1 rows
	vi JA;
	int NNZ = 0;

	// Create an outuptu filestream
	std::ofstream outFile("/mnt/weights.csr");

	// Make sure the file is open
	if(!outFile.is_open()) throw std::runtime_error("Could not open csr file");


	for (i = 0; i < SN; i++) {
		for (j = 0; j < SM; j++) {
			//std::cout << i <<" "<< j << " " << V[i*M+j] << std::endl;
			if (V[i*SM+j] != 0) {
				A.push_back(V[i*SM+j]);
				JA.push_back(j);

				// Count Number of Non Zero
				// Elements in row i
				NNZ++;
			}
		}
		IA.push_back(NNZ);
	}


	outFile << SN << " " << SM << " " << NNZ << std::endl;

	for(int i=0;i<A.size();i++)
	{
		outFile << JA[i] << " " << A[i] << std::endl;
		//std::cout << JA[i] << " " << A[i] << std::endl;
	}

	for(int i=0;i<IA.size();i++)
	{
		outFile << IA[i] << std::endl;
		//std::cout << IA[i] << std::endl;
	}

	outFile.close();
	std::cout << "Number of non-zeros " << NNZ << std::endl;
	//printVector(A, (char*)"A = ");
	//printVector(IA, (char*)"IA = ");
	//printVector(JA, (char*)"JA = ");
}

int mmult_test(uint ternary,DTYPE *A,  DTYPE *B, DTYPE_OUT *C_sw, DTYPE_OUT *C,std::ifstream & myFile)
{
     std::cout << "Testing " << NUM_TESTS << " iterations of " << SN << "x" << SM
               << " of " << ternary << std::endl;

     perf_counter hw_ctr, sw_ctr;
     
     for (int i = 0; i < NUM_TESTS; i++) 
     {

    	  if (ternary==0)
    	  {
    		  load_arrays_byte(A,myFile);
    	  }
    	  else if (ternary==1)
    	  {
    		  load_arrays_tern(A,myFile);
    	  }
    	  else
    	  {
    		  load_arrays_quad(A,myFile);
    	  }

          arraytocsr(A);
      	  double start_time, end_time, execution_time;

      	  //======================ONLY CPU ==========================================


      	  start_time = getTimestamp();
    	  init_arrays(B, C_sw, C);




          std::cout << "Kernel" << std::endl;
    	  start_time = getTimestamp();
          kernelMatrixmult1(ternary,A,B,C,SM,SP,0,SN);
          end_time = getTimestamp();

          execution_time = (end_time - start_time) / (1000);
          std::cout << "FPGA " << " Total execution time = " << execution_time << " msec" << std::endl;



    	  std::cout << "Golden" << std::endl;
        	  //std::cout << "A0  is" << A[0] << std::endl;

       	  start_time = getTimestamp();

	  if (ternary==0)
      {
          	mmult_golden_byte(A, B, C_sw);
	  }
	  else if (ternary==1)
	  {
          	mmult_golden_ternary(A, B, C_sw);
	  }
	  else
	  {
		    mmult_golden_quad(A, B, C_sw);
	  }

	  end_time = getTimestamp();

       	  execution_time = (end_time - start_time) / (1000);
       	  std::cout << "CPU " << " Total execution time = " << execution_time << " msec" << std::endl;








          if (result_check(C, C_sw))
               return 1;
     }

     return 0;
}

/**
 * Design principles to achieve performance
 *
 * 1. sds_alloc to guarantee physically contiguous buffer allocation
 *    that enables the most efficient DMA configuration (axidma_simple)
 */
int main(int argc, char* argv[]){
     int test_passed = 0;
     DTYPE *A, *B;
     DTYPE_OUT *C_sw, *C;

     if(argc<6)
     {
	std::cout << "Error not enough arguments " << std::endl;
	exit(1);
     }

     std::ifstream myFile(argv[1]);

     uint ternary = atoi(argv[2]);

     //SN,SM,SP
     SN = atoi(argv[3]);
     SM = atoi(argv[4]);
     SP = atoi(argv[5]);
     //if(ternary)
	//SM = 96;
    // else
	//SM = 384;



     A = (DTYPE *)sds_alloc(SN * SM * sizeof(DTYPE));
     B = (DTYPE *)sds_alloc(SM * SP * sizeof(DTYPE));
     C = (DTYPE_OUT *)sds_alloc(SN * SP * sizeof(DTYPE_OUT));
     C_sw = (DTYPE_OUT *)sds_alloc(SN * SP * sizeof(DTYPE_OUT));
     
     if (!A || !B || !C || !C_sw) {
          if (A) sds_free(A);
          if (B) sds_free(B);
          if (C) sds_free(C);
          if (C_sw) sds_free(C_sw);
          return 2;
     }


 	 //FILE *fp_input;

 	 //fp_input = fopen(argv[1], "r");

     test_passed = mmult_test(ternary,A, B, C_sw, C,myFile);
     
     std::cout << "TEST " << (test_passed ? "FAILED" : "PASSED") << std::endl;

     sds_free(A);
     sds_free(B);
     sds_free(C);
     sds_free(C_sw);
     
     return (test_passed ? -1 : 0);
}

