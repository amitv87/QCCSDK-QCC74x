
#ifndef  __CKS_REG_H__
#define  __CKS_REG_H__

/****************************************************************************
 * Pre-processor Definitions
****************************************************************************/

/* Register offsets *********************************************************/

#define CKS_CONFIG_OFFSET                                       (0x0)/* cks_config */
#define CKS_DATA_IN_OFFSET                                      (0x4)/* data_in */
#define CKS_OUT_OFFSET                                          (0x8)/* cks_out */

/* Register Bitfield definitions *****************************************************/

/* 0x0 : cks_config */
#define CKS_CR_CKS_CLR                                          (1<<0U)
#define CKS_CR_CKS_BYTE_SWAP                                    (1<<1U)

/* 0x4 : data_in */
#define CKS_DATA_IN_SHIFT                                       (0U)
#define CKS_DATA_IN_MASK                                        (0xff<<CKS_DATA_IN_SHIFT)

/* 0x8 : cks_out */
#define CKS_OUT_SHIFT                                           (0U)
#define CKS_OUT_MASK                                            (0xffff<<CKS_OUT_SHIFT)


#endif  /* __CKS_REG_H__ */
