/**
 * @file
 * @brief Common infrastructure for Xilinx Series-7 FPGAs
 *
 * @bug HIGHLY EXPERIMENTAL CODE (READ: HACK!)
 */
#ifndef TOOLS_FPGA_FAMILY_HPP_
#define TOOLS_FPGA_FAMILY_HPP_ 1

/// @todo Refactor (to allow more generic access)
#if defined(EXPERIMENTAL_VIRTEX_UP) && (EXPERIMENTAL_VIRTEX_UP != 0)

#include "fpga/xilinx/vup.hpp"

//
// Virtex UltraScale+ build
//
using fpga::xilinx::vup::virtex_up;
typedef virtex_up fpga_family;

#define FPGA_BUILD_TAG "vup"

#else

//
// Zynq-7 build
//
#include "fpga/xilinx/zynq7.hpp"
using fpga::xilinx::v7::zynq7;
typedef zynq7 fpga_family;

#define FPGA_BUILD_TAG "zynq7"


#endif

#endif // #ifndef TOOLS_FPGA_FAMILY_HPP_
