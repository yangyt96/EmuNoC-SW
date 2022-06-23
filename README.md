# EmuNoC Software
This is the software part of the NoC hybrid emulation prototype - EmuNoC. Currently, it supports all-to-all NoC traffic, random traffic, uniform random traffic, netrace and multicore NoC-interconnect AI traffic workloads. You can add any other traffic that you like.

## Prerequisite
Please install Xilinx SDK for the cross compiler.

## How to use?
1. Edit the hardware configuration of NoC in "src/nocpe/nocpe.h" for different dimension of NoC.
2. Change the configured AXI DMA address in "src/xaxidma/xaxidma_sg.h" according to the implemented hardware design.
3. Uncomment in the makefile for the cross compile that you needs. This depends on which Zynq processor that you are using.
4. Activate the Xilinx SDK package
5. Type "make" in the terminal
6. Move/Copy the compiled shared object to the Linux on FPGA that your are using.

## TODO
1. Update the linked list package for better memory allocation (xor list)