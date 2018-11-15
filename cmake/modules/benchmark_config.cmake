## Copyright(c) 2017, Intel Corporation
##
## Redistribution  and  use  in source  and  binary  forms,  with  or  without
## modification, are permitted provided that the following conditions are met:
##
## * Redistributions of  source code  must retain the  above copyright notice,
##   this list of conditions and the following disclaimer.
## * Redistributions in binary form must reproduce the above copyright notice,
##   this list of conditions and the following disclaimer in the documentation
##   and/or other materials provided with the distribution.
## * Neither the name  of Intel Corporation  nor the names of its contributors
##   may be used to  endorse or promote  products derived  from this  software
##   without specific prior written permission.
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
## AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
## IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
## ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
## LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
## CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
## SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
## INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
## CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
## ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
## POSSIBILITY OF SUCH DAMAGE

include(ExternalProject)

# Download and install GoogleBenchmarks
ExternalProject_Add(
  gbench
  GIT_REPOSITORY "https://github.com/google/benchmark.git"
  GIT_TAG "v1.4.1"
  UPDATE_COMMAND ""
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gbench
  CMAKE_ARGS -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DBENCHMARK_ENAGLE_GTEST_TESTS=off
  # Disable install step
  INSTALL_COMMAND "")

set (gbench_root "${CMAKE_CURRENT_BINARY_DIR}/gbench/src/gbench")
message(STATUS "gbenchmark located at: ${gbench_root}")

# Create a benchmark target to be used as a dependency by test programs
add_library(libbenchmark IMPORTED STATIC GLOBAL ALL)
add_library(libbenchmark_main IMPORTED STATIC GLOBAL)

add_dependencies(libbenchmark benchmark)
add_dependencies(libbenchmark_main benchmark_main)

# Get GTest source and binary directories from CMake project
ExternalProject_Get_Property(gbench source_dir binary_dir)

# Set libgtest properties
set_target_properties(libbenchmark PROPERTIES
  "IMPORTED_LOCATION" "${binary_dir}/libbenchmark.a"
  "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}")

# Set libgtest_main properties
set_target_properties(libbenchmark_main PROPERTIES
  "IMPORTED_LOCATION" "${binary_dir}/libbenchmark_main.a"
  "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}")
