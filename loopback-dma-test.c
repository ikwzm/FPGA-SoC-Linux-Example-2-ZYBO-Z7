/**
 * Proof of concept offloaded memcopy using AXI Direct Memory Access v7.1
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/mman.h>

#define MM2S_CONTROL_REGISTER 0x00
#define MM2S_STATUS_REGISTER 0x04
#define MM2S_START_ADDRESS 0x18
#define MM2S_LENGTH 0x28

#define S2MM_CONTROL_REGISTER 0x30
#define S2MM_STATUS_REGISTER 0x34
#define S2MM_DESTINATION_ADDRESS 0x48
#define S2MM_LENGTH 0x58

unsigned int dma_set(unsigned int* dma_virtual_address, int offset, unsigned int value) {
    dma_virtual_address[offset>>2] = value;
}

unsigned int dma_get(unsigned int* dma_virtual_address, int offset) {
    return dma_virtual_address[offset>>2];
}

int dma_mm2s_sync(unsigned int* dma_virtual_address) {
    unsigned int mm2s_status =  dma_get(dma_virtual_address, MM2S_STATUS_REGISTER);
    while(!(mm2s_status & 1<<12) || !(mm2s_status & 1<<1) ){
        dma_s2mm_status(dma_virtual_address);
        dma_mm2s_status(dma_virtual_address);

        mm2s_status =  dma_get(dma_virtual_address, MM2S_STATUS_REGISTER);
    }
}

int dma_s2mm_sync(unsigned int* dma_virtual_address) {
    unsigned int s2mm_status = dma_get(dma_virtual_address, S2MM_STATUS_REGISTER);
    while(!(s2mm_status & 1<<12) || !(s2mm_status & 1<<1)){
        dma_s2mm_status(dma_virtual_address);
        dma_mm2s_status(dma_virtual_address);

        s2mm_status = dma_get(dma_virtual_address, S2MM_STATUS_REGISTER);
    }
}

void dma_s2mm_status(unsigned int* dma_virtual_address) {
    unsigned int status = dma_get(dma_virtual_address, S2MM_STATUS_REGISTER);
    printf("Stream to memory-mapped status (0x%08x@0x%02x):", status, S2MM_STATUS_REGISTER);
    if (status & 0x00000001) printf(" halted"); else printf(" running");
    if (status & 0x00000002) printf(" idle");
    if (status & 0x00000008) printf(" SGIncld");
    if (status & 0x00000010) printf(" DMAIntErr");
    if (status & 0x00000020) printf(" DMASlvErr");
    if (status & 0x00000040) printf(" DMADecErr");
    if (status & 0x00000100) printf(" SGIntErr");
    if (status & 0x00000200) printf(" SGSlvErr");
    if (status & 0x00000400) printf(" SGDecErr");
    if (status & 0x00001000) printf(" IOC_Irq");
    if (status & 0x00002000) printf(" Dly_Irq");
    if (status & 0x00004000) printf(" Err_Irq");
    printf("\n");
}

void dma_mm2s_status(unsigned int* dma_virtual_address) {
    unsigned int status = dma_get(dma_virtual_address, MM2S_STATUS_REGISTER);
    printf("Memory-mapped to stream status (0x%08x@0x%02x):", status, MM2S_STATUS_REGISTER);
    if (status & 0x00000001) printf(" halted"); else printf(" running");
    if (status & 0x00000002) printf(" idle");
    if (status & 0x00000008) printf(" SGIncld");
    if (status & 0x00000010) printf(" DMAIntErr");
    if (status & 0x00000020) printf(" DMASlvErr");
    if (status & 0x00000040) printf(" DMADecErr");
    if (status & 0x00000100) printf(" SGIntErr");
    if (status & 0x00000200) printf(" SGSlvErr");
    if (status & 0x00000400) printf(" SGDecErr");
    if (status & 0x00001000) printf(" IOC_Irq");
    if (status & 0x00002000) printf(" Dly_Irq");
    if (status & 0x00004000) printf(" Err_Irq");
    printf("\n");
}

void memdump(void* virtual_address, int byte_count) {
    char *p = virtual_address;
    int offset;
    for (offset = 0; offset < byte_count; offset++) {
        printf("%02x", p[offset]);
        if (offset % 4 == 3) { printf(" "); }
    }
    printf("\n");
}

struct udmabuf {
    char           name[128];
    int            file;
    unsigned char* buf;
    unsigned int   buf_size;
    unsigned long  phys_addr;
    unsigned long  debug_vma;
    unsigned long  sync_mode;
    int            cache_on;
};

int udmabuf_open(struct udmabuf* udmabuf, const char* name, int cache_on)
{
    char           file_name[1024];
    int            fd;
    unsigned char  attr[1024];

    strcpy(udmabuf->name, name);
    udmabuf->file = -1;
    udmabuf->cache_on = cache_on;

    sprintf(file_name, "/sys/class/udmabuf/%s/phys_addr", name);
    if ((fd  = open(file_name, O_RDONLY)) == -1) {
        printf("Can not open %s\n", file_name);
        return (-1);
    }
    read(fd, (void*)attr, 1024);
    sscanf(attr, "%lx", &udmabuf->phys_addr);
    close(fd);

    sprintf(file_name, "/sys/class/udmabuf/%s/size", name);
    if ((fd  = open(file_name, O_RDONLY)) == -1) {
        printf("Can not open %s\n", file_name);
        return (-1);
    }
    read(fd, (void*)attr, 1024);
    sscanf(attr, "%d", &udmabuf->buf_size);
    close(fd);

    sprintf(file_name, "/dev/%s", name);
    if ((udmabuf->file = open(file_name, O_RDWR | ((cache_on == 0)? O_SYNC : 0))) == -1) {
        printf("Can not open %s\n", file_name);
        return (-1);
    }

    udmabuf->buf = mmap(NULL, udmabuf->buf_size, PROT_READ|PROT_WRITE, MAP_SHARED, udmabuf->file, 0);
    udmabuf->debug_vma = 0;
    udmabuf->sync_mode = 1;

    return 0;
}

int udmabuf_close(struct udmabuf* udmabuf)
{
    if (udmabuf->file < 0) 
        return -1;

    close(udmabuf->file);
    udmabuf->file = -1;
    return 0;
}

int udmabuf_sync_area(struct udmabuf* udmabuf, unsigned int offset, unsigned int size, int direction)
{
    char           file_name[1024];
    int            fd;
    unsigned char  attr[1024];

    sprintf(file_name, "/sys/class/udmabuf/%s/sync_offset", udmabuf->name);
    if ((fd  = open(file_name, O_RDWR)) == -1) {
        printf("Can not open %s\n", file_name);
        return (-1);
    }
    sprintf(attr, "%d", offset);
    write(fd, (void*)attr, strlen(attr));
    close(fd);
    sprintf(file_name, "/sys/class/udmabuf/%s/sync_size", udmabuf->name);
    if ((fd  = open(file_name, O_RDWR)) == -1) {
        printf("Can not open %s\n", file_name);
        return (-1);
    }
    sprintf(attr, "%d", size);
    write(fd, (void*)attr, strlen(attr));
    close(fd);
    sprintf(file_name, "/sys/class/udmabuf/%s/sync_direction", udmabuf->name);
    if ((fd  = open(file_name, O_RDWR)) == -1) {
        printf("Can not open %s\n", file_name);
        return (-1);
    }
    sprintf(attr, "%d", direction);
    write(fd, (void*)attr, strlen(attr));
    close(fd);
    return 0;
}

int udmabuf_sync_for_device(struct udmabuf* udmabuf)
{
    char           file_name[1024];
    int            fd;
    unsigned char  attr[1024];

    sprintf(file_name, "/sys/class/udmabuf/%s/sync_for_device", udmabuf->name);
    if ((fd  = open(file_name, O_RDWR)) == -1) {
        printf("Can not open %s\n", file_name);
        return (-1);
    }
    sprintf(attr, "%d", 1);
    write(fd, (void*)attr, strlen(attr));
    close(fd);
    return 0;
}

int udmabuf_sync_for_cpu(struct udmabuf* udmabuf)
{
    char           file_name[1024];
    int            fd;
    unsigned char  attr[1024];

    sprintf(file_name, "/sys/class/udmabuf/%s/sync_for_cpu", udmabuf->name);
    if ((fd  = open(file_name, O_RDWR)) == -1) {
        printf("Can not open %s\n", file_name);
        return (-1);
    }
    sprintf(attr, "%d", 1);
    write(fd, (void*)attr, strlen(attr));
    close(fd);
    return 0;
}

int main() {
    int            uio_fd;
    unsigned int*  virtual_address;
    struct udmabuf src_buf;
    struct udmabuf dst_buf;
    unsigned int*  virtual_source_address;
    unsigned int*  virtual_destination_address;
    unsigned int   test_size = 32;
    int            buf_cache_on = 1;

    if ((uio_fd = open("/dev/uio0", O_RDWR)) == -1) {
        printf("Can not open /dev/uio0\n");
        exit(1);
    }
    virtual_address = (unsigned int*)mmap(NULL, 65535, PROT_READ|PROT_WRITE, MAP_SHARED, uio_fd, 0);

    if (udmabuf_open(&src_buf, "udmabuf-tx", buf_cache_on) == -1) {
        printf("Can not open /dev/udmabuf-tx\n");
        exit(1);
    }
    virtual_source_address = (unsigned int*)src_buf.buf;
    udmabuf_sync_area(&src_buf, 0, test_size, 1);
    
    if (udmabuf_open(&dst_buf, "udmabuf-rx", buf_cache_on) == -1) {
        printf("Can not open /dev/udmabuf-rx\n");
        exit(1);
    }
    virtual_destination_address = (unsigned int*)dst_buf.buf;
    udmabuf_sync_area(&dst_buf, 0, test_size, 2);

    virtual_source_address[0]= 0x11223344; // Write random stuff to source block
    memset(virtual_destination_address, 0, test_size); // Clear destination block

    printf("Source memory block:      "); memdump(virtual_source_address, test_size);
    printf("Destination memory block: "); memdump(virtual_destination_address, test_size);

    if (buf_cache_on == 1) {
        udmabuf_sync_for_device(&src_buf);
        udmabuf_sync_for_device(&dst_buf);
    }

    printf("Resetting DMA\n");
    dma_set(virtual_address, S2MM_CONTROL_REGISTER, 4);
    dma_set(virtual_address, MM2S_CONTROL_REGISTER, 4);
    dma_s2mm_status(virtual_address);
    dma_mm2s_status(virtual_address);

    printf("Halting DMA\n");
    dma_set(virtual_address, S2MM_CONTROL_REGISTER, 0);
    dma_set(virtual_address, MM2S_CONTROL_REGISTER, 0);
    dma_s2mm_status(virtual_address);
    dma_mm2s_status(virtual_address);

    printf("Writing destination address\n");
    dma_set(virtual_address, S2MM_DESTINATION_ADDRESS, (unsigned int)dst_buf.phys_addr); // Write destination address
    dma_s2mm_status(virtual_address);

    printf("Writing source address...\n");
    dma_set(virtual_address, MM2S_START_ADDRESS, (unsigned int)src_buf.phys_addr); // Write source address
    dma_mm2s_status(virtual_address);

    printf("Starting S2MM channel with all interrupts masked...\n");
    dma_set(virtual_address, S2MM_CONTROL_REGISTER, 0xf001);
    dma_s2mm_status(virtual_address);

    printf("Starting MM2S channel with all interrupts masked...\n");
    dma_set(virtual_address, MM2S_CONTROL_REGISTER, 0xf001);
    dma_mm2s_status(virtual_address);

    printf("Writing S2MM transfer length...\n");
    dma_set(virtual_address, S2MM_LENGTH, test_size);
    dma_s2mm_status(virtual_address);

    printf("Writing MM2S transfer length...\n");
    dma_set(virtual_address, MM2S_LENGTH, test_size);
    dma_mm2s_status(virtual_address);

    printf("Waiting for MM2S synchronization...\n");
    dma_mm2s_sync(virtual_address);

    printf("Waiting for S2MM sychronization...\n");
    dma_s2mm_sync(virtual_address); // If this locks up make sure all memory ranges are assigned under Address Editor!

    dma_s2mm_status(virtual_address);
    dma_mm2s_status(virtual_address);

    if (buf_cache_on == 1) {
        udmabuf_sync_for_cpu(&src_buf);
        udmabuf_sync_for_cpu(&dst_buf);
    }
    
    printf("Destination memory block: "); memdump(virtual_destination_address, test_size);

    udmabuf_close(&src_buf);
    udmabuf_close(&dst_buf);
    close(uio_fd);
}
