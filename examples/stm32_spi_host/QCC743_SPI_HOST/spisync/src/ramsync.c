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
extern DMA_HandleTypeDef handle_GPDMA1_Channel0;
extern DMA_HandleTypeDef handle_GPDMA1_Channel1;
static DMA_QListTypeDef spi_txdma_ll;
static DMA_QListTypeDef spi_rxdma_ll;
static lramsync_ctx_t *g_ctx;

/* ------------------- platform port ------------------- */
#define rsl_malloc(x)	malloc_aligned_with_padding(x, 32)
#define rsl_free(x)		free_aligned_with_padding(x)

/* ------------------- internal type or rodata ------------------- */
struct _ramsync_low_priv {
	DMA_NodeTypeDef *tx_llis;
	DMA_NodeTypeDef *rx_llis;
};

static void dma_xfer_done(DMA_HandleTypeDef *hdma)
{
	if (hdma == &handle_GPDMA1_Channel0) {
	    if (g_ctx->tx_cb)
	        g_ctx->tx_cb(g_ctx->tx_arg);
	} else if (hdma == &handle_GPDMA1_Channel1) {
	    if (g_ctx->rx_cb)
	        g_ctx->rx_cb(g_ctx->rx_arg);
	} else {
		printf("unknown dma %p done\r\n", hdma);
	}
}

static void dma_error_cb(DMA_HandleTypeDef *hdma)
{
	if (hdma == &handle_GPDMA1_Channel0) {
		printf("spi txdma xfer error %x\r\n", hdma->ErrorCode);
	} else if (hdma == &handle_GPDMA1_Channel1) {
		printf("spi rxdma xfer error %x\r\n", hdma->ErrorCode);
	} else {
		printf("unknown dma %p, xfer error %x\r\n",
				hdma, hdma ? hdma->ErrorCode : 0);
	}
}

static void spi_hw_init(const lramsync_spi_config_t *config)
{
}

