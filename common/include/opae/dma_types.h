// Copyright(c) 2017, Intel Corporation
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

#ifndef DMA_TYPES_H
#define DMA_TYPES_H

/**
 * fpga dma transfer typeSemantic version
 *
 * Indicate the required transfer type.
 * For streaming: host to AFU, AFU to host, FPGA local mem to AFU, AFU to FPGA local mem
 * memory to memory: host to fpga, fpga to host, fpga internal  memory copy
 */
typedef enum {
    HOST_MM_TO_FPGA_ST = 0, // sreaming, host to AFU
    FPGA_ST_TO_HOST_MM,     // streaming, AFU to host
    FPGA_MM_TO_FPGA_ST,     // streaming, FPGA local mem to AFU
    FPGA_ST_TO_FPGA_MM,     // streaming, AFU to FPGA local mem
    HOST_TO_FPGA_MM,    // Memory mapped FPGA interface
    FPGA_TO_HOST_MM,    // Memory mapped FPGA interface
    FPGA_TO_FPGA_MM,    // Memory mapped FPGA interface
    FPGA_MAX_TRANSFER_TYPE,
    TERMINATE_THREAD
} fpga_dma_transfer_type_t;

// TX control values
typedef enum {
    TX_NO_PACKET = 0, // deterministic length transfer
    GENERATE_SOP,
    GENERATE_EOP,
    GENERATE_SOP_AND_EOP,
    FPGA_MAX_TX_CTRL
} fpga_dma_tx_ctrl_t;

// RX control values
typedef enum {
    RX_NO_PACKET = 0, // deterministic length transfer
    END_ON_EOP,
    FPGA_MAX_RX_CTRL
} fpga_dma_rx_ctrl_t;

// Channel types
typedef enum {
    INVALID_TYPE = 0,
    TX_ST,
    RX_ST,
    MM,
} fpga_dma_channel_type_t;

// Channel status
typedef enum {
    TRANSFER_IN_PROGRESS = 0,
    TRANSFER_NOT_IN_PROGRESS = 1
} fpga_transf_status_t;

// Handle to complete DMA engine with multiple channels
typedef void *fpga_dma_handle;

// Handle to single DMA channel
typedef void *fpga_dma_channel_handle;

// Callback for asynchronous DMA transfers
typedef void (*fpga_dma_transfer_cb)(void *context);

typedef enum _pool_type {
    POOL_INVALID = 0,
    POOL_SEMA,
    POOL_MUTEX,
    POOL_BUFFERS,
} pool_type;

typedef struct _pool_hdr {
    pool_type type;
    uint32_t destroyed;
} pool_header;

// Pinned buffers
typedef struct _buffer_pool {
    struct _buffer_pool *next;
    pool_header header;
    uint64_t size;
    uint64_t *dma_buf_ptr;
    uint64_t dma_buf_wsid;
    uint64_t dma_buf_iova;
} buffer_pool_item;

// Semaphore / mutex pool
typedef struct _sem_pool {
    struct _sem_pool *next;
    pool_header header;
    sem_t m_semaphore;
} sem_pool_item;

typedef struct _mutex_pool {
    struct _mutex_pool *next;
    pool_header header;
    pthread_mutex_t m_mutex;
} mutex_pool_item;

// Opaque type for DMA descriptors
typedef void *fpga_dma_desc;

// Opaque type for DMA transfer
typedef void *fpga_dma_transfer;

// Opaque type for DMA transfer
typedef struct _fpga_dma_channel_desc {
    uint32_t index;
    fpga_dma_channel_type_t ch_type;
} fpga_dma_channel_desc;

#endif
