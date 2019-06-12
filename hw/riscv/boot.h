
target_ulong load_firmware_and_kernel(const char *firmware_filename,
                                      const char *kernel_filename,
                                      uint64_t mem_base,
                                      uint64_t mem_size,
                                      uint64_t* kernel_start,
                                      uint64_t* kernel_end);

uint64_t load_initrd(const char *filename, uint64_t mem_base,
                     uint64_t mem_size, uint64_t *start);
