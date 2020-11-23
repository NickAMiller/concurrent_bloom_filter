// Copyright © 2020 Nicholas Miller
// All rights reserved.
//
// This code is licensed under the MIT License.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// Driver to test if we are actually getting the same output as if we were 
// sequentially executing the program.

#include "concurrent_bloom_filter.hpp"
#include <random>
#include <iostream>

// Change this to make the batch size larger or smaller
#define BatchSize 1000000

int main(void) {
    
    // Create a data array that we will insert into the bloom filter
    std::srand(0);
    uint32_t data_array[BatchSize];
    for(int i = 0; i < BatchSize; i ++){
        data_array[i] = rand();
    }

    // Declare the bloom filter with that we will insert into sequentially
    concurrent_bloom seq_filter;
    // Declare the bloom filter with that we will insert into parallel
    concurrent_bloom par_filter;

    // First we are going to insert sequentially then compare against parallel
    omp_set_num_threads(1);
    seq_filter.insert(data_array, BatchSize);

    bool seq_check[BatchSize] = {false};
    seq_filter.contains(data_array, BatchSize, seq_check);

    omp_set_num_threads(4);
    par_filter.insert(data_array, BatchSize);

    bool par_check[BatchSize] = {false};
    par_filter.contains(data_array, BatchSize, par_check);

    // Compare the two checker arrays
    for(int i = 0; i < BatchSize; i++) {
        if(par_check[i] != seq_check[i]) {
            std::cout << "Failed the test" << "\n";
            exit;
        }
    }
    std::cout << "Success" << "\n";
}