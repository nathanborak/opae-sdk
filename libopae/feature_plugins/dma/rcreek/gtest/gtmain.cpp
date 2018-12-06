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

#include <unistd.h>
#include <iostream>
#include <getopt.h>
#include <json-c/json.h>
#include <opae/fpga.h>

#include "gtest/gtest.h"
#include "benchmark/benchmark.h"

#include <sys/mman.h>

#define ST_DMA_AFU_ID "EB59BF9D-B211-4A4E-B3E3-753CE68634BA"
#define MM_DMA_AFU_ID "331DB30C-9885-41EA-9081-F88B8F655CAA"
#define TEST_BUF_SIZE (20 * 1024 * 1024)
#define ASE_TEST_BUF_SIZE (4 * 1024)

double poll_wait_count;
double buf_full_count;
char cbuf[2048];

static char *verify_buf = NULL;
static uint64_t verify_buf_size = 0;

static volatile uint32_t async_buf_count = 0;

static inline void *malloc_aligned(uint64_t align, size_t size)
{
	assert(align
	       && ((align & (align - 1)) == 0)); // Must be power of 2 and not 0
	assert(align >= 2 * sizeof(void *));
	void *blk = NULL;

	blk = reinterpret_cast<char*>(mmap(NULL,
					   size + align + 2 * sizeof(void *), // size to be mapped
					   PROT_READ | PROT_WRITE,
					   MAP_SHARED | MAP_ANONYMOUS | MAP_POPULATE, // to a private block of hardware memory
					   0,
					   0));
	void **aptr =
		(void **)(((uint64_t)blk + 2 * sizeof(void *) + (align - 1))
			  & ~(align - 1));
	aptr[-1] = blk;
	aptr[-2] = (void *)(size + align + 2 * sizeof(void *));
	return aptr;
}

static inline void free_aligned(void *ptr)
{
	void **aptr = (void **)ptr;
	munmap(aptr[-1], (size_t)aptr[-2]);

}

static inline void fill_buffer(char *buf, size_t size)
{
	size_t i = 0;

	if (verify_buf_size < size) {
		free(verify_buf);
		verify_buf = (char *)malloc(size);
		verify_buf_size = size;
		buf = verify_buf;

		// use a deterministic seed to generate pseudo-random numbers
		srand(99);

		for (i = 0; i < size; i++) {
			*buf = (char) (rand() % 128);
			buf++;
		}
	}

	memcpy(buf, verify_buf, size);
}

static inline fpga_result verify_buffer(char *buf, size_t size)
{
	assert(NULL != verify_buf);

	if (!memcmp(buf, verify_buf, size)) {
		printf("Buffer Verification Success!\n");
	} else {
		size_t i;
		char rnum = 0;
		srand(99);

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

static inline void clear_buffer(char *buf, size_t size)
{
	memset(buf, 0, size);
}

class dma_benchmark : public ::benchmark::Fixture {
    private:
	fpga_token tok;
	fpga_handle h;

    public:
	void SetUp(const ::benchmark::State& st) {

		int payload_size = st.range(0);
		(void) payload_size;
 	}

	void TearDown(const ::benchmark::State&) {
	}

};

static void CustomArguments(benchmark::internal::Benchmark* b) {
	for (int i = 0; i <= 5; ++i)
		b->Arg(1024 + i * 2 * 1024);
}

int function(int x) {
	return 2*x;
}

BENCHMARK_DEFINE_F(dma_benchmark, Unique)(benchmark::State& state) {
	while (state.KeepRunning()) {
		function(2);
	}
}
BENCHMARK_REGISTER_F(dma_benchmark, Unique)->Apply(CustomArguments)->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();
