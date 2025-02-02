#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include "ramsync.h"
#include "spisync_port.h"
#include "stm32u5xx_hal.h"
#include "stm32u5xx_ll_spi.h"

extern SPI_HandleTypeDef hspi1;
static lramsync_ctx_t *g_ctx;

/* ------------------- platform port ------------------- */
#define rsl_malloc(x)	malloc_aligned_with_padding(x, 32)
#define rsl_free(x)		free_aligned_with_padding(x)

/* ------------------- internal type or rodata ------------------- */
struct _ramsync_low_priv {
	unsigned int buf_idx;
	char half_xfer_cplt;
	int suspended;
};

void HAL_SPI_TxRxHalfCpltCallback(SPI_HandleTypeDef *hspi)
{
	struct _ramsync_low_priv *priv = g_ctx->priv;

	priv->half_xfer_cplt = 1;
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
	HAL_StatusTypeDef ret;
	struct _ramsync_low_priv *priv = g_ctx->priv;

	 if (g_ctx->tx_cb)
		 g_ctx->tx_cb(g_ctx->tx_arg);
	 if (g_ctx->rx_cb)
		 g_ctx->rx_cb(g_ctx->rx_arg);
	/*
	 * spi dma transfer stops here if it is suspended and another transfer
	 * is needed to resume it.
	 */
	
	priv->half_xfer_cplt = 0;
	priv->buf_idx++;
	if (priv->buf_idx >= g_ctx->items_tx)
		priv->buf_idx = 0;

    if (priv->suspended)
		return;

	uint8_t *txptr = g_ctx->node_tx[priv->buf_idx].buf;
	uint8_t *rxptr = g_ctx->node_rx[priv->buf_idx].buf;
	uint16_t len = g_ctx->node_tx[priv->buf_idx].len;
	ret = HAL_SPI_TransmitReceive_DMA(&hspi1, txptr, rxptr, len);
	if (ret != HAL_OK)
		spisync_err("failed to start spi txrx in completion callback, %d\r\n", ret);
}

static void spi_hw_init(const lramsync_spi_config_t *config)
{
}

static void spi_dma_init(lramsync_ctx_t *ctx)
{
	ctx->dma_tx_chan = NULL;
	ctx->dma_rx_chan = NULL;
}

static int spi_dma_start(lramsync_ctx_t *ctx)
{
	HAL_StatusTypeDef ret;
	ret = HAL_SPI_TransmitReceive_DMA(&hspi1, ctx->node_tx[0].buf,
										ctx->node_rx[0].buf, ctx->node_tx[0].len);
	if (ret != HAL_OK) {
		printf("failed to start spi txrx, %d\r\n", ret);
		return -1;
	}

    return 0;
}

static void lramsync_callback_register(
        lramsync_ctx_t *ctx,
        lramsync_cb_func_t tx_cb, void *tx_arg,
        lramsync_cb_func_t rx_cb, void *rx_arg)
{
    ctx->tx_cb  = tx_cb;
    ctx->tx_arg = tx_arg;
    ctx->rx_cb  = rx_cb;
    ctx->rx_arg = rx_arg;
}

int lramsync_get_next_write_slot(lramsync_ctx_t *ctx)
{
	struct _ramsync_low_priv *priv = ctx->priv;

    if (priv->suspended) {
        return priv->buf_idx;
    }

	return priv->buf_idx + 1;
    //return priv->half_xfer_cplt ? priv->buf_idx + 2: priv->buf_idx + 1;
}

int lramsync_start(lramsync_ctx_t *ctx)
{
    spi_dma_start(ctx);
    return 0;
}

int lramsync_stop(lramsync_ctx_t *ctx)
{
	HAL_StatusTypeDef status;

    if (ctx == NULL) {
    	spisync_err("ctx is NULL, can't stop it\r\n");
    	return -1;
    }

    /* abort SPI ongoing transfer */
    status = HAL_SPI_Abort(&hspi1);
    if (status != HAL_OK) {
    	spisync_err("failed to abort spi, %d\r\n", status);
    	return -1;
    }
    spisync_trace("spi dma is stopped\r\n");
    return 0;
}

int lramsync_reset(lramsync_ctx_t *ctx)
{
	HAL_StatusTypeDef status;

    if (ctx == NULL)
        return -1;

    /* abort SPI ongoing transfer */
    status = HAL_SPI_Abort(&hspi1);
    if (status != HAL_OK) {
    	spisync_err("failed to abort spi, %d\r\n", status);
    	return -1;
    }
    spisync_trace("spi dma is reset\r\n");
    spi_dma_init(ctx);
    return 0;
}

