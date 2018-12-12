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

void *malloc_aligned(size_t bytes, size_t alignment)
{
	void *p1 ,*p2; // basic pointer needed for computation.

	/* We need to use malloc provided by C. First we need to allocate memory
	   of size bytes + alignment + sizeof(size_t) . We need 'bytes' because
	   user requested it. We need to add 'alignment' because malloc can give us
	   any address and we need to find multiple of 'alignment', so at maximum multiple
	   of alignment will be 'alignment' bytes away from any location. We need
	   'sizeof(size_t)' for implementing 'aligned_free', since we are returning modified
	   memory pointer, not given by malloc, to the user, we must free the memory
	   allocated by malloc not anything else. So I am storing address given by malloc above
	   pointer returning to user. Thats why I need extra space to store that address.
	   Then I am checking for error returned by malloc, if it returns NULL then
	   aligned_malloc will fail and return NULL. */
	if((p1 =(void *) malloc(bytes + alignment + sizeof(size_t)))==NULL)
		return NULL;

	/* Next step is to find aligned memory address multiples of alignment.
	   By using basic formule I am finding next address after p1 which is
	   multiple of alignment.I am storing new address in p2. */
	size_t addr=(size_t)p1+alignment+sizeof(size_t);

	p2=(void *)(addr - (addr%alignment));

	/* Final step, I am storing the address returned by malloc 'p1' just "size_t"
	   bytes above p2, which will be useful while calling aligned_free. */
	*((size_t *)p2-1)=(size_t)p1;

	return p2;
}

void free_aligned(void *p)
{
	/*	Find the address stored by aligned_malloc ,"size_t" bytes above the
		current pointer then free it using normal free routine provided by C.
	*/
	free((void *)(*((size_t *) p-1)));
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

 	}

	~dma_benchmark() {
		// Global teardown code
		assert(fpgaDMACloseChannel(&dma_ch) == FPGA_OK);
		assert(fpgaDMAClose(&dma_handle) == FPGA_OK);
		assert(fpgaClose(afc_h) == FPGA_OK);
		assert(fpgaDestroyToken(&afc_token) == FPGA_OK);
		assert(fpgaDestroyProperties(&filter[0]) == FPGA_OK);
		assert(fpgaDestroyProperties(&filter[1]) == FPGA_OK);
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
		fpgaDMATransferSetLen(transfer, count * pg_size_);
		fpgaDMATransferSetTransferType(transfer, HOST_TO_FPGA_MM);
		fpgaDMATransferSetTransferCallback(transfer, NULL, NULL);
		// fpgaDMATransferStart(dma_ch, transfer);

		fpgaDMATransferDestroy(dma_ch, &transfer);
	}

};

/* Generate arguments for polling time vs buffer size */
static void CustomArguments(benchmark::internal::Benchmark* b) {
	for (int i = 1; i <= 1024; i *= 2)
		for (int j = 1; j <= 1000; j *= 10)
			b->ArgPair(i, j);
}


BENCHMARK_DEFINE_F(dma_benchmark, bw01)(benchmark::State& state) {

	// Setup code
	uint64_t *dma_buf_ptr = nullptr;
	dma_buf_ptr = (uint64_t *) malloc_aligned(pg_size_*state.range(0), pg_size_);
	assert(dma_buf_ptr);
	fill_buffer((char *) dma_buf_ptr, pg_size_*state.range(0));


	while (state.KeepRunning()) {
		dma_host_to_fpga_sync(state.range(0), dma_buf_ptr);
	}

	free_aligned(dma_buf_ptr);

}
BENCHMARK_REGISTER_F(dma_benchmark, bw01)->Apply(CustomArguments)->Threads(1);

BENCHMARK_MAIN();
