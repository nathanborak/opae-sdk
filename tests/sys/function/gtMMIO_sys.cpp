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

#include <opae/access.h>
#include <opae/mmio.h>
#include <sys/mman.h>

#include "common_utils.h"
#include "common_sys.h"
#include "gtest/gtest.h"
//#ifdef BUILD_ASE
//#include "ase/api/src/types_int.h"
//#else
//#include "types_int.h"
//#endif

#define CSR_SCRATCHPAD0 0x100
//#define MAX_MMIO_SIZE  1024*256
//#define MMIO_TEST_OFFSET 0x18  //skip first 24 bytes of DFH heder
#define MMIO_OUT_REGION_ADDRESS 1024 * 1024 * 256
#define MAX_MMIO_SIZE 1024 * 256

using namespace common_utils;

class LibopaecMmioFCommonHW : public BaseFixture, public ::testing::Test {
 public:
  uint64_t* mmio_ptr = NULL;
  uint32_t value32 = 0x12345678;
  uint64_t value64 = 0x1122334455667788;
  uint32_t read_value32 = 0;
  uint64_t read_value64 = 0;
  uint32_t offset32 = 0x40;
  uint64_t offset64 = 0x40;

 public:
  void MMIOReadWrite32(fpga_handle h) {
    offset32 = 0x40;
    value32 = 0x12345678;
    // Write value and verify
    while (offset32 < MAX_MMIO_SIZE) {
      read_value32 = 0;
      EXPECT_EQ(FPGA_OK, fpgaWriteMMIO32(h, 0, offset32, value32));
      EXPECT_EQ(FPGA_OK, fpgaReadMMIO32(h, 0, offset32, &read_value32));
      // printf("%llx  %llx \n", offset32, read_value32);
      EXPECT_EQ(read_value32, value32);
      offset32 = offset32 + 4;
      value32 = value32 + 10;
    }
  }

  void MMIOReadWrite64(fpga_handle h) {
    uint64_t value64 = 0x1122334455667788;
    uint64_t offset64 = 0x40;
    // Write value and verify
    while (offset64 < MAX_MMIO_SIZE) {
      read_value64 = 0;
      EXPECT_EQ(FPGA_OK, fpgaWriteMMIO64(h, 0, offset64, value64));
      EXPECT_EQ(FPGA_OK, fpgaReadMMIO64(h, 0, offset64, &read_value64));
      // printf("%llx  %llx \n", offset64, read_value64);
      EXPECT_EQ(read_value64, value64);
      offset64 = offset64 + 8;
      value64 = value64 + 10;
    }
  }
};


/**
 * @test       01
 *
 * @brief      Run a test that performs the functions of fpgadiag/NLB0
 *             moving 1 CL. If the test passes, then MMIO has been
 *             demonstrated successful, as its use is intrinsic to
 *             correct operation of fpgadiag/NLB0.
 */
TEST_F(LibopaecMmioFCommonHW, 01) {
  auto functor = [=]() -> void {

    EXPECT_EQ(0, exerciseNLB0Function(tokens[index]));

  };

  // pass test code to enumerator
  TestAllFPGA(FPGA_ACCELERATOR,  // object type
              true,              // reconfig default NLB0
              functor);          // test code
}

/**
 * @test       02
 *
 * @brief      Allocate a test AFU (or FME or PORT, wherever there is a
 *             test CSR), and for each of the 32-bit and 64-bit
 *             interfaces: set the test input CSR and read the
 *             corresponding test output CSR and validate that the
 *             values are the same. Requires explicit hardware support.
 */
TEST_F(LibopaecMmioFCommonHW, 02) {
  auto functor = [=]() -> void {

    fpga_handle h = NULL;
    fpga_result result = FPGA_OK;

    ASSERT_TRUE(checkReturnCodes(
        result = loadBitstream(config_map[BITSTREAM_MMIO], tokens[index]),
        LINE(__LINE__)));

    if (checkReturnCodes(fpgaOpen(tokens[index], &h, 0), LINE(__LINE__))) {
      printf("Test Read/Write 32 API \n");
      ASSERT_EQ(FPGA_OK, fpgaMapMMIO(h, 0, &mmio_ptr));
      EXPECT_FALSE(mmio_ptr == NULL);

      MMIOReadWrite32(h);
      printf("Test Read/Write 64 API \n");
      MMIOReadWrite64(h);

      EXPECT_EQ(FPGA_OK, fpgaUnmapMMIO(h, 0));
      ASSERT_TRUE(checkReturnCodes(fpgaClose(h), LINE(__LINE__)));
    }
    else {
      std::cout << "open failed\n";
      FAIL();
    }
  };

  // pass test code to enumerator
  TestAllFPGA(FPGA_ACCELERATOR,  // object type
              true,              // reconfig default NLB0
              functor);          // test code
}

/**
 * @test       03
 *
 * @brief      Allocate a test AFU, get the MMIO address and length, and
 *             then attempt to write beyond the length.  Should get a
 *             seg fault; trap with SIGSEGV handler.
 */
TEST_F(LibopaecMmioFCommonHW, 03) {
  auto functor = [=]() -> void {

    fpga_handle h = NULL;
    fpga_result result = FPGA_OK;

    ASSERT_TRUE(checkReturnCodes(
        result = loadBitstream(config_map[BITSTREAM_MMIO], tokens[index]),
        LINE(__LINE__)));

    if (checkReturnCodes(fpgaOpen(tokens[index], &h, 0), LINE(__LINE__))) {
      ASSERT_EQ(FPGA_OK, fpgaMapMMIO(h, 0, &mmio_ptr));
      EXPECT_FALSE(mmio_ptr == NULL);

      // Check errors for misalinged or out of boundary memory accesses
      EXPECT_EQ(FPGA_INVALID_PARAM,
                fpgaWriteMMIO64(h, 0, MAX_MMIO_SIZE + 8, value64));
      EXPECT_EQ(FPGA_INVALID_PARAM,
                fpgaReadMMIO64(h, 0, MAX_MMIO_SIZE + 8, &read_value64));

      EXPECT_EQ(FPGA_OK, fpgaUnmapMMIO(h, 0));
      ASSERT_TRUE(checkReturnCodes(fpgaClose(h), LINE(__LINE__)));
    }

    else {
      std::cout << "open failed\n";
      FAIL();
    }
  };

  TestAllFPGA(FPGA_ACCELERATOR,  // object type
              true,              // reconfig default NLB0
              functor);          // test code
}