int lramsync_deinit(lramsync_ctx_t *ctx)
{
    if (ctx == NULL)
        return -1;

    spisync_err("lramsync deinit is not supported yet\r\n");
    return 0;
}

int lramsync_init(
		const spisync_config_t *config,
        lramsync_ctx_t *ctx,
        node_mem_t *node_tx, uint32_t items_tx,
        lramsync_cb_func_t tx_cb, void *tx_arg,
        node_mem_t *node_rx, uint32_t items_rx,
        lramsync_cb_func_t rx_cb, void *rx_arg,
        lramsync_cb_func_t reset_signal_cb, void *reset_signal_arg)
{
    struct _ramsync_low_priv *priv = NULL;

    if (ctx == NULL)
        goto rsl_init_err;

    memset(ctx, 0, sizeof(lramsync_ctx_t));

    ctx->cfg         = NULL;
    ctx->dma_tx_chan = NULL;
    ctx->dma_rx_chan = NULL;
    ctx->reset_signal_cb = reset_signal_cb;
    ctx->reset_signal_arg = reset_signal_arg;

    ctx->node_tx = rsl_malloc(sizeof(node_mem_t) * items_tx);
    if (ctx->node_tx == NULL)
        goto rsl_init_err;

    ctx->node_rx = rsl_malloc(sizeof(node_mem_t) * items_rx);
    if (ctx->node_rx == NULL)
        goto rsl_init_err;

    memcpy(ctx->node_tx, node_tx, sizeof(node_mem_t) * items_tx);
    memcpy(ctx->node_rx, node_rx, sizeof(node_mem_t) * items_rx);
    ctx->items_tx = items_tx;
    ctx->items_rx = items_rx;

    lramsync_callback_register(ctx, tx_cb, tx_arg, rx_cb, rx_arg);

    ctx->priv = rsl_malloc(sizeof(struct _ramsync_low_priv));
    if (ctx->priv == NULL)
        goto rsl_init_err;

    priv = (struct _ramsync_low_priv *)ctx->priv;
    priv->suspended = 0;
    priv->buf_idx = 0;
    priv->half_xfer_cplt = 0;
    g_ctx = ctx;
    spi_hw_init(ctx->cfg);
    spi_dma_init(ctx);
    spi_dma_start(ctx);
    spisync_log("lramsync_init done\r\n");
    return 0;

rsl_init_err:
    if (ctx && ctx->node_rx)
        rsl_free(ctx->node_rx);

    if (ctx && ctx->node_tx)
        rsl_free(ctx->node_tx);

    if (priv)
        rsl_free(priv);
    return -1;
}

int lramsync_suspend(lramsync_ctx_t *ctx)
{
	struct _ramsync_low_priv *priv = ctx->priv;

	HAL_NVIC_DisableIRQ(SPI1_IRQn);
    if (!priv->suspended)
    	priv->suspended = 1;
    HAL_NVIC_EnableIRQ(SPI1_IRQn);
    spisync_trace("spi dma suspend flag is set\r\n");
    return 0;
}

int lramsync_resume(lramsync_ctx_t *ctx)
{
	HAL_StatusTypeDef ret;
	struct _ramsync_low_priv *priv = ctx->priv;

	HAL_NVIC_DisableIRQ(SPI1_IRQn);
    if (!priv->suspended) {
    	HAL_NVIC_EnableIRQ(SPI1_IRQn);
    	return 0;
    }

    priv->suspended = 0;

	uint8_t *txptr = g_ctx->node_tx[priv->buf_idx].buf;
	uint8_t *rxptr = g_ctx->node_rx[priv->buf_idx].buf;
	uint16_t len = g_ctx->node_tx[priv->buf_idx].len;
	ret = HAL_SPI_TransmitReceive_DMA(&hspi1, txptr, rxptr, len);
	HAL_NVIC_EnableIRQ(SPI1_IRQn);
	if (ret != HAL_OK && ret != HAL_BUSY) {
		spisync_trace("failed to resume spi, %d\r\n", ret);
		return -1;
	}
	spisync_trace("spi dma suspend flag is cleared, pumped new msg\r\n");
	return 0;
}
