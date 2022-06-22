# EmuNoC Software
This is the software part of the NoC hybrid emulation prototype - EmuNoC.

## How to use?
1. Remember to edit the hardware configuration of NoC in "src/nocpe/nocpe.h" for different dimension of NoC.
2. Change the configured AXI DMA address in "src/xaxidma/xaxidma_sg.h" according to the implemented hardware design.
3. Uncomment in the makefile for the cross compile that you needs. This depends on which Zynq processor that you are using.

## TODO
1. Update the linked list package for better memory allocation (xor list)