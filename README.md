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

## Example: Editing the ROM of an embedded CPU ##

This example flow assumes a simple CPU-based design with an initial bitstream (`original.bit`)
and a matching memory map information file (`cpu.mmi`) being available. The memory map information
file is expected to provide process address space information (`<Processor>` and `<AddressSpace>`
tags).

The fragment below shows part of an MMI file for a custom RISC-V based design. The design
includes a 64k ROM (32-bit wide data bus) in the 0x10000000--0x1000FFFF address range. The
full MMI file was generated via a custom TCL script. The underlying BRAM structure was instantiated
via the `xpm_memory_sprom` macro in HDL code:

``` xml
<?xml version="1.0" encoding="UTF-8"?>
<MemInfo Version="1" Minor="5">
 <Processor Endianness="Little" InstPath="u_copro/u_rv32">
  <AddressSpace Name="rom" Begin="268435456" End="268500991">
   <BusBlock>
	 <BitLane MemType="RAMB36" Placement="X3Y21">
	  <DataWidth    MSB="7" LSB="6" />
	  <AddressRange Begin="0" End="16383" />
	  <Parity       ON="false" NumBits="0" />
	 </BitLane>
	 <!-- Remaining MMI file omitted for simplicity -->
  </AddressSpace>
 </Processor>
</MemoryInfo>
```

Typically the first step for editing the ROM of an embedded CPU is to extract the CPU-visible
memory as Intel-Hex file. Given the preliminaries shown above this can be accomplished with
the `unbit-dump-image` command. The command requires the original bitstream (`original.bit`),
the memory map information (`cpu.mmi`) and the instance path of the processor of
interest (`u_copro/u_rv32`) as inputs. Output can be written to `rom-original.hex` via a
standard output redirection:

``` shell
$ unbit-dump-image original.bit cpu.mmi u_copro/u_rv32 > rom-original.hex
```

Once the ROM has been extracted, the Intel-Hex file can be edited/replaced with any suitable
text editor. For the next it is assumed that the new ROM image is available in `rom-updated.hex`.
In the `unbit-inject-image` tool is used to inject the updated ROM image into the bitstream,
producing a new bitstream called `updated.bit`. The image injection tool can be used as shown
below to produce the updated bitstream:

``` shell
$ unbit-inject-image updated.bit original.bit cpu.mmi u_copro/u_rv32 rom-updated.hex
```

The updated bitstream can be inspected with the `unbit-dump-bitstream` and the `unbit-dump-brams`
tools. Output of the `unbit-dump-brams` tool for the original and updated bitstream can be,
for example, used to compare the underlying BRAM initialization strings.

*CAVEAT EMPTOR*: At this stage an FPGA will does not yet accept the updated bitstream
(Configuration attempts *will* fail with `INIT_B` staying low). The root cause can be
identified by comparing the output of `unbit-dump-bitstream` between the original and
updated bitstreams: Both bitstreams (typically) contain configuration packets that instruct
the FPGA to perform CRC checks over the configuration stream. These CRC checks fail for
the updated bitstream, as the `unbit-inject-image` command does not make any attempts
to recalculate the CRC.

The updated bitstream can be fixed easily, be completely stripping the CRC checks, via
the `unbit-strip-crc-checks` tool. The next step below shows an example invocation of
this tool with `updated.bit` as input, and `updated-nocrc.bit` as output (different
file names have been used for illustrative purposes only; the tool properly handles
the same input/output file name):

``` shell
$ unbit-strip-crc-checks updated-nocrc.bit updated.bit
```

The resulting bitstream (`updated-nocrc.bit`) has the CRC checks removed, and can
be used to (re-)configure an FPGA with a design containing the modified ROM image.

## Notes ##

### Adding FPGAs ###

Additional Virtex-7 style FPGAs can be added with relative ease. The `xc7z020` class should provide
a good starting point for what is minimally needed. The first step typically is to identify the
location of the BRAM configuration frames for the FPGA of interest. This can be achieved by
implementing a "stub" design that uses all RAMB36 primitives of the FPGA, and requesting generation
of a logic location file as part of the `write_bitstream` task. See the comments in `ramb36e1.cpp`
for the approach that was used with currently available FPGA types.

*NOTE* The laziest approach to realize a mapping between the BRAM bits and the bitstream offsets
is to literally tabulate bit offsets found in the logic location file for each BRAM bits. (Tables
get huge but are straightforward to (auto-generate).

The BRAM mapping tables in `ramb36e1.cpp` and `ramb36e2.cpp` were generated with the second
laziest approach: First take an educated guess (all BRAMs have the same internal structure),
then think of a simple formula for mapping bit `x` of the BRAM to an offset relative the start
of the RAM, and finally let an SMT solver (z3) figure out the remaining parameters ... ;)
