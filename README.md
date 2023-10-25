# Xilinx FPGA Bitstream Manipulation Tools for Zynq7 (XC7Z010/015/020) FPGAs
![CMake CI](https://github.com/jowinter/unbit/actions/workflows/c-cpp.yml/badge.svg)

This small project implements tooling to manipulate the content of block RAMs of several
Xilinx Zynq-7000 FPGAs (XC7Z010/015/020). (Very experimental) support for the
Virtex UltraScale+ (XCVU9P) FPGA is available.

The tools provided in the `tools/` subdirectory enable various types of manipulation on
FPGA bitstream (.bit) files and on (binary) FPGA readback data. The following command
line tools are currently available:

- `unbit-dump-bitstream` parses a bitstream and dumps information about the configuration
  packets in the bitstream to the standard output.

- `unbit-dump-brams` extracts BRAM content from the bitstream and produces a textual dump
  similar to the `INIT` and `INITP` parameter strings used to initialize BRAM content in
  Verilog instantiations of the BRAM primitives.

- `unbit-substitute-brams` extracts BRAM content from an FPGA readback file (obtained from
  a live FPGA) into a configuration bitstream (.bit). This tool is highly experimental - use
  at your own risk and discretion. Intended usage of this tool is to provide a general-purpose
  method for BRAM initialization in design with a CPU (e.g. ARM's Cortex-M1 or M4 FPGA design start kits),
  assuming that debug access to the CPU is available.

- `unbit-dump-image` extracts BRAM content from a bitstream into an Intel-Hex file. The Intel-Hex
  file shows an embedded processor's view of a memory address space. A memory map information (MMI)
  file with address space information is required as input. This tool can be used to extract the
  content of embedded RAMs and ROMs.

- `unbit-inject-image` updates BRAM content in a bitstream from an Intel-Hex file. A memory map
  information (MMI) file with address space information, and an Intel-Hex file are required as input.
  This tool is the dual to the `unbit-dump-image`tool. This tool can be used to replace/edit the
  content of embedded RAMs and ROMs.

- `unbit-bitstream-to-readback` simulates configuration readback from a configured FPGA. This
  tool takes a bitstream as input and produces a binary readback data file as output.

- `unbit-strip-crc-checks` removes all configuration CRC check commands from a bitstream. This
  tool is required to allow configuration of an FPGA with bitstreams that have been edited
  by the `unbit-substitute-brams` or `unbit-inject-image` tools.

## Build Environment ##

The "unbit" toolset uses simple CMake-based build setup. Any reasonably recent (C++17 support)
Clang or GCC toolchain should work as build environment. The majority of the tools only depend on
the Standard C++ Library. The `unbit-dump-image` and `unbit-inject-image` tools additionally require
libxml2 for parsing memory map information (mmi) files.

Typical setup on a Debian-based operating system (using GCC):

``` shell
$ sudo apt-get install build-essential cmake g++ libxml2-dev
```

Typical build invocation:
```
$ mkdir build/
$ cmake -B build/ .
$ cmake -DCMAKE_BUILD_TYPE=Release --build build/
```

Tools can be run directly from the build directory (`build/src/tools/`). Alterntively,
an `install` target is provided (The install target honors CMake's `CMAKE_INSTALL_PREFIX`
variable).
