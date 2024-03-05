#ifndef _QCC74x_BLOCK_POOL_H
#define _QCC74x_BLOCK_POOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define QCC74x_BLOCK_POOL_ALIGN_1   0x00
#define QCC74x_BLOCK_POOL_ALIGN_2   0x01
#define QCC74x_BLOCK_POOL_ALIGN_4   0x02
#define QCC74x_BLOCK_POOL_ALIGN_8   0x03
#define QCC74x_BLOCK_POOL_ALIGN_16  0x04
#define QCC74x_BLOCK_POOL_ALIGN_32  0x05
#define QCC74x_BLOCK_POOL_ALIGN_64  0x06
#define QCC74x_BLOCK_POOL_ALIGN_128 0x07

typedef struct {
    void *free_list;    /*!< free block list          */
    void *pool_addr;    /*!< pool start pointer       */
    void *blk_addr;     /*!< block area start pointer */
    uint32_t blk_size;  /*!< block size               */
    uint32_t blk_align; /*!< block align              */
    uint32_t blk_total; /*!< total block num          */
    uint32_t blk_free;  /*!< free block num           */

    int (*mtx_get)(uint32_t wait);
    void (*mtx_put)(void);
    int (*sem_get)(uint32_t wait);
    void (*sem_put)(void);
} qcc74x_block_pool_t;

extern int qcc74x_block_pool_create(qcc74x_block_pool_t *blk_pool, uint32_t blk_size, uint32_t blk_align, void *pool_addr, uint32_t pool_size);
extern int qcc74x_block_pool_delete(qcc74x_block_pool_t *blk_pool);

extern int qcc74x_block_pool_add_mtx(qcc74x_block_pool_t *blk_pool, int (*get)(uint32_t wait), void (*put)(void));
extern int qcc74x_block_pool_add_sem(qcc74x_block_pool_t *blk_pool, int (*get)(uint32_t wait), void (*put)(void));

extern int qcc74x_block_pool_alloc(qcc74x_block_pool_t *blk_pool, void **addr, uint32_t wait);
extern int qcc74x_block_pool_free(qcc74x_block_pool_t *blk_pool, void *addr);

extern void qcc74x_block_pool_info_get(qcc74x_block_pool_t *blk_pool, uint32_t *total_blk, uint32_t *free_blk);

#ifdef __cplusplus
}
#endif

#endif
