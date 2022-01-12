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
    

