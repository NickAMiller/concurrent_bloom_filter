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
// Driver to test measure the timing of the insert and contains methods

#include "concurrent_bloom_filter.hpp"
#include <random>
#include <iostream>
#include <chrono>

// Change these to customize the tests
#define Steps 4
#define StartingSize 1000

int main(void) {

    uint64_t batch_size = StartingSize;
    std::srand(0);

    // Test various batch sizes
    for(int i = 0; i < Steps; i++) {
        std::cout << "Batch Size: " << batch_size << std::endl;

        uint32_t data_array[batch_size];
        for(int i = 0; i < batch_size; i ++){
            data_array[i] = rand();
        }

        // test up to 8 threads
        for(int j = 1; j <= 8; j *= 2) {
        
            concurrent_bloom filter;
            // Change the number of threads
            omp_set_num_threads(j);

            // Time the insertion
            std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
            filter.insert(data_array, batch_size);
            std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
            std::cout << "Insertion Time " << j << " Threads: " << std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count() << std::endl;

            // Time the contains
            bool bool_array[batch_size] = {false};
            t1 = std::chrono::high_resolution_clock::now();
            filter.contains(data_array, batch_size, bool_array);
            t2 = std::chrono::high_resolution_clock::now();
            std::cout << "Contains Time " << j << " Threads: " << std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count() << std::endl;
        }

        // Make the batch size larger
        batch_size *= 10;
    }
}