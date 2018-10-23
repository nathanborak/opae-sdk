// Copyright(c) 2018, Intel Corporation
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

/**
 * \fpga_dma_types.h
 * \brief FPGA DMA BBB Types
 *
 */

#include "dma_types_int.h"

#ifndef __RC_DMA_TYPES_H__
#define __RC_DMA_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define DMA_BUFFER_POOL_SIZE 8
#define FPGA_DMA_MAX_INFLIGHT_TRANSACTIONS 100000
#define FPGA_DMA_MAX_SMALL_BUFFERS 4

/* Data structures from DMA MM implementation */
typedef union {
    uint32_t reg;
    struct {
	uint32_t tx_channel:8;
	uint32_t generate_sop:1;
	uint32_t generate_eop:1;
	uint32_t park_reads:1;
	uint32_t park_writes:1;
	uint32_t end_on_eop:1;
	uint32_t eop_rvcd_irq_en:1;
	uint32_t transfer_irq_en:1;
	uint32_t early_term_irq_en:1;
	uint32_t trans_error_irq_en:8;
	uint32_t early_done_en:1;
	uint32_t wait_for_wr_rsp:1;
	uint32_t reserved_2:5;
	uint32_t go:1;
    };
} _fpga_dma_desc_ctrl_t;

/* The fpga_dma_desc encapsulateds all information about the descriptors */
#pragma pack(push, 1)
    typedef struct {
	//0x0
	uint32_t rd_address;
	//0x4
	uint32_t wr_address;
	//0x8
	uint32_t len;
	//0xC
	uint16_t seq_num;
	uint8_t rd_burst_count;
	uint8_t wr_burst_count;
	//0x10
	uint16_t rd_stride;
	uint16_t wr_stride;
	//0x14
	uint32_t rd_address_ext;
	//0x18
	uint32_t wr_address_ext;
	//0x1c
	_fpga_dma_desc_ctrl_t control;
} _fpga_dma_desc;
#pragma pack(pop)

// The `fpga_dma_transfer` objects encapsulate all the information about an transfer
typedef struct _fpga_dma_transfer {
	// Sync
	sem_pool_item *tf_semaphore;
	mutex_pool_item *tf_mutex;

	fpga_dma_channel_type_t ch_type;

	// Transfer information
	fpga_dma_transfer_type_t transfer_type;
	fpga_dma_tx_ctrl_t tx_ctrl;
	fpga_dma_rx_ctrl_t rx_ctrl;

	// Transfer callback and fd (fd used when cb is null)
	fpga_dma_async_tx_cb cb;
	int fd;
	void *context;

	// For non-preallocated buffers
	uint64_t src;
	uint64_t dst;
	uint64_t len;

	// For pre-allocated buffers
	uint64_t wsid;

	// Transfer pinned buffers
	struct _buffer_pool buffer_pool;
	uint32_t num_buffers;
	buffer_pool_item **buffers;
	buffer_pool_item *small_buffer;

	// Hold transfer state
	bool eop_status;
	size_t rx_bytes;

	// When locked, the transfer in progress
	sem_t tf_status;
} fpga_dma_transfer_t;

/* Queue dispatching transfers to the hardware */
typedef struct fpga_dma_transfer_q {
	int read_index;
	int write_index;
	fpga_dma_transfer *queue; // Transfers queue
	sem_t entries; // Counting semaphore, count represents available entries in queue
	pthread_mutex_t qmutex; // Gain exclusive access before queue operations
} fpga_dma_transfer_q;

/* Queue dispatching descriptors (PDs) to the hardware */
typedef struct fpga_dma_desc_q {
	int read_index;
	int write_index;
	fpga_dma_desc *queue; // transfers queue
	sem_t entries; // Counting semaphore, count represents available entries in queue
	pthread_mutex_t qmutex; // Gain exclusive access before queue operations
} fpga_dma_desc_q;

/* DMA-channel specific information which it is stored in the fpga_dma_channel_handle */
struct _fpga_dma_channel_mgr {
	// Channel type
	fpga_dma_channel_type_t ch_type;
	uint64_t ch_number;

	// DMA channel information
	uint64_t cpu_affinity;
	uint64_t ring_size;

	// CSR layout
	uint64_t dma_csr_base;
	uint64_t dma_desc_base;
	uint64_t dma_rsp_base;
	uint64_t dma_streaming_valve_base;

