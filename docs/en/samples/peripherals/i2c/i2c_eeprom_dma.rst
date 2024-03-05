I2C - eeprom_dma
====================

This demo mainly introduces how I2C uses DMA to read and write eeprom.

Hardware Connection
-----------------------------

The gpio used in this demo refers to ``board_i2c0_gpio_init``. Connect the eeprom module to the development board. The specific pin connection method is as follows (taking QCC743 as an example):

.. table:: Hardware connections
    :widths: 50, 50
    :width: 80%
    :align: center

    +-------------------+------------------+
    | Signal            | eeprom signal    |
    +===================+==================+
    | SCL(GPIO14)       | SCL              |
    +-------------------+------------------+
    | SDA(GPIO15)       | SDA              |
    +-------------------+------------------+
    | GND               | GND              |
    +-------------------+------------------+
    | VCC               | VCC              |
    +-------------------+------------------+

Software Implementation
-----------------------------

For more detailed code, please refer to **examples/peripherals/i2c/i2c_eeprom_dma**

.. code-block:: C
    :linenos:

    board_init();

- ``board_init`` will turn on the I2C IP clock and select the I2C clock source and frequency division.

.. code-block:: C
    :linenos:

    board_i2c0_gpio_init();

- Configure related pins as ``I2C`` function

.. code-block:: C
    :linenos:

    /* Send and receive buffer init */
    for (size_t i = 0; i < 32; i++) {
        ((uint8_t *)send_buffer)[i] = i;
        ((uint8_t *)receive_buffer)[i] = 0;
    }

    i2c0 = qcc74x_device_get_by_name("i2c0");

    qcc74x_i2c_init(i2c0, 400000);
    qcc74x_i2c_link_txdma(i2c0, true);
    qcc74x_i2c_link_rxdma(i2c0, true);

- Initialize send and receive buffers
- Get the ``i2c0`` handle and initialize the i2c0 frequency to 400K
- ``qcc74x_i2c_link_txdma(i2c0, true)`` enables I2C TX DMA function
- ``qcc74x_i2c_link_rxdma(i2c0, true)`` enables I2C RX DMA function

.. code-block:: C
    :linenos:

    /* Write page 0 */
    dma0_ch0 = qcc74x_device_get_by_name("dma0_ch0");

    struct qcc74x_dma_channel_config_s tx_config;

    tx_config.direction = DMA_MEMORY_TO_PERIPH;
    tx_config.src_req = DMA_REQUEST_NONE;
    tx_config.dst_req = DMA_REQUEST_I2C0_TX;
    tx_config.src_addr_inc = DMA_ADDR_INCREMENT_ENABLE;
    tx_config.dst_addr_inc = DMA_ADDR_INCREMENT_DISABLE;
    tx_config.src_burst_count = DMA_BURST_INCR1;
    tx_config.dst_burst_count = DMA_BURST_INCR1;
    tx_config.src_width = DMA_DATA_WIDTH_32BIT;
    tx_config.dst_width = DMA_DATA_WIDTH_32BIT;
    qcc74x_dma_channel_init(dma0_ch0, &tx_config);

    qcc74x_dma_channel_irq_attach(dma0_ch0, dma0_ch0_isr, NULL);

- For TX, the DMA configuration is as follows: the transfer direction (direction) is memory to peripheral (MEMORY_TO_PERIPH), the source request (src_req) is memory, and the target request (dst_req) is DMA_REQUEST_I2C0_TX
- Call ``qcc74x_dma_channel_init(dma0_ch0, &tx_config)`` to initialize DMA
- Call ``qcc74x_dma_channel_irq_attach(dma0_ch0, dma0_ch0_isr, NULL)`` to register dma channel 0 interrupt

.. code-block:: C
    :linenos:

    struct qcc74x_dma_channel_lli_pool_s tx_llipool[20]; /* max trasnfer size 4064 * 20 */
    struct qcc74x_dma_channel_lli_transfer_s tx_transfers[1];
    tx_transfers[0].src_addr = (uint32_t)send_buffer;
    tx_transfers[0].dst_addr = (uint32_t)DMA_ADDR_I2C0_TDR;
    tx_transfers[0].nbytes = 32;
    qcc74x_dma_channel_lli_reload(dma0_ch0, tx_llipool, 20, tx_transfers, 1);

    msgs[0].addr = 0x50;
    msgs[0].flags = I2C_M_NOSTOP;
    msgs[0].buffer = subaddr;
    msgs[0].length = 2;

    msgs[1].addr = 0x50;
    msgs[1].flags = I2C_M_DMA;
    msgs[1].buffer = NULL;
    msgs[1].length = 32;
    qcc74x_i2c_transfer(i2c0, msgs, 2);

    qcc74x_dma_channel_start(dma0_ch0);