static void spi_dma_init(lramsync_ctx_t *ctx)
{
	uint32_t i;
	HAL_StatusTypeDef ret = HAL_OK;
	DMA_NodeConfTypeDef pNodeConfig;
	struct _ramsync_low_priv *priv = ctx->priv;


	HAL_DMAEx_List_ResetQ(&spi_txdma_ll);
	/* Set spi tx DMA node configuration ################################################*/
	pNodeConfig.NodeType = DMA_GPDMA_LINEAR_NODE;
	pNodeConfig.Init.Request = GPDMA1_REQUEST_SPI1_TX;
	pNodeConfig.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
	pNodeConfig.Init.Direction = DMA_MEMORY_TO_PERIPH;
	pNodeConfig.Init.SrcInc = DMA_SINC_INCREMENTED;
	pNodeConfig.Init.DestInc = DMA_DINC_FIXED;
	pNodeConfig.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_BYTE;
	pNodeConfig.Init.DestDataWidth = DMA_DEST_DATAWIDTH_BYTE;
	pNodeConfig.Init.SrcBurstLength = 1;
	pNodeConfig.Init.DestBurstLength = 1;
	pNodeConfig.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0|DMA_DEST_ALLOCATED_PORT0;
	pNodeConfig.Init.TransferEventMode = DMA_TCEM_EACH_LL_ITEM_TRANSFER;
	pNodeConfig.TriggerConfig.TriggerPolarity = DMA_TRIG_POLARITY_MASKED;
	pNodeConfig.DataHandlingConfig.DataExchange = DMA_EXCHANGE_NONE;
	pNodeConfig.DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED;
	pNodeConfig.DstAddress = (uint32_t)&SPI1->TXDR;

	for (i = 0; i < ctx->items_tx; i++) {
		pNodeConfig.SrcAddress = (uint32_t)ctx->node_tx[i].buf;
		pNodeConfig.DataSize = ctx->node_tx[i].len;

		/* Build this Node */
		ret = HAL_DMAEx_List_BuildNode(&pNodeConfig, &priv->tx_llis[i]);
		if (ret != HAL_OK) {
			printf("failed to build tx dma node %d, %d\r\n", i, ret);
			return;
		}

		/* Insert spi_txdma_lli to Queue */
		ret = HAL_DMAEx_List_InsertNode_Tail(&spi_txdma_ll, &priv->tx_llis[i]);
		if (ret != HAL_OK) {
			printf("failed to queue tx dma node %d, %d\r\n", i, ret);
			return;
		}

		if (i == 0) {
			ret = HAL_DMAEx_List_SetCircularModeConfig(&spi_txdma_ll,
					&priv->tx_llis[i]);
			if (ret != HAL_OK) {
				printf("failed to set first tx dma node, %d\r\n", ret);
				return;
			}
		}
	}
	printf("tx lli number %d, head %p\r\n", spi_txdma_ll.NodeNumber, spi_txdma_ll.FirstCircularNode);

	HAL_DMAEx_List_ResetQ(&spi_rxdma_ll);
	/* Set spi rx DMA node configuration ################################################*/
	pNodeConfig.NodeType = DMA_GPDMA_LINEAR_NODE;
	pNodeConfig.Init.Request = GPDMA1_REQUEST_SPI1_RX;
	pNodeConfig.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
	pNodeConfig.Init.Direction = DMA_PERIPH_TO_MEMORY;
	pNodeConfig.Init.SrcInc = DMA_SINC_FIXED;
	pNodeConfig.Init.DestInc = DMA_DINC_INCREMENTED;
	pNodeConfig.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_BYTE;
	pNodeConfig.Init.DestDataWidth = DMA_DEST_DATAWIDTH_BYTE;
	pNodeConfig.Init.SrcBurstLength = 1;
	pNodeConfig.Init.DestBurstLength = 1;
	pNodeConfig.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0|DMA_DEST_ALLOCATED_PORT0;
	pNodeConfig.Init.TransferEventMode = DMA_TCEM_EACH_LL_ITEM_TRANSFER;
	pNodeConfig.TriggerConfig.TriggerPolarity = DMA_TRIG_POLARITY_MASKED;
	pNodeConfig.DataHandlingConfig.DataExchange = DMA_EXCHANGE_NONE;
	pNodeConfig.DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED;
	pNodeConfig.SrcAddress = (uint32_t)&SPI1->RXDR;

	for (i = 0; i < ctx->items_rx; i++) {
		pNodeConfig.DstAddress = (uint32_t)ctx->node_rx[i].buf;
		pNodeConfig.DataSize = ctx->node_rx[i].len;

		/* Build this Node */
		ret = HAL_DMAEx_List_BuildNode(&pNodeConfig, &priv->rx_llis[i]);
		if (ret != HAL_OK) {
			printf("failed to build rx dma node %d, %d\r\n", i, ret);
			return;
		}

		/* Insert spi_rxdma_lli to Queue */
		ret = HAL_DMAEx_List_InsertNode_Tail(&spi_rxdma_ll, &priv->rx_llis[i]);
		if (ret != HAL_OK) {
			printf("failed to queue rx dma node %d, %d\r\n", i, ret);
			return;
		}

		if (i == 0) {
			ret = HAL_DMAEx_List_SetCircularModeConfig(&spi_rxdma_ll,
					&priv->rx_llis[i]);
			if (ret != HAL_OK) {
				printf("failed to set first rx dma node, %d\r\n", ret);
				return;
			}
		}
	}
	printf("rx lli number %d, head %p\r\n", spi_rxdma_ll.NodeNumber, spi_rxdma_ll.FirstCircularNode);

	if (HAL_DMAEx_List_LinkQ(&handle_GPDMA1_Channel0, &spi_txdma_ll) != HAL_OK)
		printf("failed to link txdma\r\n");

	if (HAL_DMAEx_List_LinkQ(&handle_GPDMA1_Channel1, &spi_rxdma_ll) != HAL_OK)
		printf("failed to link rxdma\r\n");

	HAL_DMA_RegisterCallback(&handle_GPDMA1_Channel0, HAL_DMA_XFER_CPLT_CB_ID, dma_xfer_done);
	HAL_DMA_RegisterCallback(&handle_GPDMA1_Channel0, HAL_DMA_XFER_ERROR_CB_ID, dma_error_cb);
	HAL_DMA_RegisterCallback(&handle_GPDMA1_Channel1, HAL_DMA_XFER_CPLT_CB_ID, dma_xfer_done);
	HAL_DMA_RegisterCallback(&handle_GPDMA1_Channel1, HAL_DMA_XFER_ERROR_CB_ID, dma_error_cb);

	ctx->dma_tx_chan = &handle_GPDMA1_Channel0;
	ctx->dma_rx_chan = &handle_GPDMA1_Channel1;
}

