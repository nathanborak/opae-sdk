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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <pthread.h>

#include <opae/types.h>
#include <opae/sysobject.h>
#include <opae/types_enum.h>
#include <opae/dma_types.h>

#include "thpool.h"

#define DMA_BUFFER_POOL_SIZE 8

#ifndef DMA_TYPES_INT_H
#define DMA_TYPES_INT_H

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
typedef struct __attribute__((__packed__)) _fpga_dma_desc {
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
	fpga_dma_transfer_cb cb;
	int fd;
	void* context;

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
struct _fpga_dma_channel {
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
	threadpool thpool;
	pthread_t thread_id;

	// Channel-local pinned buffers
	struct _buffer_pool buffer_pool[DMA_BUFFER_POOL_SIZE];

	// Channel-local queue of transfers
	fpga_dma_transfer_q dma_transfer_queue;

	// Channel-local index of the next available transfer in the dispatcher queue
	uint64_t next_avail_transfer_idx;

	// Channel-local total number of unused transfer in the dispatcher queue of transfers
	// Note: Count includes the next available transfer in
	// the dispatcher queue indexed by next_avail_transfer_idx
	uint64_t unused_transfer_count;
};

#endif
