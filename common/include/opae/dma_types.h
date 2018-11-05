// Copyright(c) 2017-2018, Intel Corporation
//
// Redistribution  and  use  in source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of  source code  must retain the  above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name  of Intel Corporation  nor the names of its contributors
//   may be used to  endorse or promote  products derived  from this  software
//   without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
// IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
// LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
// CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
// SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
// INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
// CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifndef __USE_XOPEN2K
#define __USE_XOPEN2K
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>

#include "opae/types.h"
#undef _GNU_SOURCE

#ifndef DMA_TYPES_H
#define DMA_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * typedef dma_cookie_t - an opaque DMA cookie
 *
 * if dma_cookie_t is >0 it's a DMA request cookie, <0 it's an error code
 */
typedef int32_t fpga_dma_cookie_t;
#define FPGA_DMA_MIN_COOKIE	1

static inline int fpga_dma_submit_error(fpga_dma_cookie_t cookie)
{
	return cookie < 0 ? cookie : 0;
}

/**
 * enum fpga_dma_transaction_t DMA transaction type
 *
 * Indicate the required DMA transaction type.
 * For streaming: host to AFU, AFU to host, FPGA local mem to AFU, AFU to FPGA local mem
 * memory to memory: host to fpga, fpga to host, fpga internal  memory copy
 */
typedef enum {
	HOST_MM_TO_FPGA_ST = 0, // sreaming, host to AFU
	FPGA_ST_TO_HOST_MM,     // streaming, AFU to host
	FPGA_MM_TO_FPGA_ST,     // streaming, FPGA local mem to AFU
	FPGA_ST_TO_FPGA_MM,     // streaming, AFU to FPGA local mem
	HOST_TO_FPGA_MM,        // Memory mapped FPGA interface
	FPGA_TO_HOST_MM,        // Memory mapped FPGA interface
	FPGA_TO_FPGA_MM,        // Memory mapped FPGA interface
	TRANS_NONE,
	FPGA_MAX_TRANSFER_TYPE,
	TERMINATE_THREAD
} fpga_dma_transfer_type_t;

/**
 * enum fpga_dma_[tx|rx]_ctrl_t - DMA flags to augment operation preparation,
 *  control completion, and communicate status.
 * @DMA_PREP_INTERRUPT - trigger an interrupt (callback) upon completion of
 *  this transaction
 * @DMA_TX_PREP_FENCE - tell the driver that subsequent operations depend
 *  on the result of this operation
 * @DMA_TX_CMD: tell the driver that the data passed to DMA API is command
 *  data and the descriptor should be in different format from normal
 *  data descriptors.
 */
typedef enum {
	DMA_TX_NO_PACKET = 0, // conventional MM DMA
	DMA_TX_GENERATE_SOP,
	DMA_TX_GENERATE_EOP,
	DMA_TX_GENERATE_SOP_AND_EOP,
	DMA_TX_GENERATE_INTERRUPT,
	DMA_TX_GENERATE_FENCE,
	DMA_TX_CMD,
	DMA_TX_MAX_CTRL
} fpga_dma_tx_ctrl_t;

typedef enum {
	DMA_RX_NO_PACKET = 0, // conventional MM DMA
	DMA_RX_END_ON_EOP,
	DMA_RX_GENERATE_INTERRUPT,
	DMA_RX_GENERATE_FENCE,
	DMA_RX_CMD,
	DMA_RX_MAX_CTRL
} fpga_dma_rx_ctrl_t;

// Channel types
typedef enum {
	DMA_INVALID_TYPE = 0,
	DMA_TX_ST,
	DMA_RX_ST,
	DMA_MM,
} fpga_dma_channel_type_t;

/**
 * struct dma_chan - devices supply DMA channels, clients use them
 * @device: ptr to the dma device who supplies this channel, always !%NULL
 * @cookie: last cookie value returned to client
 * @completed_cookie: last completed cookie for this channel
 * @context: private data for certain client-channel associations
*/

typedef struct _fpga_dma_channel {

	int ch_id;
	fpga_dma_channel_type_t ch_type;
	uint32_t index;

	fpga_dma_cookie_t cookie;
	fpga_dma_cookie_t completed_cookie;

	void *context;

} fpga_dma_channel;

/**
 * enum fpga_transf_status_t - DMA transaction status
 * @DMA_TRANS_COMPLETE: transaction completed
 * @DMA_TRANS_IN_PROGRESS: transaction not yet processed
 * @DMA_TRANS_PAUSED: transaction is paused
 * @DMA_TRANS_ERROR: transaction failed
 */
