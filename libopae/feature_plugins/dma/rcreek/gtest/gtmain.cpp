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

#include <iostream>
#include <memory>
#include <vector>

#include <unistd.h>
#include <hwloc.h>
#include <uuid/uuid.h>
#include <sys/mman.h>
#include <opae/fpga.h>

#include "gtest/gtest.h"
#include "benchmark/benchmark.h"

#define ST_DMA_AFU_ID "EB59BF9D-B211-4A4E-B3E3-753CE68634BA"
#define MM_DMA_AFU_ID "331DB30C-9885-41EA-9081-F88B8F655CAA"

using namespace std;

// Aligned malloc
static inline void *malloc_aligned(uint64_t size, size_t align)
{
	assert(align
	       && ((align & (align - 1)) == 0)); // Must be power of 2 and not 0
	assert(align >= 2 * sizeof(void *));
	void *blk = NULL;

	blk = mmap(NULL, size + align + 2 * sizeof(void *),
		   PROT_READ | PROT_WRITE,
		   MAP_SHARED | MAP_ANONYMOUS | MAP_POPULATE, 0, 0);
	void **aptr =
		(void **)(((uint64_t)blk + 2 * sizeof(void *) + (align - 1))
			  & ~(align - 1));
	aptr[-1] = blk;
	aptr[-2] = (void *)(size + align + 2 * sizeof(void *));
	return aptr;
}

// Aligned free
static inline void free_aligned(void *ptr)
{
	void **aptr = (void **)ptr;
	munmap(aptr[-1], (size_t)aptr[-2]);

}

class dma_benchmark : public ::benchmark::Fixture {
    protected:
	fpga_properties filter[2];
	fpga_token afc_token;
	fpga_handle afc_h;

	fpga_dma_handle dma_handle;
	fpga_dma_channel_handle dma_ch;

	volatile uint64_t *mmio_ptr;
	uint64_t *dma_tx_buf_ptr;
	uint64_t *dma_rx_buf_ptr;
	uint64_t *dma_buf_ptr;

	size_t pg_size_;
	vector<char> verify_buf;
	uint64_t verify_buf_size;

    protected:
	void fill_buffer(char *buf, size_t size)
	{

		(void) buf;
		if (verify_buf_size < size) {
			verify_buf_size = size;
			verify_buf.resize(size);

			// use a deterministic seed to generate pseudo-random numbers
			::srand(99);
			::generate(verify_buf.begin(), verify_buf.end(), [] () mutable {return (char) (std::rand() % 128);});
		}

		::memcpy(buf, verify_buf.data(), size);
	}

	fpga_result verify_buffer(char *buf, size_t size)
	{
		assert(verify_buf.data() != nullptr);

		if (!::memcmp(buf, verify_buf.data(), size)) {
			printf("Buffer Verification Success!\n");
		} else {
			size_t i;
			char rnum = 0;
			::srand(99);

			for (i = 0; i < size; i++) {
				rnum = (char) (rand() % 128);
				if ((*buf & 0xFF) != rnum) {
					printf("Invalid data at %zx Expected = %d Actual = %x\n",
					       i, rnum, (*buf & 0xFF));
					return FPGA_OK;
				}
				buf++;
			}
		}

		return FPGA_OK;
	}

	void clear_buffer(char *buf, size_t size)
	{
		::memset(buf, 0, size);
	}

    public:

