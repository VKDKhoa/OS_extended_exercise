#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(int argc, char *argv[]){
    uint32_t n = atoi(argv[1]);
    uint32_t page_size = 4096; // 4KB
    uint32_t page_number = n / page_size;
    uint32_t offset = n % page_size;

    printf("Address: %u\n", n);
    printf("Page Number: %u\n", page_number);
    printf("Offset: %u\n", offset);
    return 0;
}