/**
 * @file gtest_bayan.cpp
 *
 * @brief Test bayan
 *  */

#include "bayan.h"
#include "hash.h"
#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <sstream>

namespace {

   // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

  TEST(bayan, testfiles) {
    {
      namespace fs = std::filesystem;
        // create 5 files, 3 with same content and 2 unique

      std::error_code ec;
      fs::remove_all("testdir1", ec);
      fs::create_directory("testdir1");
      {
        std::ofstream ofs("testdir1/f1");
        ofs << "test content1" << "\n";
      }
      fs::copy("testdir1/f1", "testdir1/f2");
      fs::copy("testdir1/f1", "testdir1/f3");
      {
        std::ofstream ofs("testdir1/f4");
        ofs << "test content4" << "\n";
      }
      {
        std::ofstream ofs("testdir1/f5");
        ofs << "test content5" << "\n";
      }

      auto hf = hash::hashFuncByName("md5");
      EXPECT_TRUE(hf);
      if(!hf) return;
      Bayan bayan(1, 512, std::regex(".*", std::regex_constants::icase), *hf);
      bayan.processFile("testdir1/f1");
      bayan.processFile("testdir1/f2");
      bayan.processFile("testdir1/f3");
      bayan.processFile("testdir1/f4");
      bayan.processFile("testdir1/f5");

      std::ostringstream ostr;
      bayan.printDups(ostr);
      std::string expect_str = "testdir1/f1\n""testdir1/f2\n""testdir1/f3\n";
      EXPECT_EQ(ostr.str(), expect_str);
      fs::remove_all("testdir1");
    }
  }

    // NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
}
