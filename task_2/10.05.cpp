//g++ -std=c++23 -Wall -Wextra -Wpedantic -O3 -m32 solution.cpp -o solution.out

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <fstream>
#include <algorithm>
#include <random>
#include <filesystem> // For creating directories

namespace fs = std::filesystem;

// Hash Functions from https://www.partow.net/programming/hashfunctions/#AvailableHashFunctions

// Could have changed them to function_name(const std::string&) 
unsigned int RSHash(const char* str, unsigned int length)
{
   unsigned int b    = 378551;
   unsigned int a    = 63689;
   unsigned int hash = 0;
   unsigned int i    = 0;

   for (i = 0; i < length; ++str, ++i)
   {
      hash = hash * a + (*str);
      a    = a * b;
   }

   return hash;
}

unsigned int JSHash(const char* str, unsigned int length)
{
   unsigned int hash = 1315423911;
   unsigned int i    = 0;

   for (i = 0; i < length; ++str, ++i)
   {
      hash ^= ((hash << 5) + (*str) + (hash >> 2));
   }

   return hash;
}

unsigned int PJWHash(const char* str, unsigned int length)
{
   const unsigned int BitsInUnsignedInt = (unsigned int)(sizeof(unsigned int) * 8);
   const unsigned int ThreeQuarters     = (unsigned int)((BitsInUnsignedInt  * 3) / 4);
   const unsigned int OneEighth         = (unsigned int)(BitsInUnsignedInt / 8);
   const unsigned int HighBits          =
                      (unsigned int)(0xFFFFFFFF) << (BitsInUnsignedInt - OneEighth);
   unsigned int hash = 0;
   unsigned int test = 0;
   unsigned int i    = 0;

   for (i = 0; i < length; ++str, ++i)
   {
      hash = (hash << OneEighth) + (*str);

      if ((test = hash & HighBits) != 0)
      {
         hash = (( hash ^ (test >> ThreeQuarters)) & (~HighBits));
      }
   }

   return hash;
}

unsigned int ELFHash(const char* str, unsigned int length)
{
   unsigned int hash = 0;
   unsigned int x    = 0;
   unsigned int i    = 0;

   for (i = 0; i < length; ++str, ++i)
   {
      hash = (hash << 4) + (*str);

      if ((x = hash & 0xF0000000L) != 0)
      {
         hash ^= (x >> 24);
      }

      hash &= ~x;
   }

   return hash;
}

unsigned int BKDRHash(const char* str, unsigned int length)
{
   unsigned int seed = 131; // Multiplier for hash calculation
   unsigned int hash = 0;
   unsigned int i    = 0;

   for (i = 0; i < length; ++str, ++i)
   {
      hash = (hash * seed) + (*str);
   }

   return hash;
}

unsigned int SDBMHash(const char* str, unsigned int length)
{
   unsigned int hash = 0;
   unsigned int i    = 0;

   for (i = 0; i < length; ++str, ++i)
   {
      hash = (*str) + (hash << 6) + (hash << 16) - hash;
   }

   return hash;
}

unsigned int DJBHash(const char* str, unsigned int length)
{
   unsigned int hash = 5381;
   unsigned int i    = 0;

   for (i = 0; i < length; ++str, ++i)
   {
      hash = ((hash << 5) + hash) + (*str);
   }

   return hash;
}

unsigned int DEKHash(const char* str, unsigned int length)
{
   unsigned int hash = length;
   unsigned int i    = 0;

   for (i = 0; i < length; ++str, ++i)
   {
      hash = ((hash << 5) ^ (hash >> 27)) ^ (*str);
   }

   return hash;
}

unsigned int APHash(const char* str, unsigned int length)
{
   unsigned int hash = 0xAAAAAAAA;
   unsigned int i    = 0;

   for (i = 0; i < length; ++str, ++i)
   {
      hash ^= ((i & 1) == 0) ? ( (hash <<  7) ^ (*str) * (hash >> 3)) :
                                   (~((hash << 11) + ((*str) ^ (hash >> 5))));
   }

   return hash;
}

