/* hal implementation for libwolfboot on Zephyr */

#include <stdint.h>
#include <string.h>
#include "target.h"

#include <zephyr/device.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/sys/printk.h>

#include "fsl_common.h"
#include "fsl_flash.h"

static flash_config_t pflash;
static uint32_t pflash_sector_size = WOLFBOOT_SECTOR_SIZE;
static uint32_t pflash_block_base = CONFIG_FLASH_BASE_ADDRESS;
static uint32_t pflash_total_size = DT_REG_SIZE(DT_CHOSEN(zephyr_flash));
static uint32_t pflash_write_block_size;
static int pflash_ready;
static const struct device *flash_dev;

#define HAL_FLASH_NODE DT_CHOSEN(zephyr_flash_controller)

static int hal_flash_offset(uint32_t address, off_t *offset)
{
    if (address >= pflash_block_base &&
        address < (pflash_block_base + pflash_total_size)) {
        *offset = (off_t)(address - pflash_block_base);
        return 0;
    }

    if (address < pflash_total_size) {
        *offset = (off_t)address;
        return 0;
    }

    printk("hal_flash: address out of range addr=0x%08x base=0x%08x size=0x%08x\n",
           (unsigned int)address, (unsigned int)pflash_block_base,
           (unsigned int)pflash_total_size);

    return -1;
}

static int hal_flash_init_once(void)
{
    const struct flash_parameters *flash_params;
    status_t status;

    if (pflash_ready) {
        return 0;
    }

    flash_dev = DEVICE_DT_GET(HAL_FLASH_NODE);
    if (!device_is_ready(flash_dev)) {
        printk("hal_init: flash device not ready\n");
        return -1;
    }

    memset(&pflash, 0, sizeof(pflash));

    status = FLASH_Init(&pflash);
    if (status != kStatus_FLASH_Success) {
        printk("hal_init: FLASH_Init failed status=%d\n", (int)status);
        return -1;
    }

    status = FLASH_GetProperty(&pflash, kFLASH_PropertyPflashSectorSize,
                               &pflash_sector_size);
    if (status != kStatus_FLASH_Success) {
        printk("hal_init: FLASH_GetProperty failed status=%d\n",
               (int)status);
        pflash_sector_size = WOLFBOOT_SECTOR_SIZE;
        return -1;
    }

    flash_params = flash_get_parameters(flash_dev);
    pflash_write_block_size = flash_params->write_block_size;
    if (pflash_write_block_size == 0U) {
        printk("hal_init: invalid write block size\n");
        return -1;
    }

    pflash_ready = 1;

        printk("hal_init: base=0x%08x size=0x%08x sector_size=%u write_block=%u\n",
           (unsigned int)pflash_block_base, (unsigned int)pflash_sector_size,
            (unsigned int)pflash_total_size,
           (unsigned int)pflash_write_block_size);

    return 0;
}

void hal_init(void)
{
    (void)hal_flash_init_once();
} 

void hal_prepare_boot(void)
{
    /* nothing to do */
}

int RAMFUNCTION hal_flash_write(uint32_t address, const uint8_t *data, int len)
{
    uint8_t write_buf[256];
    int written = 0;
    off_t offset;

    BUILD_ASSERT(sizeof(write_buf) >= 128,
                 "write buffer must fit the flash write block size");

    if (!pflash_ready && hal_flash_init_once() != 0) {
        return -1;
    }

    if ((uint32_t)pflash_write_block_size > sizeof(write_buf)) {
        printk("hal_flash_write: unsupported write block %u\n",
               (unsigned int)pflash_write_block_size);
        return -1;
    }

    if (hal_flash_offset(address, &offset) != 0) {
        return -1;
    }

    printk("hal_flash_write: addr=0x%08x len=%d\n", (unsigned int)address,
           len);

    while (len > 0) {
        off_t block_offset = offset & ~((off_t)pflash_write_block_size - 1);
        uint32_t within_block = (uint32_t)(offset - block_offset);
        uint32_t copy = pflash_write_block_size - within_block;
        int rc;

        if (copy > (uint32_t)len) {
            copy = (uint32_t)len;
        }

        rc = flash_read(flash_dev, block_offset, write_buf,
                        pflash_write_block_size);
        if (rc != 0) {
            printk("hal_flash_write: read failed at off=0x%08x rc=%d\n",
                   (unsigned int)block_offset, rc);
            return -1;
        }

        memcpy(write_buf + within_block, data + written, copy);

        rc = flash_write(flash_dev, block_offset, write_buf,
                         pflash_write_block_size);
        if (rc != 0) {
            printk("hal_flash_write: program failed at off=0x%08x size=%u rc=%d\n",
                   (unsigned int)block_offset,
                   (unsigned int)pflash_write_block_size, rc);
            return -1;
        }

        address += copy;
        offset += (off_t)copy;
        len -= (int)copy;
        written += (int)copy;
    }

    printk("hal_flash_write: done\n");

    return 0;
}

void RAMFUNCTION hal_flash_unlock(void)
{
    /* nothing to do */
}

void RAMFUNCTION hal_flash_lock(void)
{
    /* nothing to do */
}

int RAMFUNCTION hal_flash_erase(uint32_t address, int len)
{
    uint32_t sector_size = pflash_sector_size;
    off_t offset;
    int rc;

    if (!pflash_ready && hal_flash_init_once() != 0) {
        return -1;
    }

    if (hal_flash_offset(address, &offset) != 0) {
        return -1;
    }

    printk("hal_flash_erase: addr=0x%08x len=%d\n", (unsigned int)address,
           len);

    if (sector_size == 0U) {
        sector_size = WOLFBOOT_SECTOR_SIZE;
    }

    if ((address % sector_size) != 0U) {
        address -= address % sector_size;
        offset -= (off_t)(offset % sector_size);
    }

    while (len > 0) {
        rc = flash_erase(flash_dev, offset, sector_size);
        if (rc != 0) {
            printk("hal_flash_erase: erase failed at 0x%08x off=0x%08x size=%u rc=%d\n",
                   (unsigned int)address, (unsigned int)offset,
                   (unsigned int)sector_size, rc);
            return -1;
        }
        address += sector_size;
        offset += (off_t)sector_size;
        len -= (int)sector_size;
    }

    printk("hal_flash_erase: done\n");

    return 0;
}