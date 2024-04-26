#ifndef __SPISYNC_CONFIG_H__
#define __SPISYNC_CONFIG_H__

typedef struct spisync_config {
    const char *spi_name;
    const char *spi_tx_dmach;
    const char *spi_rx_dmach;

    uint8_t  spi_port;
    uint8_t  spi_mode;
    uint32_t spi_speed;

    uint32_t spi_dma_req_tx;
    uint32_t spi_dma_req_rx;
    uint32_t spi_dma_tdr;
    uint32_t spi_dma_rdr;
} spisync_config_t;

#endif