	dma_benchmark() {
		// Global setup code
		fpga_guid guid;
		fpga_guid mm_guid;
		uint32_t num_matches;
		uint32_t count = 0;

		filter[0] = {nullptr};
		filter[1] = {nullptr};

		mmio_ptr = nullptr;
		dma_tx_buf_ptr = nullptr;
		dma_rx_buf_ptr = nullptr;
		dma_buf_ptr = nullptr;

		verify_buf_size = 0;
		pg_size_ = (size_t) sysconf(_SC_PAGE_SIZE);

		// enumerate the afc
		if (uuid_parse(ST_DMA_AFU_ID, guid) < 0) {
			exit(1);
		}
		if (uuid_parse(MM_DMA_AFU_ID, mm_guid) < 0) {
			exit(1);
		}

		assert(fpgaGetProperties(NULL, &filter[0]) == FPGA_OK);
		assert(fpgaGetProperties(NULL, &filter[1]) == FPGA_OK);

		assert(fpgaPropertiesSetObjectType(filter[0], FPGA_ACCELERATOR) == FPGA_OK);
		assert(fpgaPropertiesSetObjectType(filter[1], FPGA_ACCELERATOR) == FPGA_OK);

		assert(fpgaPropertiesSetGUID(filter[0], guid) == FPGA_OK);
		assert(fpgaPropertiesSetGUID(filter[1], mm_guid) == FPGA_OK);
		assert(fpgaEnumerate(&filter[0], 2, &afc_token, 1, &num_matches) == FPGA_OK);
		assert(num_matches >= 1);
		assert(fpgaOpen(afc_token, &afc_h, 0) == FPGA_OK);

		// Enumerate DMA handles
		assert(fpgaDMAOpen(afc_h, &dma_handle) == FPGA_OK);
		assert(fpgaDMAEnumerateChannels(dma_handle, 0, NULL, &count) == FPGA_OK);
		assert(count > 0);
		int channel = 0;
		assert(fpgaDMAOpenChannel(dma_handle, channel, &dma_ch) == FPGA_OK);

		dma_buf_ptr = (uint64_t *) malloc_aligned(64 * 1024 * 1024, pg_size_);
		madvise(dma_buf_ptr, count, MADV_SEQUENTIAL);

 	}

	~dma_benchmark() {

		// Global teardown code
		assert(fpgaDMACloseChannel(&dma_ch) == FPGA_OK);
		assert(fpgaDMAClose(&dma_handle) == FPGA_OK);
		assert(fpgaClose(afc_h) == FPGA_OK);
		assert(fpgaDestroyToken(&afc_token) == FPGA_OK);
		assert(fpgaDestroyProperties(&filter[0]) == FPGA_OK);
		assert(fpgaDestroyProperties(&filter[1]) == FPGA_OK);

		free_aligned(dma_buf_ptr);
	}

	virtual void TearDown(const ::benchmark::State& st) override {
		(void) st;
	}

	virtual void SetUp(const ::benchmark::State& st) override {
		(void) st;
	}

	void dma_host_to_fpga_sync(int count, uint64_t *dma_buf_ptr) {
		fpga_dma_transfer transfer;
		fpgaDMATransferInit(dma_ch, &transfer);

		fpgaDMATransferSetSrc(transfer, (uint64_t) dma_buf_ptr);
		fpgaDMATransferSetDst(transfer, 0);
		fpgaDMATransferSetLen(transfer, count);
		fpgaDMATransferSetTransferType(transfer, HOST_TO_FPGA_MM);
		fpgaDMATransferSetTransferCallback(transfer, NULL, NULL);
		fpgaDMATransferStart(dma_ch, transfer);

		fpgaDMATransferDestroy(dma_ch, &transfer);
	}

};

/* Generate arguments for input buffer size */
static void CustomArguments(benchmark::internal::Benchmark* b) {
	for (int i = 64; i >= 4; i -= 4)
		for (int j = 1; j <= 1; j *= 10)
			b->ArgPair(i, j);
}


BENCHMARK_DEFINE_F(dma_benchmark, mm_host_to_fpga)(benchmark::State& state) {

	// Setup code
	const size_t count = state.range(0) * 1024 * 1024;
	uint64_t *src = dma_buf_ptr;

	assert(src);
	fill_buffer((char *) src, count);

	for (auto _ : state) {
		dma_host_to_fpga_sync(count, src);
	}

	state.SetBytesProcessed(uint64_t(state.iterations()) * uint64_t(count) / 256);

}
BENCHMARK_REGISTER_F(dma_benchmark, mm_host_to_fpga)->Apply(CustomArguments);

BENCHMARK_MAIN();
