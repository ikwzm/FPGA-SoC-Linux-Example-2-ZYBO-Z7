FPGA-SoC-Linux-Example-2-ZYBO-Z7
================================

## Intrduction

FPGA-SoC-Linux example(2) binary and project and test code for ZYBO-Z7

### Requirement

* Board: ZYBO-Z7
* OS: [FPGA-SoC-Linux](https://github.com/ikwzm/FPGA-SoC-Linux.git)

### Reference

* https://lauri.xn--vsandi-pxa.com/hdl/zynq/xilinx-dma.html

## Install

### Download FPGA-SoC-Linux-Example-2-ZYBO-Z7

```
shell$ git clone https://github.com/ikwzm/FPGA-SoC-Linux-Example-2-ZYBO-Z7
shell$ cd FPGA-SoC-Linux-Example-2-ZYBO-Z7
```

### Install to FPGA and Device Tree

```
shell# rake install
cp loopback-dma.bin /lib/firmware/loopback-dma.bin
dtbocfg.rb --install loopback-dma --dts loopback-dma.dts
<stdin>:30.57-37.14: Warning (unit_address_vs_reg): /fragment@1/__overlay__/axidma@40400000/dma-channel@40400000: node has a unit name, but no reg property
<stdin>:39.57-46.7: Warning (unit_address_vs_reg): /fragment@1/__overlay__/axidma@40400000/dma-channel@40400030: node has a unit name, but no reg property
<stdin>:9.13-62.4: Warning (avoid_unnecessary_addr_size): /fragment@1: unnecessary #address-cells/#size-cells without "ranges" or child "reg" property
[  987.295033] fpga_manager fpga0: writing loopback-dma.bin to Xilinx Zynq FPGA Manager
[  987.369880] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /amba/fpga-region0/firmware-name
[  987.383069] fclkcfg amba:fclk0: driver installed.
[  987.387790] fclkcfg amba:fclk0: device name    : fclk0
[  987.395130] fclkcfg amba:fclk0: clock  name    : fclk0
[  987.400272] fclkcfg amba:fclk0: clock  rate    : 99999999
[  987.407654] fclkcfg amba:fclk0: clock  enabled : 1
[  987.428443] udmabuf udmabuf-tx: driver version = 1.4.2
[  987.433768] udmabuf udmabuf-tx: major number   = 244
[  987.438748] udmabuf udmabuf-tx: minor number   = 0
[  987.444227] udmabuf udmabuf-tx: phys address   = 0x30100000
[  987.449799] udmabuf udmabuf-tx: buffer size    = 1048576
[  987.455297] udmabuf udmabuf-tx: dma coherent   = 0
[  987.460085] udmabuf amba:udmabuf-tx: driver installed.
[  987.477466] udmabuf udmabuf-rx: driver version = 1.4.2
[  987.482686] udmabuf udmabuf-rx: major number   = 244
[  987.487642] udmabuf udmabuf-rx: minor number   = 1
[  987.492535] udmabuf udmabuf-rx: phys address   = 0x30200000
[  987.498102] udmabuf udmabuf-rx: buffer size    = 1048576
[  987.503461] udmabuf udmabuf-rx: dma coherent   = 0
[  987.508250] udmabuf amba:udmabuf-rx: driver installed.
```

## Run loopback-dma-test

### Compile loopback-dma-test

```
shell# rake loopback-dma-test
```

### Run loopback-dma-test

```
shell# ./loopback-dma-test
Source memory block:      44332211 00000000 00000000 00000000 00000000 00000000 00000000 00000000
Destination memory block: 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
Resetting DMA
Stream to memory-mapped status (0x00000001@0x34): halted
Memory-mapped to stream status (0x00000001@0x04): halted
Halting DMA
Stream to memory-mapped status (0x00000001@0x34): halted
Memory-mapped to stream status (0x00000001@0x04): halted
Writing destination address
Stream to memory-mapped status (0x00000001@0x34): halted
Writing source address...
Memory-mapped to stream status (0x00000001@0x04): halted
Starting S2MM channel with all interrupts masked...
Stream to memory-mapped status (0x00000000@0x34): running
Starting MM2S channel with all interrupts masked...
Memory-mapped to stream status (0x00000000@0x04): running
Writing S2MM transfer length...
Stream to memory-mapped status (0x00000000@0x34): running
Writing MM2S transfer length...
Memory-mapped to stream status (0x00000000@0x04): running
Waiting for MM2S synchronization...
Waiting for S2MM sychronization...
Stream to memory-mapped status (0x00001002@0x34): running idle IOC_Irq
Memory-mapped to stream status (0x00001002@0x04): running idle IOC_Irq
Destination memory block: 44332211 00000000 00000000 00000000 00000000 00000000 00000000 00000000
```

## Uninstall

```
shell# rake uninstall
dtbocfg.rb --remove loopback-dma
[ 1079.388898] udmabuf amba:udmabuf-rx: driver removed.
[ 1079.395070] udmabuf amba:udmabuf-tx: driver removed.
[ 1079.400960] fclkcfg amba:fclk0: driver unloaded
```


## Build Bitstream file

### Requirement

* Vivado 2019.1

### Create Project

```
Vivado > Tools > Run Tcl Script > project/create_project.tcl
```

### Implementation

```
Vivado > Tools > Run Tcl Script > project/implementation.tcl
```

### Convert from Bitstream File to Binary File

```
shell$ tools/fpga-bit-to-bin.py --flip project/project.runs/impl_1/design_1_wrapper.bit loopback-dma.bin
```
