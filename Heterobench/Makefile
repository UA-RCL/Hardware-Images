PLATFORM := xilinx_u280_gen3x16_xdma_1_202211_1
KERNEL_CONFIG_FILE :=xclbin_overlay.cfg
BUILD_DIR := ./build

VPP_TEMP_DIRS :=vpp_temp_dir
VPP_LOG_DIRS :=vpp_log_dir

XCLBIN := heterobench_binary.xclbin

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
VPPFLAGS += --platform $(PLATFORM) -R1 --save-temps
VPPFLAGS += --temp_dir $(BUILD_DIR)/$(VPP_TEMP_DIRS)
VPPFLAGS += --log_dir $(BUILD_DIR)/$(VPP_LOG_DIRS)

create_dirs: 
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/$(VPP_TEMP_DIRS)
	mkdir -p $(BUILD_DIR)/$(VPP_LOG_DIRS)

############## Kernel XO and Xclbin File Generation

# Link Kernel
$(BUILD_DIR)/$(XCLBIN):
	v++ $(VPPFLAGS) --config $(KERNEL_CONFIG_FILE) -l -o $@ conv2d/build/conv2d.xo max_pooling/build/max_pooling.xo relu/build/relu.xo softmax/build/softmax.xo

xclbin: $(BUILD_DIR)/$(XCLBIN)

clean:
	rm -rf $(BUILD_DIR)
	rm -rf hls
