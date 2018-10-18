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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>

#include "opae/types.h"
#undef _GNU_SOURCE

#ifndef FEATURE_TYPES_H
#define FEATURE_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 1)
typedef struct {
	uint64_t dfh;
	uint64_t feature_uuid_lo;
	uint64_t feature_uuid_hi;
} dfh_feature_t;
#pragma pack(pop)

typedef union {
	uint64_t reg;
	struct {
		uint64_t feature_type:4;
		uint64_t reserved_8:8;
		uint64_t afu_minor:4;
		uint64_t reserved_7:7;
		uint64_t end_dfh:1;
		uint64_t next_dfh:24;
		uint64_t afu_major:4;
		uint64_t feature_id:12;
	} bits;
} dfh_reg_t;

typedef uint32_t fpga_feature_type;

typedef void *fpga_sub_feature;

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
 */
typedef void *fpga_feature_token;

/**
 * Handle to a feature resource
 *
 * A valid `fpga_feature_handle` object, as populated by fpgaFeatureOpen(), denotes ownership
 * of a resource. Note that ownership can be exclusive or shared,
 * depending on the flags used in fpgaFeatureOpen(). Ownership can be released by
 * calling fpgaFeatureClose(), which will render the underlying handle invalid.
 */
typedef void *fpga_feature_handle;

/**
 * Object for expressing FPGA feature resource properties
 *
 * Used for selective feature enumeration (discovery)
 *
 * For selective enumeration, set the relevant fields with the desired resource information.
 * Pass this structure to fpgaFeatureEnumetate.
 *
 * @note Initialize the fields in this structure to 0xFF to indicate value is not set.
 */
typedef struct {
	fpga_feature_type type;
	fpga_guid guid;
	uint64_t reserved[32];
} fpga_feature_properties;

#ifdef __cplusplus
}
#endif

#endif
