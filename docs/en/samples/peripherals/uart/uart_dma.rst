UART - dma
====================

This demo mainly demonstrates the UART dma mode sending and receiving function.

Hardware Connection
-----------------------------

- Chip UART TX pin connected to USB2TTL module RX
- Chip UART RX pin connected to USB2TTL module TX

The gpio used in this demo is as follows:

.. table:: GPIO
    :widths: 30, 30, 40
    :width: 80%
    :align: center

    +----------+-----------+---------------------------+
    |   Signal | Chip      |           GPIO            |
    +==========+===========+===========================+
    | UART1_TX | QCC743    | GPIO 23                   |
    +----------+-----------+---------------------------+
    | UART1_RX | QCC743    | GPIO 24                   |
    +----------+-----------+---------------------------+

Software Implementation
-----------------------------

For specific software code, see **examples/peripherals/uart/uart_podma**

.. code-block:: C
    :linenos:

    board_init();

- ``board_init`` will turn on the UART IP clock and select the UART clock source and frequency division.

.. code-block:: C
    :linenos:

    board_uartx_gpio_init();

- Configure the relevant pins as ``UARTx TX`` and ``UARTx RX`` functions. By default, the demo uses the UART1 peripheral.

.. code-block:: C
    :linenos:

    uartx = qcc74x_device_get_by_name(DEFAULT_TEST_UART);

    struct qcc74x_uart_config_s cfg;

    cfg.baudrate = 2000000;
    cfg.data_bits = UART_DATA_BITS_8;
    cfg.stop_bits = UART_STOP_BITS_1;
    cfg.parity = UART_PARITY_NONE;
    cfg.flow_ctrl = 0;
    cfg.tx_fifo_threshold = 7;
    cfg.rx_fifo_threshold = 7;
    qcc74x_uart_init(uartx, &cfg);

- Get the ``DEFAULT_TEST_UART`` handle and initialize the UART

.. code-block:: C
    :linenos:

    qcc74x_uart_link_txdma(uartx, true);
    qcc74x_uart_link_rxdma(uartx, true);

- Enable uart tx, rx dma function

.. code-block:: C
    :linenos:

    struct qcc74x_dma_channel_config_s config;

    config.direction = DMA_MEMORY_TO_PERIPH;
    config.src_req = DMA_REQUEST_NONE;
    config.dst_req = DEFAULT_TEST_UART_DMA_TX_REQUEST;
    config.src_addr_inc = DMA_ADDR_INCREMENT_ENABLE;
    config.dst_addr_inc = DMA_ADDR_INCREMENT_DISABLE;
    config.src_burst_count = DMA_BURST_INCR1;
    config.dst_burst_count = DMA_BURST_INCR1;
    config.src_width = DMA_DATA_WIDTH_8BIT;
    config.dst_width = DMA_DATA_WIDTH_8BIT;
    qcc74x_dma_channel_init(dma0_ch0, &config);

    struct qcc74x_dma_channel_config_s rxconfig;

    rxconfig.direction = DMA_PERIPH_TO_MEMORY;
    rxconfig.src_req = DEFAULT_TEST_UART_DMA_RX_REQUEST;
    rxconfig.dst_req = DMA_REQUEST_NONE;
    rxconfig.src_addr_inc = DMA_ADDR_INCREMENT_DISABLE;
    rxconfig.dst_addr_inc = DMA_ADDR_INCREMENT_ENABLE;
    rxconfig.src_burst_count = DMA_BURST_INCR1;
    rxconfig.dst_burst_count = DMA_BURST_INCR1;
    rxconfig.src_width = DMA_DATA_WIDTH_8BIT;
    rxconfig.dst_width = DMA_DATA_WIDTH_8BIT;
    qcc74x_dma_channel_init(dma0_ch1, &rxconfig);

    qcc74x_dma_channel_irq_attach(dma0_ch0, dma0_ch0_isr, NULL);
    qcc74x_dma_channel_irq_attach(dma0_ch1, dma0_ch1_isr, NULL);

- Configure ``DMA CH0`` as ``UARTx TX`` and ``DMA CH1`` as ``UARTx RX``.
- Register dma channel interrupt

.. code-block:: C
    :linenos:

    struct qcc74x_dma_channel_lli_pool_s tx_llipool[20]; /* max trasnfer size 4064 * 20 */
    struct qcc74x_dma_channel_lli_transfer_s tx_transfers[3];

    tx_transfers[0].src_addr = (uint32_t)src_buffer;
    tx_transfers[0].dst_addr = (uint32_t)DEFAULT_TEST_UART_DMA_TDR;
    tx_transfers[0].nbytes = 4100;

    tx_transfers[1].src_addr = (uint32_t)src2_buffer;
    tx_transfers[1].dst_addr = (uint32_t)DEFAULT_TEST_UART_DMA_TDR;
    tx_transfers[1].nbytes = 4100;

    tx_transfers[2].src_addr = (uint32_t)src3_buffer;
    tx_transfers[2].dst_addr = (uint32_t)DEFAULT_TEST_UART_DMA_TDR;
    tx_transfers[2].nbytes = 4100;

    struct qcc74x_dma_channel_lli_pool_s rx_llipool[20];
    struct qcc74x_dma_channel_lli_transfer_s rx_transfers[1];
    rx_transfers[0].src_addr = (uint32_t)DEFAULT_TEST_UART_DMA_RDR;
    rx_transfers[0].dst_addr = (uint32_t)receive_buffer;
    rx_transfers[0].nbytes = 50;

    qcc74x_dma_channel_lli_reload(dma0_ch0, tx_llipool, 20, tx_transfers, 3);
    qcc74x_dma_channel_lli_reload(dma0_ch1, rx_llipool, 20, rx_transfers, 1);
    qcc74x_dma_channel_start(dma0_ch0);
    qcc74x_dma_channel_start(dma0_ch1);

- Allocate an lli memory pool, the number is 20, and can transfer up to 4094 * 20 bytes
- Configure three non-contiguous memories for transmission
- Call ``qcc74x_dma_channel_lli_reload`` to initialize
- Call ``qcc74x_dma_channel_start`` to start transmission
- Wait for transfer to complete and enter interrupt

Compile and Program
-----------------------------

Reference :ref:`get_started`

Experimental Phenomena
-----------------------------


