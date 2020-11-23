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
// Simple bloom filter implementation that allows for batch processing.


#include <vector>
#include <random>
#include <omp.h>

class concurrent_bloom {

    public:


    // The Jenkins 96 bit mix function:
    // http://www.concentric.net/~Ttwang/tech/inthash.htm
    uint32_t HashMix(uint64_t key, uint32_t c) {
        uint32_t a = static_cast<uint32_t>(key)       & 0xFFFFFFFF;
        uint32_t b = static_cast<uint32_t>(key >> 32) & 0xFFFFFFFF;

        a -= (b + c);  a ^= (c >> 13);
        b -= (c + a);  b ^= (a << 8);
        c -= (a + b);  c ^= (b >> 13);
        a -= (b + c);  a ^= (c >> 12);
        b -= (c + a);  b ^= (a << 16);
        c -= (a + b);  c ^= (b >> 5);
        a -= (b + c);  a ^= (c >> 3);
        b -= (c + a);  b ^= (a << 10);
        c -= (a + b);  c ^= (b >> 15);

        return c;
    }


    // Default constructor
    concurrent_bloom() {
        std::srand(0);
        // Create the table of specified size
        table_data = new uint8_t[table_size];
        // Setup the random Keys
        for(int i = 0; i <= num_hashes; i++) {
            hash_keys.push_back(rand());
        }
    }


    //  constructor with parameters
    concurrent_bloom(int seed, int hashes, uint32_t size) {
        std::srand(seed);
        table_size = size;
        num_hashes = hashes;
        // Create the table of specified size
        table_data = new uint8_t[table_size];
        // Setup the random Keys
        for(int i = 0; i <= num_hashes; i++) {
            hash_keys.push_back(rand());
        }
    }


    void insert(uint32_t batch[], int batch_size){
        uint64_t indexes_size = batch_size * num_hashes;
        uint32_t* indexes = new uint32_t[indexes_size];
        // Hash everything seperate
        #pragma omp parallel for schedule(dynamic)
        for(int i = 0; i < batch_size; i++) {
            for(int j = 0; j < num_hashes; j++) {
                indexes[(i * num_hashes) + j] = HashMix(hash_keys[j], batch[i]) 
                    % table_bit_size;
            }
        }
        // Bring it together in serial
        for(int i = 0; i < indexes_size; i++) {
            table_data[indexes[i] / 8] |= 1 << (indexes[i] % 8);
        }
    }


    void contains(uint32_t batch[], int batch_size, bool bool_array[]){
        // Because we are only reading we can parallelize this
        #pragma omp parallel for schedule(dynamic)
        for(int i = 0; i < batch_size; i++) {
            bool flag = 1;
            for(int j = 0; j < num_hashes; j++) {
                uint32_t index = HashMix(hash_keys[j], batch[i]) 
                    % table_bit_size;
                if(!(table_data[index / 8] & (1 << (index % 8)))) {
                    flag = 0;
                    break;
                }
            }
            bool_array[i] = flag;
        }
    }


    uint8_t* table_data;
    std::vector<uint64_t> hash_keys;
    uint32_t num_hashes = 20;
    uint32_t table_size = 128 * 1024 * 1024; // 128 MB
    uint32_t table_bit_size = table_size * 8; // Used for a bitmask
};