static int spi_dma_start(lramsync_ctx_t *ctx)
{
	/* start DMA engine */
	if (HAL_DMAEx_List_Start_IT(ctx->dma_tx_chan) != HAL_OK)
		printf("failed to start spi txdma\r\n");
	else
		printf("started spi txdma\r\n");

	if (HAL_DMAEx_List_Start_IT(ctx->dma_rx_chan) != HAL_OK)
		printf("failed to start spi rxdma\r\n");
	else
		printf("started spi rxdma\r\n");

	/* start spi and trigger DMA */
	LL_SPI_EnableDMAReq_TX(hspi1.Instance);
	LL_SPI_EnableDMAReq_RX(hspi1.Instance);
	LL_SPI_Enable(hspi1.Instance);
	LL_SPI_StartMasterTransfer(hspi1.Instance);
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

int lramsync_start(lramsync_ctx_t *ctx)
{
    spi_dma_start(ctx);
    return 0;
}

int lramsync_reset(lramsync_ctx_t *ctx)
{
	HAL_StatusTypeDef status;

    if (ctx == NULL)
        return -1;

    printf("lramsync_reset\r\n");
    /* abort SPI ongoing transfer */
    status = HAL_SPI_Abort(&hspi1);
    if (status != HAL_OK)
    	printf("failed to abort spi, %d\r\n", status);

    /* abort and reconfig */
    status = HAL_DMA_Abort(ctx->dma_tx_chan);
    if (status != HAL_OK)
    	printf("failed to abort tx dma channel, %d\r\n", status);
    status = HAL_DMA_Abort(ctx->dma_rx_chan);
    if (status != HAL_OK)
        printf("failed to abort rx dma channel, %d\r\n", status);

    spi_dma_init(ctx);
    return 0;
}

int lramsync_deinit(lramsync_ctx_t *ctx)
{
    struct _ramsync_low_priv *priv;
    struct qcc74x_device_s *spi_dev;

    if (ctx == NULL)
        return -1;

    priv = (struct _ramsync_low_priv *)ctx->priv;
    printf("lramsync deinit is not supported yet\r\n");
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
    /* tx lli config */
    priv->tx_llis = rsl_malloc(sizeof(DMA_NodeTypeDef) * ctx->items_tx);
    if (priv->tx_llis == NULL)
        goto rsl_init_err;

    /* rx lli config */
    priv->rx_llis = rsl_malloc(sizeof(DMA_NodeTypeDef) * ctx->items_rx);
    if (priv->rx_llis == NULL)
        goto rsl_init_err;

    g_ctx = ctx;
    spi_hw_init(ctx->cfg);
    spi_dma_init(ctx);
    spi_dma_start(ctx);
    printf("lramsync_init done\r\n");
    return 0;

rsl_init_err:
    if (ctx && ctx->node_rx)
        rsl_free(ctx->node_rx);

    if (ctx && ctx->node_tx)
        rsl_free(ctx->node_tx);

    if (priv && priv->tx_llis)
        rsl_free(priv->tx_llis);

    if (priv && priv->rx_llis)
        rsl_free(priv->rx_llis);

    if (priv)
        rsl_free(priv);
    return -1;
}

int lramsync_suspend(lramsync_ctx_t *ctx)
{
    printf("%s\r\n", __func__);
    HAL_DMAEx_Suspend(ctx->dma_tx_chan);
    HAL_DMAEx_Suspend(ctx->dma_rx_chan);
    return 0;
}

int lramsync_resume(lramsync_ctx_t *ctx)
{
    printf("%s\r\n", __func__);
    HAL_DMAEx_Resume(ctx->dma_rx_chan);
    HAL_DMAEx_Resume(ctx->dma_tx_chan);
    return 0;
}

