# gemm_spmm
 hardware accelerator for pruned nertworks

example commands to use the accelerators:

GEMM

./gem_neural_multi_zed_size.elf /mnt/weights_tern_090_block_1_1.csv 1 6 84  512

csv file with packed weights, mode 0 byte/1 ternary/2 quad, A matrix rows, A matrix columns , rows for B matrix

SPMM

./spmm_neural_multi_zed_wide_input.elf /mnt/weights_quad_block_095_1_1.csr 2

csr file with sparsed packed weights, mode 0 byte/1 ternary/2 quad, 

rows for B hardwired to 512, rows and columns for A part of the CSR file.