// Function to generate random strings
std::string generateRandomString(std::size_t length)
{
    const std::string CHARACTERS = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string random_string;
    random_string.reserve(length);
    
    // Use a more robust random number generator
    static std::mt19937 generator(std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<std::size_t> distribution(0, CHARACTERS.length() - 1);

    for (std::size_t i = 0; i < length; ++i)
    {
        random_string += CHARACTERS[distribution(generator)];
    }
    return random_string;
}

// Why does your code have char * 
// For c++ code only use std::string
// char * is c style programming
// I think you could have just used std::string and std::function
// unsigned int (*hashFunc)(const char*, unsigned int) -> std::function<unsigned int>(const std::string&)

// Function to test a hash function and count collisions
std::map<unsigned int, int> testHashFunction(unsigned int (*hashFunc)(const char*, unsigned int), const std::vector<std::string>& strings)
{
    std::map<unsigned int, int> hash_counts;
    for (const auto& str : strings)
    {
        unsigned int hash_val = hashFunc(str.c_str(), str.length());
        hash_counts[hash_val]++;
    }
    return hash_counts;
}

// Function to calculate collisions from hash counts
int calculateCollisions(const std::map<unsigned int, int>& hash_counts)
{
    int collisions = 0;
    // C++ 17 >, you can just do
    // for (const auto& [hash_value, count_coourence]&: hash_counts)
    // second first is not informative
    for (const auto& pair : hash_counts)
    {
        if (pair.second > 1)
        {
            collisions += (pair.second - 1);
        }
    }
    return collisions;
}

// Main test function
void run_all_tests()
{
    std::cout << "Running all tests...\n";

    // Define hash functions to test
    std::vector<std::pair<std::string, unsigned int (*)(const char*, unsigned int)>> hash_functions = {
        {"RSHash", RSHash},
        {"JSHash", JSHash},
        {"PJWHash", PJWHash},
        {"ELFHash", ELFHash},
        {"BKDRHash", BKDRHash},
        {"SDBMHash", SDBMHash},
        {"DJBHash", DJBHash},
        {"DEKHash", DEKHash},
        {"APHash", APHash}
    };

    // Test parameters
    const int MAX_STRINGS_TO_HASH = 100000; // Maximum number of strings to hash
    const int RANDOM_STRING_LENGTH = 10; // Length of random strings
    const int TEST_STEP_SIZE = 10000; // Step size for increasing number of strings
    const std::string RESULTS_DIR = "./results_for_10_05"; // Directory for results

    // Create results directory if it doesn't exist
    if (!fs::exists(RESULTS_DIR)) {
        fs::create_directory(RESULTS_DIR);
    }

    // Open a CSV file to store results for plotting
    std::ofstream results_file(RESULTS_DIR + "/collision_results.csv");
    
    // You are not supposed to repeat code by copy pasting, you use a for loop on your hash_functions, and make this list with code not with hand
    results_file << "NumStrings,RSHash,JSHash,PJWHash,ELFHash,BKDRHash,SDBMHash,DJBHash,DEKHash,APHash\n";

    // Iterate through different numbers of strings
    for (int num_strings = TEST_STEP_SIZE; num_strings <= MAX_STRINGS_TO_HASH; num_strings += TEST_STEP_SIZE)
    {
        std::cout << "Testing with " << num_strings << " strings...\n";
        std::vector<std::string> random_strings;
        random_strings.reserve(num_strings);
        for (int i = 0; i < num_strings; ++i)
        {
            random_strings.push_back(generateRandomString(RANDOM_STRING_LENGTH));
        }

        results_file << num_strings;

        for (const auto& hash_func_pair : hash_functions)
        {
            std::map<unsigned int, int> hash_counts = testHashFunction(hash_func_pair.second, random_strings);
            int collisions = calculateCollisions(hash_counts);
            results_file << "," << collisions;
        }
        results_file << "\n";
    }

    results_file.close();
    std::cout << "Collision results saved to " << RESULTS_DIR << "/collision_results.csv\n"; // collision_results.csv HERE and line 242, you are typing it manually, it should be a variable like RESULTS_DIR
    std::cout << "All tests passed successfully!\n";
}

int main()
{
    run_all_tests();
    return 0;
}

/*
 * The result and conclusion is good!
 * Code can be made better
 * Score is 8/10
 */
