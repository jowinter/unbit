# Xilinx FPGA Bitstream Manipulation Tools for Zynq7 (XC7Z010/015/020) FPGAs
![CMake CI](https://github.com/jowinter/unbit/actions/workflows/c-cpp.yml/badge.svg)

This small (and very experimetal) project implements tooling to manipulate the content
of block RAMs (RAMB36) of several Xilinx Zynq-7000 FPGAs (XC7Z010/015/020).

The tools provided in the `tools/` subdirectory take an FPGA bitstream (.bit) as input and
provide capabilities to dump and manipulate the block RAMs contained in this bitstream:

- `unbit-dump-bitstream` parses the bitstream and dumps information about the configuration
  packets in the stream.

- `unbit-dump-brams` extracts BRAM content form the bitstream and produces a textual dump
  similar to the `INIT` and `INITP` parameter strings used to initialize BRAM content in
  Verilog instantiations of the BRAM primitives.

- `unbit-substitute-brams` extracts BRAM content from an FPGA readback file (obtained from
  a live FPGA) into a configuration bitstream (.bit). This tool is highly experimental - use
  at your own risk and discretion. Intended usage of this tool is to provide a general-purpose
  method for BRAM initialization in design with a CPU (e.g. ARM's Cortex-M1 or M4 FPGA design start kits),
  assuming that debug access to the CPU is available.
