PLATFORM := xilinx_u280_gen3x16_xdma_1_202211_1
KERNEL_CONFIG_FILE :=krnl_build_options.cfg
BUILD_DIR := ./build

VPP_TEMP_DIRS :=vpp_temp_dir
VPP_LOG_DIRS :=vpp_log_dir

XCLBIN := binary.xclbin

############## Kernel Source Files  Dependencies
# KERNEL_SRC_CPP := ./filter2d_hw.cpp ./krnl_vadd.cpp
KERNEL_SRC_HEADERS := ./common.h
# KERNEL_INCLUDES := -I$(SRC_REPO)

############## Check the version of gcc avaiable and select
ifneq ($(shell expr $(shell g++ -dumpversion) \>= 5), 1)
   ifndef XILINX_VIVADO
        $(error [ERROR]: g++ version older. Please use 5.0 or above.)
   else
      CXX := $(XILINX_VIVADO)/tps/lnx64/gcc-6.2.0/bin/g++
      $(warning [WARNING]: g++ version is older. Using g++ provided by the Vitis tool : $(CXX))
   endif
endif

############## Set "HOST" Compiler Paths and Flags
# CXXFLAGS += -I$(XILINX_XRT)/include/
# CXXFLAGS += -I$(XILINX_VIVADO)/include/
# ifneq ($(INPUT_TYPE), random)
# CXXFLAGS += -I$(OPENCV_INCLUDE)/
# endif
# CXXFLAGS += -I$(SRC_REPO)
# CXXFLAGS += -O3 -Wall -fmessage-length=0 -std=c++14

############## Set "HOST" Set Linker Paths and Flags
# CXXLDFLAGS := -L$(XILINX_XRT)/lib/
# ifneq ($(INPUT_TYPE),random)
# CXXLDFLAGS += -L$(OPENCV_LIB)/
# endif
# CXXLDFLAGS += -lOpenCL -pthread -lrt -lstdc++ -lxilinxopencl -fopenmp
# ifneq ($(INPUT_TYPE),random)
# CXXLDFLAGS += -Wl,-rpath=$(OPENCV_LIB)/ -lopencv_core -lopencv_highgui
# endif

############## Kernel Compiler and Linker Flags
# VPPFLAGS := -t $(TARGET)
VPPFLAGS += --platform $(PLATFORM) -R1 --save-temps
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
#Compile Kernel 
$(BUILD_DIR)/Filter2DKernel.xo: ./filter2d_hw.cpp $(KERNEL_SRC_HEADERS)
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/$(VPP_TEMP_DIRS)
	mkdir -p $(BUILD_DIR)/$(VPP_LOG_DIRS)
	v++ $(VPPFLAGS) -c -k Filter2DKernel  $(KERNEL_INCLUDES) ./filter2d_hw.cpp -o $@

$(BUILD_DIR)/krnl_vadd.xo: ./krnl_vadd.cpp $(KERNEL_SRC_HEADERS)
	v++ $(VPPFLAGS) -c -k krnl_vadd $(KERNEL_INCLUDES) ./krnl_vadd.cpp -o $@

# Link Kernel
$(BUILD_DIR)/$(XCLBIN): $(BUILD_DIR)/Filter2DKernel.xo $(BUILD_DIR)/krnl_vadd.xo
	v++ $(VPPFLAGS) --config $(KERNEL_CONFIG_FILE) -l -o $@ $(BUILD_DIR)/Filter2DKernel.xo $(BUILD_DIR)/krnl_vadd.xo

xclbin: $(BUILD_DIR)/$(XCLBIN)

clean:
	rm -rf $(BUILD_DIR)
	rm -rf hls
