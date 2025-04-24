#ifndef PAR_UTILITY_H
#define PAR_UTILITY_H
#include <atomic>
#include <fcntl.h>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include "cmdline_param_parser.hpp"

// map the file pointed by filepath in memory
// if size is zero, it looks for file size
// if everything is ok, it returns the memory pointer ptr
static inline bool mapFile(const char fname[], size_t &size, unsigned char *&ptr, const CompressionParams &cpar) {
    // open input file.
    int fd = open(fname,O_RDONLY);
    if (fd < 0) {
        if (cpar.quite_mode >= 1) {
            perror("mapFile open");
            std::fprintf(stderr, "Failed opening file %s\n", fname);
        }
        return false;
    }
    if (size == 0) {
        struct stat s;
        if (fstat(fd, &s)) {
            if (cpar.quite_mode >= 1) {
                perror("fstat");
                std::fprintf(stderr, "Failed to stat file %s\n", fname);
            }
            return false;
        }
        size = s.st_size;
    }

    // map all the file in memory
    ptr = (unsigned char *) mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (ptr == MAP_FAILED) {
        if (cpar.quite_mode >= 1) {
            perror("mmap");
            std::fprintf(stderr, "Failed to memory map file %s\n", fname);
        }
        return false;
    }
    close(fd);
    return true;
}

// unmap a previously memory-mapped file
static inline void unmapFile(unsigned char *ptr, size_t size, const CompressionParams &cpar) {
    if (munmap(ptr, size) < 0) {
        if (cpar.quite_mode >= 1) {
            perror("nummap");
            std::fprintf(stderr, "Failed to unmap file\n");
        }
    }
}
#endif //PAR_UTILITY_H
