/**
 * @file main.cpp
 * @brief Exercise 8, search dup files
 *  */


#include <boost/container_hash/hash.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/directory.hpp>
#include <boost/program_options.hpp>
#include <functional>
#include <ios>
#include <iostream>
#include <fstream>
#include <stdexcept>

namespace po = boost::program_options;
namespace fs = boost::filesystem;

const int generic_errorcode = 102;
const int nodir_errorcode = 103;

class Bayan
{
public:
  using hash_t = size_t;
  using buf_t = std::vector<char>;

  struct F
  {
    std::string path;
    boost::uintmax_t size{};
    std::list<hash_t> hash;
    std::list<std::string> dups;
  };

  Bayan(const boost::uintmax_t minsize, const size_t blocksize)
    : m_minsize(minsize), m_bs(blocksize), m_buf(m_bs)
  {}

  hash_t getHash(std::ifstream &fs, F &f, unsigned ii)
  {
    if(!fs.is_open())
    {
      std::ifstream fin(f.path, std::ios_base::in|std::ios::binary);
      if(!fin.is_open())
      {
        throw std::runtime_error("can not open file for reading");
      }
      fs = std::move(fin);
      if(ii != 0) {
        fs.seekg((std::streamoff)m_bs * ii);
      }
    }
    fs.read(m_buf.data(), (std::streamsize)m_bs);
    if(fs.bad()) {
      throw std::runtime_error("error reading file " + f.path);
    }
    if(m_bs*(ii+1) > f.size)
    {
      for(unsigned kk = f.size % m_bs; kk < m_bs;++kk) {
        m_buf[kk] = 0;
      }
    }
    auto h = boost::hash<buf_t>()(m_buf);
    f.hash.emplace_back(h);
    return h;
  }

  void processFile(fs::directory_entry &f2_de)
  {
    std::cout << "Processing " << f2_de.path() << " ";

    F f2;
    if(!fs::is_regular_file(f2_de))
    {
      std::cout << "dir, skipping\n";
      return;
    }
    f2.size = file_size(f2_de);
    if(f2.size < m_minsize)
    {
      std::cout << "too small, skipping\n";
      return;
    }
    std::cout << " (" << f2.size << "): \n";
    f2.path = f2_de.path().string();
    bool dup_found = false;
    std::ifstream f2s;
    for(auto && f1 : m_files)
    {
      std::cout << "  Compare with " << f1.path << "(" <<  f1.size << "): ";
      if(f1.size == f2.size)
      {
        auto hit1 = f1.hash.begin();
        std::ifstream f1s;

        auto hit2 = f2.hash.begin();
        hash_t h1 = 0;
        hash_t h2 = 0;
        bool hash_mismatch = false;
        for(unsigned ii = 0; ii < ((f1.size-1)/m_bs+1); ++ii)
        {
          if(hit1 == f1.hash.end())
          {
            h1 = getHash(f1s, f1, ii);
          }
          else {
            h1 = *hit1;
            hit1++;
          }
          if(hit2 == f2.hash.end())
          {
            h2 = getHash(f2s, f2, ii);
          }
          else {
            h2 = *hit2;
            hit2++;
          }
          if(h1 != h2)
          {
            std::cout << "  hash mismatch\n";
            hash_mismatch = true;
            break;
          }
        }
        if(!hash_mismatch)
        {
          std::cout << "is a dup of " + f2.path + "!\n";
          dup_found = true;
          f1.dups.emplace_back(f2.path);
          break;
        }
      }
      else {
        std::cout << "  size mismatch\n";
      }
    }
    if(!dup_found)
    {
      std::cout << "dups not found, adding\n";
      m_files.emplace_back(f2);
    }
  }
private:
  const boost::uintmax_t m_minsize;
  const size_t m_bs;
  buf_t m_buf;
  std::vector<F> m_files;
};



  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
  // NOLINTNEXTLINE(hicpp-named-parameter,readability-named-parameter)
int main(int argc, char const * argv[])
{
  try
  {
    std::vector<std::string> toscan;
    std::vector<std::string> toex;
    unsigned recursive{1};
    size_t minsize{1};
    std::string wc;
    size_t bs{512};
    std::string hash;
    po::options_description desc("Allowed options");
    desc.add_options()
      ("help,h"     , "produce help message"     )
      ("dirs,d"     , po::value(&toscan   ), "dir to scan"              )
      ("exclude,x"  , po::value(&toex     ), "dir to exclude"           )
      ("recursive,r", po::value(&recursive), "1: scan subdirs recursively, 0: no recursion" )
      ("minsize,m"  , po::value(&minsize  ), "min file size, must larger than 0 (default 1)")
      ("wildcard,w" , po::value(&wc       ), "wildcard for filenames"   )
      ("blocksize,b", po::value(&bs       ), "blocksize (default 512)"  )
      ("hash,h"     , po::value(&hash     ), "hash to use (default md5)")
      ;


    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    po::notify(vm);

    if((vm.count("help") != 0U) || recursive > 1 || minsize < 1)
    {
      std::cout << "Find duplicate files.\n\n"
                << "Usage: bayan -h\n"
                << "   or: bayan [-d <dir1> [-d <dir2> [...]]]  [-x <direx1> [ -x <direx2> [...]]] [-r <0|1>] [-m <minsize>] [-b <blocksize>] [-h <hash>]\n"
                << "\n\n"
                << desc;
      return 0;
    }

    if(toscan.empty())
    {
      toscan.emplace_back(".");
    }

    Bayan bayan(minsize, bs);

    for(auto && dirname : toscan)
    {
      const fs::path dir(dirname);

      if(!exists(dir))
      {
        std::cerr << dir << " does not exist\n";
        return nodir_errorcode;
      }

      if (!is_directory(dir))
      {
        std::cerr << dir << " exists, but is not a directory\n";
        return (nodir_errorcode);
      }

      std::cout << dir << " is a directory containing:\n";

      if(recursive != 0)
      {
        for (auto && f : fs::recursive_directory_iterator(dir))
        {
          bayan.processFile(f);
        }
      }
      else {
        for (auto && f : fs::directory_iterator(dir))
        {
          bayan.processFile(f);
        }
      }
    }
  }
  catch(const std::exception &e)
  {
    std::cerr << e.what() << std::endl;
    return generic_errorcode;
  }
  return 0;
}
  // NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