- Allocate twenty lli memory pools, which can transfer up to 4064 * 20 bytes
- Configure a piece of memory (tx_transfers) for transmission, the source address (src_addr) is the memory address (send_buffer) where the sent data is stored, and the destination address (dst_addr) is the I2C TX FIFO address (DMA_ADDR_I2C0_TDR)
- Call ``qcc74x_dma_channel_lli_reload(dma0_ch0, tx_llipool, 20, tx_transfers, 1)`` to initialize
- Call ``qcc74x_i2c_transfer(i2c0, msgs, 2)`` to enable I2C transfer
- Call ``qcc74x_dma_channel_start(dma0_ch0)`` to start DMA transfer

.. code-block:: C
    :linenos:

    /* Read page 0 */
    dma0_ch1 = qcc74x_device_get_by_name("dma0_ch1");

    struct qcc74x_dma_channel_config_s rx_config;

    rx_config.direction = DMA_PERIPH_TO_MEMORY;
    rx_config.src_req = DMA_REQUEST_I2C0_RX;
    rx_config.dst_req = DMA_REQUEST_NONE;
    rx_config.src_addr_inc = DMA_ADDR_INCREMENT_DISABLE;
    rx_config.dst_addr_inc = DMA_ADDR_INCREMENT_ENABLE;
    rx_config.src_burst_count = DMA_BURST_INCR1;
    rx_config.dst_burst_count = DMA_BURST_INCR1;
    rx_config.src_width = DMA_DATA_WIDTH_32BIT;
    rx_config.dst_width = DMA_DATA_WIDTH_32BIT;
    qcc74x_dma_channel_init(dma0_ch1, &rx_config);

    qcc74x_dma_channel_irq_attach(dma0_ch1, dma0_ch1_isr, NULL);

- For RX, the DMA configuration is as follows: the transfer direction (direction) is peripheral to memory (PERIPH_TO_MEMORY), the source request (src_req) is DMA_REQUEST_I2C0_RX, and the target request (dst_req) is memory
- Call ``qcc74x_dma_channel_init(dma0_ch1, &rx_config)`` to initialize DMA
- Call ``qcc74x_dma_channel_irq_attach(dma0_ch1, dma0_ch1_isr, NULL)`` to register dma channel 1 interrupt

.. code-block:: C
    :linenos:

    struct qcc74x_dma_channel_lli_pool_s rx_llipool[20];
    struct qcc74x_dma_channel_lli_transfer_s rx_transfers[1];
    rx_transfers[0].src_addr = (uint32_t)DMA_ADDR_I2C0_RDR;
    rx_transfers[0].dst_addr = (uint32_t)receive_buffer;
    rx_transfers[0].nbytes = 32;

    qcc74x_dma_channel_lli_reload(dma0_ch1, rx_llipool, 20, rx_transfers, 1);

    msgs[1].addr = 0x50;
    msgs[1].flags = I2C_M_DMA | I2C_M_READ;
    msgs[1].buffer = NULL;
    msgs[1].length = 32;
    qcc74x_i2c_transfer(i2c0, msgs, 2);

    qcc74x_dma_channel_start(dma0_ch1);

- Allocate twenty lli memory pools, which can transfer up to 4064 * 20 bytes
- Configure a piece of memory (rx_transfers) for transmission, the source address (src_addr) is the I2C RX FIFO address (DMA_ADDR_I2C0_RDR), and the destination address (dst_addr) is the memory address (receive_buffer) where the received data is stored.
- Call ``qcc74x_dma_channel_lli_reload(dma0_ch1, rx_llipool, 20, rx_transfers, 1)`` to initialize
- Call ``qcc74x_i2c_transfer(i2c0, msgs, 2)`` to enable I2C transfer
- Call ``qcc74x_dma_channel_start(dma0_ch1)`` to start DMA transfer

.. code-block:: C
    :linenos:

    while (dma_tc_flag1 == 0) {
    }
    while ((qcc74x_i2c_get_intstatus(i2c0) & I2C_INTSTS_END) == 0) {
    }
    qcc74x_i2c_deinit(i2c0);

- After data transfer is completed, reset the I2C module

.. code-block:: C
    :linenos:

    /* Check read data */
    for (uint8_t i = 0; i < 32; i++) {
        if (((uint8_t *)send_buffer)[i] != ((uint8_t *)receive_buffer)[i]) {
            printf("check fail, %d write: %02x, read: %02x\r\n", i, ((uint8_t *)send_buffer)[i], ((uint8_t *)receive_buffer)[i]);
        }
    }

- Check whether the data sent and read are consistent

Compile and Program
-----------------------------

Reference :ref:`get_started`

Experimental Phenomena
-----------------------------

Press the RST button and after the data transfer is completed, "write over", "read over" and "check over" are printed.
