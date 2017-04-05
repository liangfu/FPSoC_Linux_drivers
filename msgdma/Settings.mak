# ==================== COMPILATION RELATED SETTINGS ====================
# Path to the kernel sources (from "./driver", if relative path is used)
KSOURCE_DIR=../../../linux-socfpga

# Cross compiler "prepend" string
CROSS_COMPILE=arm-linux-gnueabihf-

# Architecture
ARCH=arm

# Compile with debug information
MSGDMA_DEBUG=0

# ==================== DRIVER RELATED SETTINGS ====================
# Base name for all devices/classes
DRIVER_NODE_NAME="msgdma"

# compatible string in .dtb file by witc, this driver is recognized
COMPATIBLE_STRING="msgdma"

# Magic number for msgdma ioctl interface
MSGDMA_IOCTL_MAGIC=0xf1

