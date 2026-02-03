# -----------------------------------------------------------------------------
# Setup and Paths
# -----------------------------------------------------------------------------
PLATFORM   := xilinx_u280_gen3x16_xdma_1_202211_1
BUILD_DIR  := ./build
VITIS_LIBS := ./Vitis_Libraries

VPP_TEMP_DIRS := vpp_temp_dir
VPP_LOG_DIRS  := vpp_log_dir

# Kernel Configuration
KERNEL_NAME := fft_kernel
TARGET      := hw
XO_OUTPUT   := $(BUILD_DIR)/$(KERNEL_NAME).xo
SRC_FILES   := fft_kernel.cpp

# -----------------------------------------------------------------------------
# Tool Check: GCC Version Selection
# -----------------------------------------------------------------------------
ifneq ($(shell expr $(shell g++ -dumpversion) \>= 5), 1)
   ifndef XILINX_VIVADO
        $(error [ERROR]: g++ version older. Please use 5.0 or above.)
   else
      CXX := $(XILINX_VIVADO)/tps/lnx64/gcc-6.2.0/bin/g++
      $(warning [WARNING]: g++ version is older. Using g++ provided by Vitis: $(CXX))
   endif
endif

# -----------------------------------------------------------------------------
# Compiler Flags
# -----------------------------------------------------------------------------
VPP := v++

# Include Paths
INCLUDES := -I$(VITIS_LIBS)/dsp/L1/include/hw/vitis_fft/float \
            -I.

# Base VPP Flags
VPP_FLAGS += -t $(TARGET) --platform $(PLATFORM) -R1 --save-temps
VPP_FLAGS += --temp_dir $(BUILD_DIR)/$(VPP_TEMP_DIRS)
VPP_FLAGS += --log_dir $(BUILD_DIR)/$(VPP_LOG_DIRS)
VPP_FLAGS += $(INCLUDES)

# -----------------------------------------------------------------------------
# Targets
# -----------------------------------------------------------------------------
.PHONY: all clean xo create_dirs

all: xo

# Create build directories
create_dirs: 
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/$(VPP_TEMP_DIRS)
	mkdir -p $(BUILD_DIR)/$(VPP_LOG_DIRS)

# Compile the XO file
# Using '|' makes create_dirs an order-only prerequisite
xo: $(XO_OUTPUT)

$(XO_OUTPUT): $(SRC_FILES) | create_dirs
	@echo "Compiling Vitis Kernel: $(KERNEL_NAME)..."
	$(VPP) $(VPP_FLAGS) -c -k $(KERNEL_NAME) $(SRC_FILES) -o $@
	@echo "Success! Generated: $@"

# Clean up build artifacts
clean:
	rm -rf *.xo *.log *.jou .Xil _x hls $(BUILD_DIR)