typedef enum {
	DMA_TRANS_IN_PROGRESS = 0,
	DMA_TRANS_NOT_IN_PROGRESS = 1,
	DMA_TRANS_COMPLETE,
	DMA_TRANS_PAUSED,
	DMA_TRANS_ERROR
} fpga_transf_status_t;

/**
     * enum dma_desc_metadata_mode - per descriptor metadata mode types supported
     * @DESC_METADATA_CLIENT - the metadata buffer is allocated/provided by the
     *  client driver and it is attached (via the dmaengine_desc_attach_metadata()
     *  helper) to the descriptor.
     *
     * Note: the two mode is not compatible and clients must use one mode for a
     * descriptor.
     */
enum dma_desc_metadata_mode {
	DESC_METADATA_NONE = 0,
	DESC_METADATA_CLIENT = (1 << 1),
	DESC_METADATA_ENGINE = (1 << 2),
};

enum dmaengine_tx_result {
	DMA_TRANS_NOERROR = 0,		/* SUCCESS */
	DMA_TRANS_READ_FAILED,		/* Source DMA read failed */
	DMA_TRANS_WRITE_FAILED,		/* Destination DMA write failed */
	DMA_TRANS_ABORTED,		/* Op never submitted / aborted */
};

struct dmaengine_result {
	enum dmaengine_tx_result result;
	uint32_t residue;
};

// Callbacks for asynchronous DMA transfers
typedef void (*fpga_dma_async_tx_cb_result)(void *context,
					    struct dmaengine_result *result);

typedef void (*fpga_dma_async_tx_cb)(void *context);

/**
 * struct fpga_dma_async_transfer - async transaction descriptor
 * ---dma generic offload fields---
 * @cookie: tracking cookie for this transaction, set to -EBUSY if
 *	this tx is sitting on a dependency list
 * @flags: flags to augment operation preparation, control completion, and
 * 	communicate status
 * @phys: physical address of the descriptor
 * @chan: target channel for this operation
 * @tx_submit: accept the descriptor, assign ordered cookie and mark the
 * descriptor pending. To be pushed on .issue_pending() call
 * @callback: routine to call after this operation is complete
 * @callback_param: general parameter to pass to the callback routine
 * ---async_tx api specific fields---
 * @next: at completion submit this descriptor
 * @parent: pointer to the next level up in the dependency chain
 * @lock: protect the parent and next pointers
 */
typedef struct _fpga_dma_async_transfer {
	/* General information */
	fpga_dma_cookie_t cookie;
	uint64_t buf;
	size_t len;
	uint64_t phys;
	fpga_dma_channel *chan;
	fpga_dma_transfer_type_t transfer_type;

	/* Control operations */
	fpga_dma_tx_ctrl_t tx_flags;
	fpga_dma_rx_ctrl_t rx_flags;
	fpga_dma_cookie_t (*fpga_dma_post_descriptors)(struct _fpga_dma_async_transfer *tx);
	int (*fpga_dma_transfer_free)(struct _fpga_dma_async_transfer *tx);
	fpga_dma_async_tx_cb cb;
	fpga_dma_async_tx_cb_result cb_result;
	int fd;
	void *context;

	/* Transfer-local scatter-gather local */
	struct scatterlist *sgl;
	uint64_t sg_len;

	/* When using metadata */
	enum dma_desc_metadata_mode desc_metadata_mode;
	struct dma_descriptor_metadata_ops *metadata_ops;

	/* For maintaining a link list */
	struct _fpga_dma_async_transfer *next;
	struct _fpga_dma_async_transfer *parent;

	/* When locked, the transaction is in progress */
	sem_t tf_status;
	pthread_spinlock_t lock;
} fpga_dma_async_transfer;

struct fpga_dma_transfer_metadata_ops {
	int (*attach)(fpga_dma_async_transfer *desc, void *data,
		      size_t len);

	void *(*get_ptr)(fpga_dma_async_transfer *desc,
			 size_t *payload_len, size_t *max_len);
	int (*set_len)(fpga_dma_async_transfer *desc,
		       size_t payload_len);
};

// Handle to complete DMA engine with multiple channels
typedef void *fpga_dma_handle;

// Handle to single DMA channel
typedef void *fpga_dma_channel_handle;

// Opaque type for DMA descriptors
typedef void *fpga_dma_desc;

// Opaque type for DMA transfer
typedef void *fpga_dma_transfer;

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif
