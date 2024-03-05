#include <stdint.h>
#include <stdio.h>
#include "qcc74x_block_pool.h"

#ifdef CONFIG_QCC74x_BLOCK_POOL_DEBUG
#define _QCC74x_BLOCK_POOL_CHECK(_expr, _ret) \
    if (!(_expr))                           \
    return _ret
#else
#define _QCC74x_BLOCK_POOL_CHECK(_expr, _ret) ((void)0)
#endif

#ifndef CONFIG_QCC74x_BLOCK_POOL_MTX_TIMEOUT
#define CONFIG_QCC74x_BLOCK_POOL_MTX_TIMEOUT 100
#endif

/**
 *   @brief         create a block blk_pool
 *   @param  blk_pool               block pool instance
 *   @param  blk_size               block size
 *   @param  blk_align              block align
 *   @param  pool_addr              pool address
 *   @param  pool_size              pool size
 *   @return int 
 */
int qcc74x_block_pool_create(qcc74x_block_pool_t *blk_pool, uint32_t blk_size, uint32_t blk_align, void *pool_addr, uint32_t pool_size)
{
    _QCC74x_BLOCK_POOL_CHECK(blk_pool != NULL, -1);
    _QCC74x_BLOCK_POOL_CHECK(blk_size > 0, -1);
    _QCC74x_BLOCK_POOL_CHECK(((blk_align >= QCC74x_BLOCK_POOL_ALIGN_1) &&
                            (blk_align <= QCC74x_BLOCK_POOL_ALIGN_128)),
                           -1);
    _QCC74x_BLOCK_POOL_CHECK(pool_addr != NULL, -1);
    _QCC74x_BLOCK_POOL_CHECK(pool_size > 0, -1);
    _QCC74x_BLOCK_POOL_CHECK(((uintptr_t)pool_addr & (sizeof(void *) - 1)) == 0, -1);

    uint32_t bitmask = ((0x1 << blk_align) - 1);
    uint32_t blk_count;
    uintptr_t address;

    if (blk_size & bitmask) {
        blk_size = (blk_size & (~bitmask)) + (0x1 << blk_align);
    }

    blk_count = pool_size / (sizeof(void *) + blk_size);

    if (blk_count == 0) {
        return -1;
    }

    /*!< calculate block area start address */
    address = (uintptr_t)pool_addr + sizeof(void *) * blk_count;

    /*!< align address */
    if (address & bitmask) {
        address = (address & (~bitmask)) + (0x1 << blk_align);
    }

    /*!< if not enough memory after align */
    if ((address + (blk_count * blk_size)) > ((uintptr_t)pool_addr + pool_size)) {
        /*!< do block count -1 */
        blk_count -= 1;
        if (blk_count == 0) {
            return -1;
        }

        /*!< recalculate block area start address */
        address = (uintptr_t)pool_addr + sizeof(void *) * blk_count;

        /*!< align address */
        if (address & bitmask) {
            address = (address & (~bitmask)) + (0x1 << blk_align);
        }
    }

    /*!< build list */
    for (uint32_t i = 0; i < blk_count; i++) {
        ((void **)pool_addr)[i] = &((void **)pool_addr)[i + 1];
    }

    /*!< last node link to NULL */
    ((void **)pool_addr)[blk_count - 1] = NULL;

    /*!< set free block node list */
    blk_pool->free_list = pool_addr;

    /*!< set pool start address */
    blk_pool->pool_addr = pool_addr;

    /*!< set block area start address */
    blk_pool->blk_addr = (void *)address;

    /*!< set block size (aligned) */
    blk_pool->blk_size = blk_size;

    blk_pool->blk_align = blk_align;

    blk_pool->blk_total = blk_count;

    blk_pool->blk_free = blk_count;

    blk_pool->mtx_get = NULL;
    blk_pool->mtx_put = NULL;
    blk_pool->sem_get = NULL;
    blk_pool->sem_put = NULL;

    return 0;
}

/**
 *   @brief         delete a block blk_pool
 *   @param  blk_pool               block pool instance
 *   @return int 
 */
int qcc74x_block_pool_delete(qcc74x_block_pool_t *blk_pool)
{
    _QCC74x_BLOCK_POOL_CHECK(blk_pool != NULL, -1);

    if (blk_pool->blk_total != blk_pool->blk_free) {
        return -1;
    }

    blk_pool->free_list = NULL;
    blk_pool->pool_addr = NULL;
    blk_pool->blk_addr = NULL;
    blk_pool->blk_size = 0;
    blk_pool->blk_align = 0;
    blk_pool->blk_total = 0;
    blk_pool->blk_free = 0;
    blk_pool->mtx_get = NULL;
    blk_pool->mtx_put = NULL;
    blk_pool->sem_get = NULL;
    blk_pool->sem_put = NULL;

    return 0;
}

/**
 *   @brief         register or unregister mutex
 *   @param  blk_pool               block pool instance
 *   @param  get                    mutex get
 *   @param  put                    mutex put
 *   @return int 
 */
int qcc74x_block_pool_add_mtx(qcc74x_block_pool_t *blk_pool, int (*get)(uint32_t wait), void (*put)(void))
{
    _QCC74x_BLOCK_POOL_CHECK(blk_pool != NULL, -1);

    if ((get == NULL) || (put == NULL)) {
        blk_pool->mtx_get = NULL;
        blk_pool->mtx_put = NULL;
    } else {
        blk_pool->mtx_get = get;
        blk_pool->mtx_put = put;
    }

    return 0;
}

/**
 *   @brief         register or unregister semaphore
 *   @param  blk_pool               block pool instance
 *   @param  get                    semaphore get
 *   @param  put                    semaphore put
 *   @return int 
 */
int qcc74x_block_pool_add_sem(qcc74x_block_pool_t *blk_pool, int (*get)(uint32_t wait), void (*put)(void))
{
    _QCC74x_BLOCK_POOL_CHECK(blk_pool != NULL, -1);

    if ((get == NULL) || (put == NULL)) {
        blk_pool->sem_get = NULL;
        blk_pool->sem_put = NULL;
    } else {
        blk_pool->sem_get = get;
        blk_pool->sem_put = put;
    }

    return 0;
}

/**
 *   @brief         alloc a block
 *   @param  blk_pool               block pool instance
 *   @param  addr                   block pointer
 *   @param  wait                   timeout
 *   @return int 
 */
int qcc74x_block_pool_alloc(qcc74x_block_pool_t *blk_pool, void **addr, uint32_t wait)
{
    _QCC74x_BLOCK_POOL_CHECK(blk_pool != NULL, -1);
    _QCC74x_BLOCK_POOL_CHECK(addr != NULL, -1);

    void *node;
    uint32_t index;

    if (blk_pool->sem_get) {
        /*!< check for free block by sem */
        if (blk_pool->sem_get(wait)) {
            return -1;
        }
    }

    /*!< check for free block */
    if (blk_pool->blk_free == 0) {
        return -1;
    }

    /*!< get mutex */
    if (blk_pool->mtx_get) {
        if (blk_pool->mtx_get(CONFIG_QCC74x_BLOCK_POOL_MTX_TIMEOUT)) {
            blk_pool->sem_put();
            return -1;
        }
    }

    /*!< get first free list node */
    node = blk_pool->free_list;

    /*!< delete node from free_list */
    blk_pool->free_list = *((void **)node);

    /*!< calculate the index */
    index = ((uintptr_t)node - (uintptr_t)(blk_pool->pool_addr)) / sizeof(void *);

    /*!< calculate block address */
    *addr = (void *)((uintptr_t)(blk_pool->blk_addr) + (index * blk_pool->blk_size));

    /*!< decrease free block count */
    blk_pool->blk_free -= 1;

    /*!< put mutex */
    if (blk_pool->mtx_put) {
        blk_pool->mtx_put();
    }

    return 0;
}

/**
 *   @brief         free a block
 *   @param  blk_pool               blk_pool
 *   @param  addr                   block pointer
 *   @return int 
 */
int qcc74x_block_pool_free(qcc74x_block_pool_t *blk_pool, void *addr)
{
    _QCC74x_BLOCK_POOL_CHECK(blk_pool != NULL, -1);
    _QCC74x_BLOCK_POOL_CHECK(addr != NULL, -1);

    void *node;
    uint32_t index;

    if (blk_pool->mtx_get) {
        if (blk_pool->mtx_get(CONFIG_QCC74x_BLOCK_POOL_MTX_TIMEOUT)) {
            return -1;
        }
    }

    /*!< simple check if addr is our block */
    if (blk_pool->blk_total == blk_pool->blk_free) {
        return -1;
    }

    /*!< calculate the index */
    /*!< maybe the original value calculated will be negative, but it has no effect. */
    index = ((uintptr_t)addr - (uintptr_t)(blk_pool->blk_addr)) / blk_pool->blk_size;

    /*!< this is not our block */
    if (index > blk_pool->blk_total) {
        return -1;
    }

    /*!< calculate node address */
    node = (void *)((uintptr_t)(blk_pool->pool_addr) + (index * sizeof(void *)));

    /*!< link first free list node */
    *((void **)node) = blk_pool->free_list;

    /*!< set first free list node */
    blk_pool->free_list = node;

    /*!< increase free block count */
    blk_pool->blk_free += 1;

    if (blk_pool->mtx_put) {
        blk_pool->mtx_put();
    }

    if (blk_pool->sem_put) {
        blk_pool->sem_put();
    }

    return 0;
}

/**
 *   @brief         get block blk_pool info
 *   @param  blk_pool               block pool instance
 *   @param  total_blk              total block count pointer
 *   @param  free_blk               free block count pointer
 *   @return int 
 */
void qcc74x_block_pool_info_get(qcc74x_block_pool_t *blk_pool, uint32_t *total_blk, uint32_t *free_blk)
{
    _QCC74x_BLOCK_POOL_CHECK(blk_pool != NULL, );

    if (total_blk != NULL) {
        *total_blk = blk_pool->blk_total;
    }

    if (free_blk != NULL) {
        *free_blk = blk_pool->blk_free;
    }
}
