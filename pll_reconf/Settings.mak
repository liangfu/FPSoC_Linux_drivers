# ==================== COMPILATION RELATED SETTINGS ====================
# Path to the kernel sources (from "./driver", if relative path is used)
KSOURCE_DIR=../../../linux-socfpga

# Cross compiler "prepend" string
CROSS_COMPILE=arm-linux-gnueabihf-

# Architecture
ARCH=arm

# Compile with debug information
DEBUG_PLL_CTL=0

# ==================== DRIVER RELATED SETTINGS ====================
# Node name used in "/dev" folder
DRIVER_NODE_NAME="pll_control"

# Compatibility string found in device tree blob.
COMPATIBLE_STRING="pll_ctl"

# Unique (across system) ioctl magic number. Every ioctl interface should have one.
PLL_IOC_MAGIC=0xf0
