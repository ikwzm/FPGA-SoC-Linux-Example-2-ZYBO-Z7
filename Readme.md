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
<stdin>:22.17-47.6: Warning (unit_address_vs_reg): /fragment@1/__overlay__/loopback_dma: node has a reg or ranges property, but no unit name
<stdin>:30.57-37.14: Warning (unit_address_vs_reg): /fragment@1/__overlay__/loopback_dma/dma-channel@40400000: node has a unit name, but no reg property
<stdin>:39.57-46.7: Warning (unit_address_vs_reg): /fragment@1/__overlay__/loopback_dma/dma-channel@40400030: node has a unit name, but no reg property
<stdin>:9.13-62.4: Warning (avoid_unnecessary_addr_size): /fragment@1: unnecessary #address-cells/#size-cells without "ranges" or child "reg" property
[ 4810.664289] fpga_manager fpga0: writing loopback-dma.bin to Xilinx Zynq FPGA Manager
[ 4810.739042] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /amba/fpga-region0/firmware-name
[ 4810.752987] fclkcfg amba:fclk0: driver installed.
[ 4810.760961] fclkcfg amba:fclk0: device name    : amba:fclk0
[ 4810.766621] fclkcfg amba:fclk0: clock  name    : fclk0
[ 4810.771770] fclkcfg amba:fclk0: clock  rate    : 99999999
[ 4810.777274] fclkcfg amba:fclk0: clock  enabled : 1
[ 4810.785038] uio_pdrv_genirq 40400000.loopback_dma: IRQ index 0 not found
[ 4810.808665] u-dma-buf udmabuf-tx: driver version = 3.0.1
[ 4810.814069] u-dma-buf udmabuf-tx: major number   = 244
[ 4810.819204] u-dma-buf udmabuf-tx: minor number   = 0
[ 4810.824735] u-dma-buf udmabuf-tx: phys address   = 0x30100000
[ 4810.830480] u-dma-buf udmabuf-tx: buffer size    = 1048576
[ 4810.836454] u-dma-buf udmabuf-tx: dma device     = amba:udmabuf-tx
[ 4810.842632] u-dma-buf udmabuf-tx: dma coherent   = 1
[ 4810.847682] u-dma-buf amba:udmabuf-tx: driver installed.
[ 4810.861219] u-dma-buf udmabuf-rx: driver version = 3.0.1
[ 4810.866576] u-dma-buf udmabuf-rx: major number   = 244
[ 4810.871709] u-dma-buf udmabuf-rx: minor number   = 1
[ 4810.876722] u-dma-buf udmabuf-rx: phys address   = 0x30200000
[ 4810.882463] u-dma-buf udmabuf-rx: buffer size    = 1048576
[ 4810.887998] u-dma-buf udmabuf-rx: dma device     = amba:udmabuf-rx
[ 4810.894216] u-dma-buf udmabuf-rx: dma coherent   = 1
[ 4810.899173] u-dma-buf amba:udmabuf-rx: driver installed.
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
