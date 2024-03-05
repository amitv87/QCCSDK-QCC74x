#include "ip/lmac/src/tx/tx_swdesc.h"
#include "ip/lmac/src/tx/txl/txl_buffer.h"
#if NX_MFP
#include "ip/umac/src/mfp/mfp.h"
#endif
/// Structure containing FHOST control information for the present buffer
struct fhost_tx_ctrl_tag
{
    /// Pointer to the network stack buffer structure
    void *buf;
    /// TX confirmation callback (Only used for mgmt frame)
    void *cfm_cb;
    /// TX confirmation callback argument
    void *cfm_cb_arg;
    /// RX environment structure that is resent
    void *buf_rx;
    /// Buffer timeout
    uint32_t timeout;
};

/// Structure mapped into the TX buffer for internal handling
struct fhost_tx_desc_tag
{
    /// Chained list element
    struct co_list_hdr hdr;
    /// FHOST TX control information
    struct fhost_tx_ctrl_tag ctrl;
    /// TX SW descriptor passed to MAC
    struct txdesc_host txdesc;
    /// TX HW descriptor attached to the MPDU
    struct tx_hw_desc hw_desc;
    /// Buffer header structure - Must be the last element of the structure as it shall
    /// be placed just before the payload space
    struct txl_buffer_tag buffer;
};

void tx_desc_init_for_fhost(struct fhost_tx_desc_tag *desc, int seg_cnt, uint32_t *seg_addr, uint16_t *seg_len)
{
    struct tx_hw_desc *hwdesc;
    struct txl_buffer_tag *buffer;

    buffer = &desc->buffer;
    hwdesc = &desc->hw_desc;

    // Then save first segment info in HW desc
    hwdesc->thd.datastartptr = 0;
    hwdesc->thd.dataendptr = 0;

    // and chain remaining segment as payload buffer descriptor
    hwdesc->thd.first_pbd_ptr = CPU2HW(&buffer->pbd[0]);
    for (int i = 0; i < seg_cnt; i++)
    {
        buffer->pbd[i].upatterntx = TX_PAYLOAD_DESC_PATTERN;
        buffer->pbd[i].next = 0;
        buffer->pbd[i].datastartptr = CPU2HW(seg_addr[i]);
        buffer->pbd[i].dataendptr = CPU2HW(seg_addr[i]) + seg_len[i] - 1;
        buffer->pbd[i].bufctrlinfo = 0;
        if (i > 0)
            buffer->pbd[i - 1].next = CPU2HW(&buffer->pbd[i]);
    }
}

void fhost_tx_get_hw_desc_pointers(hostid_t hostid,
                                  struct tx_hw_desc **hwdesc,
                                  struct txl_buffer_tag **buffer)
{
    struct fhost_tx_desc_tag *desc = hostid;
    *hwdesc = &desc->hw_desc;
    *buffer = &desc->buffer;
}
