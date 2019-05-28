
#include "qemu/osdep.h"
#include "qemu/units.h"
#include "qemu/error-report.h"
#include "hw/loader.h"
#include "target/riscv/cpu.h"
#include "hw/riscv/boot.h"
#include "elf.h"

target_ulong load_firmware_and_kernel(const char *firmware_filename,
                                      const char *kernel_filename,
                                      uint64_t mem_size,
                                      uint64_t* kernel_start,
                                      uint64_t* kernel_end)
{
    uint64_t firmware_entry, firmware_end;
    int size;

    if (load_elf(firmware_filename, NULL, NULL, NULL,
                 &firmware_entry, NULL, &firmware_end,
                 0, EM_RISCV, 1, 0) < 0) {
        error_report("could not load firmware '%s'", firmware_filename);
        exit(1);
    }

    /* align kernel load address to the megapage after the firmware */
#if defined(TARGET_RISCV32)
    *kernel_start = (firmware_end + 0x3fffff) & ~0x3fffff;
#else
    *kernel_start = (firmware_end + 0x1fffff) & ~0x1fffff;
#endif

    size = load_image_targphys(kernel_filename, *kernel_start,
                               mem_size - *kernel_start);
    if (size == -1) {
        error_report("could not load kernel '%s'", kernel_filename);
        exit(1);
    }
    *kernel_end = *kernel_start + size;
    return firmware_entry;
}

target_ulong load_kernel(const char *kernel_filename)
{
    uint64_t kernel_entry;

    if (load_elf(kernel_filename, NULL, NULL, NULL,
                 &kernel_entry, NULL, NULL,
                 0, EM_RISCV, 1, 0) < 0) {
        error_report("could not load kernel '%s'", kernel_filename);
        exit(1);
    }
    return kernel_entry;
}

uint64_t load_initrd(const char *filename, uint64_t mem_base,
                     uint64_t mem_size, uint64_t *start)
{
    int size;

    /* We want to put the initrd far enough into RAM that when the
     * kernel is uncompressed it will not clobber the initrd. However
     * on boards without much RAM we must ensure that we still leave
     * enough room for a decent sized initrd, and on boards with large
     * amounts of RAM we must avoid the initrd being so far up in RAM
     * that it is outside lowmem and inaccessible to the kernel. So
     * for boards with less than 256MB of RAM we put the initrd
     * halfway into RAM, and for boards with 256MB of RAM or more we
     * put the initrd at 128MB.
     */
    *start = mem_base + MIN(mem_size / 2, 128 * MiB);

    size = load_ramdisk(filename, *start, mem_size - *start);
    if (size == -1) {
        size = load_image_targphys(filename, *start, mem_size - *start);
        if (size == -1) {
            error_report("could not load ramdisk '%s'", filename);
            exit(1);
        }
    }
    return *start + size;
}