	// Address span extender
	uint64_t dma_ase_cntl_base;
	uint64_t dma_ase_data_base;

	// Channel-local threadpool and master
	thread_pool_item thread_pool;

	// Channel-local pinned buffers
	buffer_pool_item buffer_pool;

	// Channel-local queue of transfers
	fpga_dma_transfer_q dma_transfer_queue;

	// Channel-local index of the next available transfer in the dispatcher queue
	uint64_t next_avail_transfer_idx;

	// Channel-local total number of unused transfer in the dispatcher queue of transfers
	// Note: Count includes the next available transfer in
	// the dispatcher queue indexed by next_avail_transfer_idx
	uint64_t unused_transfer_count;
};

typedef struct qinfo {
	int read_index;
	int write_index;
	int num_free;
	fpga_dma_transfer_t *queue[FPGA_DMA_MAX_INFLIGHT_TRANSACTIONS];
	fpga_dma_transfer_t *free_queue[FPGA_DMA_MAX_INFLIGHT_TRANSACTIONS];
	// Counting semaphore, count represents available entries
	// in queue. mutex to gain exclusive access before queue operations
	sem_pool_item *q_semaphore;
	pthread_spinlock_t q_mutex;
} qinfo_t;

typedef struct _internal_channel_desc {
	fpga_dma_channel desc;
	uint32_t mmio_num;
	uint64_t mmio_offset;
	uint64_t mmio_va;
	uint64_t dma_base;
	uint64_t dma_csr_base;
	uint64_t dma_desc_base;
} internal_channel_desc;

// *MUST* be first element of each handle type
typedef struct _handle_common {
	uint32_t magic_id;
	struct _fpga_dma_handle *dma_h;
	fpga_handle fpga_h;
	internal_channel_desc *chan_desc;
	uint64_t dma_channel;
	fpga_dma_channel_type_t ch_type;

	// Interrupt event handle
	fpga_event_handle eh;
	// Transaction queue (model as a fixed-size circular buffer)
	qinfo_t transferRequestq;
} handle_common;

typedef struct _fpga_dma_handle {
	handle_common main_header;

	uint32_t num_open_channels;
	void **open_channels;

	// For protection for sem/mutex manipulation
	// TODO: These can be globals, right?
	pthread_spinlock_t sem_mutex;
	pthread_spinlock_t mutex_mutex;
	pthread_spinlock_t buffer_mutex;

	sem_pool_item *sem_in_use_head;
	sem_pool_item *sem_free_head;
	mutex_pool_item *mutex_in_use_head;
	mutex_pool_item *mutex_free_head;
	buffer_pool_item *buffer_in_use_head;
	buffer_pool_item *buffer_free_head;

	// Descriptors for channels (array)
	internal_channel_desc *chan_descs;
	uint32_t num_avail_channels;

	pthread_t completion_thread_id;
	sem_t completion_thread_sem;

	pthread_t m2s_thread_id;
	sem_t m2s_thread_sem;
	pthread_t s2m_thread_id;
	sem_t s2m_thread_sem;
	pthread_t m2m_thread_id;
	sem_t m2m_thread_sem;
	// Transaction completion queue (model as a fixed-size circular buffer)
	qinfo_t transferCompleteq;

	struct sigaction old_action;
	volatile uint32_t *CsrControl;

	uint32_t num_smalls;
} fpga_dma_handle_t;

typedef struct _m2s_dma_handle {
	handle_common header;
} m2s_dma_handle_t;

typedef struct _s2m_dma_handle {
	handle_common header;

	uint64_t dma_rsp_base;
	uint64_t dma_streaming_valve_base;

	// Index of the next available descriptor in the dispatcher queue
	uint64_t next_avail_desc_idx;
	// Total number of unused descriptors in the dispatcher queue
	// Leftover descriptors are reused for subsequent transfers
	// Note: Count includes the next available descriptor in
	// the dispatcher queue indexed by next_avail_desc_idx
	uint64_t unused_desc_count;
} s2m_dma_handle_t;

typedef struct _m2m_dma_handle {
	handle_common header;

	uint64_t cur_ase_page;

	uint64_t dma_ase_cntl_base;
	uint64_t dma_ase_data_base;

	// magic number buffer
	volatile uint64_t *magic_buf;
	uint64_t magic_iova;
	uint64_t magic_wsid;
} m2m_dma_handle_t;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __RC_DMA_TYPES_H__
