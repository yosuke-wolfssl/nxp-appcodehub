/* target.h - HAL target definitions for libwolfboot */

#define RAMFUNCTION __attribute__((used,section(".ramfunc")))

#define WOLFBOOT_FIXED_PARTITIONS
#define WOLFBOOT_SECTOR_SIZE                 0x2000
#define WOLFBOOT_PARTITION_BOOT_ADDRESS   0x10000
#define WOLFBOOT_PARTITION_UPDATE_ADDRESS 0xD0000
#define WOLFBOOT_PARTITION_SWAP_ADDRESS   0x190000
#define WOLFBOOT_PARTITION_SIZE           0xC0000