# gemm_spmm
Hardware accelerator for matrix multiplication in dense and pruned networks

The GEMM and SPMM folders contain the source code for the paper targetting a baremetal zedboard platform.

Jose Nunez-Yanez, Mohammad Hosseinabady, Sparse and dense matrix multiplication hardware for heterogeneous multi-precision neural networks,Array,Volume 12,
2021,ISSN 2590-0056

In this paper, we present hardware accelerators created with high-level synthesis techniques for sparse and dense matrix multiplication operations. 
The cores can operate with different precisions and are designed to be integrated in a heterogeneous CPU-FPGA system for Edge AI applications. 
The methodology involves quantization-sparsity aware training and it is applied to a case study consisting of human activity classification. 
We initially investigate the effects of quantization and sparsity on the accuracy of neural networks with convolution, dense and recurrent layers 
observing better tolerance to pruning when recurrent layers are present. 
Then, we propose the hardware accelerators that can switch precision at run-time and work with any matrix size up to a maximum configured at compile time. 
We compare the performance of these accelerators at different levels of precision and sparsity levels and create a performance model to enable 
workload balancing. The results show that the proposed sparse matrix multipliers can outperform dense multipliers when sparsity levels are 
higher than 70% and the improvements are more evident when higher precision arithmetic or structural pruning is used. 
Additionally, sparsity levels as high as 99% can maintain the level of accuracy required in the network especially when recurrent layers are deployed. 
Overall, the balance between sparse and dense performance depends on matrix shape, precision, structural pruning and sparsity levels and performance 
modelling can be used to balance concurrent execution in a heterogeneous configuration.

example commands to use the accelerators:

GEMM

./gem_neural_multi_zed_size.elf /mnt/weights_tern_090_block_1_1.csv 1 6 84  512

csv file with packed weights, mode 0 byte/1 ternary/2 quad, A matrix rows, A matrix columns , rows for B matrix

SPMM

./spmm_neural_multi_zed_wide_input.elf /mnt/weights_quad_block_095_1_1.csr 2

csr file with sparsed packed weights, mode 0 byte/1 ternary/2 quad, 

rows for B hardwired to 512, rows and columns for A part of the CSR file.

The FUSED folder contains a higher performance architecture that merges the sparse and dense accelerators and targets a ZCU102 Zynq Ultrascale platform.

It uses a streaming/dataflow architecture with read,compute,scale and write stages and can switch operation between sparse and dense modes using a mode register.
It has been integrated as part of a Tensorflow Lite (1.15/2.8) inference engine running on the device PS that offloads matrix operations to the accelerator as required.
The scaling engine follows the scale specifications part of tensorflow Lite. The hardware also supports multi-core configurations where the user can select to split the weight or activation matrices depending on matrix shape.
Tensorflow is used to obtain sparse matrices using the model optimization toolflow. The FUSED core deploys 8-bit precision that is currently the preferred option in Tensorflow Lite

Inside FUSED the PL folder contains the source code and compile binaries to deploy the accelerators on the zcu102 board running Linux. 

PL hardware development:

You can run the provided makefile in your computer with the SDS++ Xilinx compiler to create the bitstream for the FPGA device and a hardware library that can be linked with the Tensorflow Lite inference engine.
Your hardware development system needs to have the Xilinx SDX tools installed and running the make_silent.sh script will call the Makefile. After succesful completion
you can find the hardware library libkernelmatrixmult.a in the source directory and kernelmatrixmult.linked.bit.bin in /_sds/p0/sd_card directory.

You need to copy these two files to the ZCU102 board as indicated below. 

PS software development:

In our system we have used Petalinux to create an Ubuntu based OS for the ZCU102 board and compile natively Tensorflow Lite. 
You can download tensorflow 1.14/15 and unzip in the ZCU102 board sdcard. Then you can download the lite.zip provided file and obtain a lite directory that you use to replace the lite
directory in .../tensorflow-14.1/tensorflow/lite. Then you copy the libkernelmatrixmult.a and kernelmatrixmult.linked.bit.bin to 
.../tensorflow-14.1/tensorflow/lite/tools/make/LibMatrixmult and .../tensorflow-14.1/tensorflow/lite/tools/make/ respectively. 

Now you are ready to compile the inference engine, hardware library and linked the image classification example with the script ./build_arch64_lib.sh.
Once this completes you can execute the run.sh script that will load the PL with the accelerator bitstream and launch the inference engine example with:

./gen/aarch64_armv8-a/bin/label_image  --tflite_model ./pruned_quant_mobilenet_085.tflite --image ./3.bmp --labels ./labelsclas.txt --threads 1 --fpga_cores 1

The entry point to the hardware is located in tensorflow/lite/kernels/cpu_backend_gemm_ruy.h that calls the hardware at 

kernelmult1(fpga_cores,mode,quantized_multiplier,shift,bias,0,zero_point_lhs,zero_point_rhs,zero_point_dst,clamp_max,clamp_min,A,B,C,array_values,array_colIndices,array_rowPtr,nnz,ruy_lhs.layout.rows,ruy_lhs.layout.cols/4,ruy_rhs.layout.cols);

to test the sparse accelerator modify 

//hardware control
#define SPMM_MODE 0
//#define  GET_CSR 1

to 

//hardware control
#define SPMM_MODE 1
#define  GET_CSR 1

in the cpu_backend_gemm_ruy.h file



     

