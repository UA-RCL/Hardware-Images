PLATFORM := xilinx_u280_gen3x16_xdma_1_202211_1
BUILD_DIR := ./build

VPP_TEMP_DIRS :=vpp_temp_dir
VPP_LOG_DIRS :=vpp_log_dir

############## Kernel Source Files  Dependencies
KERNEL_SRC_CPP := ./softmax.cpp
KERNEL_SRC_HEADERS := ../include/fpga_impl.h

############## Check the version of gcc avaiable and select
ifneq ($(shell expr $(shell g++ -dumpversion) \>= 5), 1)
   ifndef XILINX_VIVADO
        $(error [ERROR]: g++ version older. Please use 5.0 or above.)
   else
      CXX := $(XILINX_VIVADO)/tps/lnx64/gcc-6.2.0/bin/g++
      $(warning [WARNING]: g++ version is older. Using g++ provided by the Vitis tool : $(CXX))
   endif
endif

############## Kernel Compiler and Linker Flags
# VPPFLAGS := -t $(TARGET)
VPPFLAGS += --platform $(PLATFORM) -R1 --save-temps -I../include
VPPFLAGS += --temp_dir $(BUILD_DIR)/$(VPP_TEMP_DIRS)
VPPFLAGS += --log_dir $(BUILD_DIR)/$(VPP_LOG_DIRS)
# VPPFLAGS += --profile.data all:all:all:all
# VPPFLAGS += --profile.trace_memory $(TRACE_DDR)
# ifeq ($(ENABLE_STALL_TRACE),yes)
# 	VPPFLAGS += --profile.stall all:all:all
# endif

create_dirs: 
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/$(VPP_TEMP_DIRS)
	mkdir -p $(BUILD_DIR)/$(VPP_LOG_DIRS)

############## Kernel XO and Xclbin File Generation
# Compile Kernel 
$(BUILD_DIR)/softmax.xo: ${KERNEL_SRC_CPP}
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/$(VPP_TEMP_DIRS)
	mkdir -p $(BUILD_DIR)/$(VPP_LOG_DIRS)
	v++ $(VPPFLAGS) -c -k softmax ${KERNEL_SRC_HEADERS} ${KERNEL_SRC_CPP} -o $@

xo: $(BUILD_DIR)/softmax.xo

clean:
	rm -rf $(BUILD_DIR)
	rm -rf hls
