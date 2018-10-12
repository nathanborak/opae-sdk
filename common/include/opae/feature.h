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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>

#undef _GNU_SOURCE

#ifndef __FPGA_FEATURE_H__
#define __FPGA_FEATURE_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Object for expressing FPGA feature resource properties
 *
 * A valid `fpga_feature_properties` object, as populated by fpgaFtrGetProperties()
 * `fpga_feature_properties` objects encapsulate all enumerable information about an
 * feature FPGA resources. They can be used for two purposes: selective enumeration
 * (discovery) and querying information about existing resources.
 *
 * For selective enumeration, usually an empty `fpga_feature properties` object is
 * created (using fpgaFtrGetProperties()) and then populated with the desired
 * criteria for enumeration. An array of `fpga_feature_properties` can then be passed
 * to fpgaFeatureEnumerate(), which will return a list of `fpga_feature_token` objects
 * matching these criteria.
 *
 * For querying properties of existing FPGA resources, fpgaGetProperties() can
 * also take an `fpga_feature_token` and will return an `fpga_feature_properties` object
 * populated with information about the resource referenced by that token.
 *
 * To Set and Get values use fpgaFtrPropertiesSetxxxxx and fpgaFtrPropertiesGetxxxxx
 * After use, `fpga_feature_properties` objects should be destroyed using
 * fpga_destroyProperties() to free backing memory used by the
 * `fpga_feature_properties` object.
 */
typedef void *fpga_feature_properties;

/**
 * Token for referencing FPGA feature resources
 *
 * A valid `fpga_feature_token` object, as populated by fpgaFeatureEnumerate()
 * An `fpga_feature_token` serves as a reference to a specific FPGA feature resource present in
 * the system. Holding an `fpga_feature_token` does not constitute ownership of the
 * feature resource - it merely allows the user to query further information about
 * a resource, or to use the feature open function to acquire ownership.
 *
 * Used by feature open function to acquire ownership and yield a handle to the resource.
 *
 * In case of DMA feature, a token represent a "physical" DMA device as represented by a DFH
 */
typedef void *fpga_feature_token;

/**
 * Handle to manipulate feature_token
 *
 */

typedef void *fpga_feature_handle;

/**
 * Sub feature such as DMA engine, HSSI etc.
 *
 */
typedef void *fpga_sub_feature;

/**
 * Handle to an DMA resource
 *
 * A valid `fpga_dma_handle` object, as populated by fpgaDMAOpen(), denotes ownership
 * of an DMA resource. Note that ownership can be exclusive or shared,
 * depending on the flags used in fpgaDMAOpen(). Ownership can be released by
 * calling fpgaDMAClose(), which will render the underlying handle invalid.
 */

typedef void *fpga_dma_handle;

// Callback for asynchronous DMA transfers
typedef void (*fpga_dma_transfer_cb)(void *context);

// DMA descriptors
typedef void *fpga_dma_desc;

// Physically contiguous & pinned DMA buffer
typedef struct fpga_dma_buffer {
	uint64_t *dma_buf_ptr;
	uint64_t dma_buf_wsid;
	uint64_t dma_buf_iova;
	uint64_t dma_buf_len;
} fpga_dma_buffer;

// The `fpga_dma_transfer` objects encapsulate all the information about an transfer
typedef struct fpga_dma_transfer {
	// Transfer information
	fpga_dma_transfer_type_t transfer_type;
	fpga_dma_tx_ctrl_t tx_ctrl;
	fpga_dma_rx_ctrl_t rx_ctrl;

	// Transfer callback and fd (fd used when cb is null)
	fpga_dma_transfer_cb cb;
	int fd;

	// For non-preallocated buffers
	uint64_t src;
	uint64_t dst;
	uint64_t len;

	// For pre-allocated buffers
	uint64_t wsid;

	// Transfer pinned buffers
	fpga_dma_buffer *buffer_pool;

	// Hold transfer state
	bool eop_status;
	size_t rx_bytes;

	// When locked, the transfer in progress
	sem_t tf_status;
} fpga_dma_transfer;

#ifdef __cplusplus
}
#endif

#endif // __FPGA_FEATURE_H__
