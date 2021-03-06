/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _FSL_LPI2C_EDMA_H_
#define _FSL_LPI2C_EDMA_H_

#include "fsl_lpi2c.h"
#include "fsl_edma.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @addtogroup lpi2c_master_edma_driver
 * @{
 */

/* Forward declaration of the transfer descriptor and handle typedefs. */
typedef struct _lpi2c_master_edma_handle lpi2c_master_edma_handle_t;

/*!
 * @brief Master DMA completion callback function pointer type.
 *
 * This callback is used only for the non-blocking master transfer API. Specify the callback you wish to use
 * in the call to LPI2C_MasterCreateEDMAHandle().
 *
 * @param base The LPI2C peripheral base address.
 * @param handle Handle associated with the completed transfer.
 * @param completionStatus Either #kStatus_Success or an error code describing how the transfer completed.
 * @param userData Arbitrary pointer-sized value passed from the application.
 */
typedef void (*lpi2c_master_edma_transfer_callback_t)(LPI2C_Type *base,
                                                      lpi2c_master_edma_handle_t *handle,
                                                      status_t completionStatus,
                                                      void *userData);

/*!
 * @brief Driver handle for master DMA APIs.
 * @note The contents of this structure are private and subject to change.
 */
struct _lpi2c_master_edma_handle
{
    LPI2C_Type *base;                                         /*!< LPI2C base pointer. */
    bool isBusy;                                              /*!< Transfer state machine current state. */
    uint16_t commandBuffer[7];                                /*!< LPI2C command sequence. */
    lpi2c_master_transfer_t transfer;                         /*!< Copy of the current transfer info. */
    lpi2c_master_edma_transfer_callback_t completionCallback; /*!< Callback function pointer. */
    void *userData;                                           /*!< Application data passed to callback. */
    edma_handle_t *rx;                                        /*!< Handle for receive DMA channel. */
    edma_handle_t *tx;                                        /*!< Handle for transmit DMA channel. */
    edma_tcd_t tcds[2]; /*!< Software TCD. Two are allocated to provide enough room to align to 32-bytes. */
};

/*! @} */

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @addtogroup lpi2c_master_edma_driver
 * @{
 */

/*! @name Master DMA */
/*@{*/

/*!
 * @brief Create a new handle for the LPI2C master DMA APIs.
 *
 * The creation of a handle is for use with the DMA APIs. Once a handle
 * is created, there is not a corresponding destroy handle. If the user wants to
 * terminate a transfer, the LPI2C_MasterTransferAbortEDMA() API shall be called.
 *
 * For devices where the LPI2C send and receive DMA requests are OR'd together, the @a txDmaHandle
 * parameter is ignored and may be set to NULL.
 *
 * @param base The LPI2C peripheral base address.
 * @param[out] handle Pointer to the LPI2C master driver handle.
 * @param rxDmaHandle Handle for the eDMA receive channel. Created by the user prior to calling this function.
 * @param txDmaHandle Handle for the eDMA transmit channel. Created by the user prior to calling this function.
 * @param callback User provided pointer to the asynchronous callback function.
 * @param userData User provided pointer to the application callback data.
 */
void LPI2C_MasterCreateEDMAHandle(LPI2C_Type *base,
                                  lpi2c_master_edma_handle_t *handle,
                                  edma_handle_t *rxDmaHandle,
                                  edma_handle_t *txDmaHandle,
                                  lpi2c_master_edma_transfer_callback_t callback,
                                  void *userData);

/*!
 * @brief Performs a non-blocking DMA-based transaction on the I2C bus.
 *
 * The callback specified when the @a handle was created is invoked when the transaction has
 * completed.
 *
 * @param base The LPI2C peripheral base address.
 * @param handle Pointer to the LPI2C master driver handle.
 * @param transfer The pointer to the transfer descriptor.
 * @retval #kStatus_Success The transaction was started successfully.
 * @retval #kStatus_LPI2C_Busy Either another master is currently utilizing the bus, or another DMA
 *      transaction is already in progress.
 */
status_t LPI2C_MasterTransferEDMA(LPI2C_Type *base,
                                  lpi2c_master_edma_handle_t *handle,
                                  lpi2c_master_transfer_t *transfer);

/*!
 * @brief Returns number of bytes transferred so far.
 *
 * @param base The LPI2C peripheral base address.
 * @param handle Pointer to the LPI2C master driver handle.
 * @param[out] count Number of bytes transferred so far by the non-blocking transaction.
 * @retval #kStatus_Success
 * @retval #kStatus_NoTransferInProgress There is not a DMA transaction currently in progress.
 */
status_t LPI2C_MasterTransferGetCountEDMA(LPI2C_Type *base, lpi2c_master_edma_handle_t *handle, size_t *count);

/*!
 * @brief Terminates a non-blocking LPI2C master transmission early.
 *
 * @note It is not safe to call this function from an IRQ handler that has a higher priority than the
 *      eDMA peripheral's IRQ priority.
 *
 * @param base The LPI2C peripheral base address.
 * @param handle Pointer to the LPI2C master driver handle.
 * @retval #kStatus_Success A transaction was successfully aborted.
 * @retval #kStatus_LPI2C_Idle There is not a DMA transaction currently in progress.
 */
status_t LPI2C_MasterTransferAbortEDMA(LPI2C_Type *base, lpi2c_master_edma_handle_t *handle);

/*@}*/

/*! @} */

#if defined(__cplusplus)
}
#endif

#endif /* _FSL_LPI2C_EDMA_H_ */
