LIBRARY = libkernelMatrixmult.a

#PLATFORM = /home/programs/Xilinx2/SDx/2017.1/platforms/zcu102_es2_ig/zcu102_es2

PLATFORM = zcu102

#SDSFLAGS = -xp "param:compiler.skipTimingCheckAndFrequencyScaling=1" -sds-proc a53 -sds-pf ${PLATFORM} -dmclkid 3\
#	 -sds-hw    mmult_top kernelMatrixmult.cpp  -clkid 4  -sds-end\
#	-Wno-unused-label -Wno-unused-function

SDSFLAGS = -xp "param:compiler.skipTimingCheckAndFrequencyScaling=1" -sds-proc a53 -sds-pf ${PLATFORM} \
	 -sds-hw    mmult_top kernelMatrixmult.cpp  -clkid 3  -sds-end\
	-Wno-unused-label -Wno-unused-function

#SDSFLAGS = -xp "param:compiler.skipTimingCheckAndFrequencyScaling=1" -sds-proc a53 -sds-pf ${PLATFORM} \
#	 -sds-hw  mmult_top matrix_mult_accel.cpp  -clkid 4 -sds-end\
#	 -sds-hw  spmm_wrapper spmm.cpp  -clkid 4 -sds-end\
#	-Wno-unused-label -Wno-unused-function

#SDSFLAGS =  -xp "param:compiler.skipTimingCheckAndFrequencyScaling=1" -sds-proc a53 -sds-pf ${PLATFORM} \
#          -sds-hw  mmult_top matrix_mult_accel.cpp  -clkid 4 -sds-end\
#    	  -Wno-unused-label -Wno-unused-function

#SDSFLAGS =  -xp "param:compiler.skipTimingCheckAndFrequencyScaling=1" -sds-proc a53 -sds-pf ${PLATFORM} \
#	-sds-hw  spmm_wrapper spmm.cpp  -clkid 4 -sds-end\
#	-Wno-unused-label -Wno-unused-function

#SDSFLAGS =  -xp "param:compiler.skipTimingCheckAndFrequencyScaling=1" -sds-proc a53 -sds-pf ${PLATFORM} \
#	-sds-hw  dummy_add dummy.cpp  -clkid 4 -sds-end\
#	-Wno-unused-label -Wno-unused-function


#SDSFLAGS =  -synth-strategy Flow_AlternateRoutability  -synth-strategy Flow_PerfOptimized_high  -impl-strategy Area_ExploreWithRemap -sds-proc a53 -sds-pf ${PLATFORM} \
#	 -sds-hw  mmult_top matrix_mult_accel.cpp  -clkid 4 -sds-end\
#	-Wno-unused-label -Wno-unused-function


# 0 74.992500
# 1 99.990000
# 2 149.985000
# 3 199.980000
# 4 299.970000
# 5 399.960000
# 6 599.940000

#synth strategy
#Flow_AreaOptimized_high
#Flow_AreaOptimized_medium
#Flow_AreaMultThresholdDSP
#Flow_AlternateRoutability
#Flow_PerfOptimized_high
#Flow_PerfThresholdCarry
#Flow_RuntimeOptimized



DEPS = matrix_mult.h 


LDFLAGS = -pthread

SOURCE= kernelMatrixmult.h

XI_LDFLAGS+= -lrt -lkernelMatrixmult

CC = sds++ ${SDSFLAGS}

#CC = v++ ${SDSFLAGS}

CFLAGS = -Wall -O3 -c -fPIC
LFLAGS = -O3

XI_CFLAGS = --std=gnu++11 -I $(LIB_gemm)

.PHONY: all

all: ${LIBRARY}

kernelMatrixmult_sw.o: kernelMatrixmult_sw.cpp
	${CC} ${CFLAGS} $< -o $@

kernelMatrixmult.o: kernelMatrixmult.cpp
	${CC} ${CFLAGS} $< -o $@

# Create dummy linked file to build the hardware
kernelMatrixmult.linked:  kernelMatrixmult.o 
	${CC} $^ -nostartfiles -o $@ 

${LIBRARY}: kernelMatrixmult.linked kernelMatrixmult_sw.o
	cp _sds/swstubs/${LIBRARY} .
	arm-linux-gnueabihf-ar rcs ${LIBRARY} _sds/swstubs/kernelMatrixmult.o kernelMatrixmult_sw.o
	ranlib libkernelMatrixmult.a


clean:
	${RM} ${LIBRARY} matrix_mult_accel.o spmm.o kernelMatrixmult.o kernelMatrixmult.linked

ultraclean: clean
	${RM}  kernelMatrixmult.linked.bit 
	${RM} -rf _sds sd_card

# LocalWords:  cpp arm-linux-gnueabihf-ar rcs libkernelMatrixmult.a _sds/swstubs/kernelMatrixmult.o


