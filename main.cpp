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
#include <utility>

namespace po = boost::program_options;
namespace fs = boost::filesystem;

const int generic_errorcode = 102;
const int nodir_errorcode = 103;

class Bayan
{
public:
  using hash_t = size_t;
  using buf_t = std::vector<char>;

    /** file signature */
  struct FSig
  {
    FSig(const std::string&_path, const boost::uintmax_t _size)
      : m_path(_path), m_size(_size)
    {}

    std::string m_path;
    boost::uintmax_t m_size;
    std::vector<hash_t> m_hash;
    std::vector<std::string> m_dups;
  };

  struct FSigExt
  {
    FSigExt(FSig &fsig, size_t bs, buf_t &buf)
      : m_fsig(fsig), m_bs(bs), m_buf(buf)
    {}

    hash_t getHash(unsigned blocki)
    {
      if(blocki < m_fsig.m_hash.size())
      {
        return m_fsig.m_hash[blocki];
      }
      if(!m_ifs.is_open())
      {
        std::ifstream fin(m_fsig.m_path, std::ios_base::in|std::ios::binary);
        if(!fin.is_open())
        {
          throw std::runtime_error("can not open file for reading");
        }
        m_ifs = std::move(fin);
        if(blocki != 0) {
          m_ifs.seekg((std::streamoff)m_bs * blocki);
        }
      }
      m_ifs.read(m_buf.data(), (std::streamsize)m_bs);
      if(m_ifs.bad()) {
        throw std::runtime_error("error reading file " + m_fsig.m_path);
      }
      if(m_bs*(blocki+1) > m_fsig.m_size)
      {
        for(unsigned kk = m_fsig.m_size % m_bs; kk < m_bs;++kk) {
          m_buf[kk] = 0;
        }
      }
      auto h = boost::hash<buf_t>()(m_buf);
      m_fsig.m_hash.emplace_back(h);
      return h;
    }

    std::string path() const { return m_fsig.m_path;}
    boost::uintmax_t size() const { return m_fsig.m_size;}
    FSig fsig() const { return m_fsig;}


    template<typename ...Args>
    void emplace_back_dup(Args && ...args) { m_fsig.m_dups.emplace_back(std::forward<Args>(args)...);}

  private:
    FSig &m_fsig;
    const size_t m_bs;
    buf_t &m_buf;
    std::ifstream m_ifs;
  };

  Bayan(const boost::uintmax_t minsize, const size_t blocksize)
    : m_minsize(minsize), m_bs(blocksize), m_buf(m_bs)
  {}

  void processFile(fs::directory_entry &f2_de)
  {
    std::cout << "Processing " << f2_de.path() << " ";

    if(!fs::is_regular_file(f2_de))
    {
      std::cout << "dir, skipping\n";
      return;
    }
    auto size = file_size(f2_de);
    if(size < m_minsize)
    {
      std::cout << "too small, skipping\n";
      return;
    }
    std::cout << " (" << size << "): \n";
    FSig fs2(f2_de.path().string(), size);
    FSigExt fse2(fs2, m_bs, m_buf);
    bool dup_found = false;
    for(auto && fs1 : m_files)
    {
      FSigExt fse1(fs1, m_bs, m_buf);
      std::cout << "  Compare with " << fse1.path() << "(" <<  fse1.size() << "): ";
      if(fse1.size() == fse2.size())
      {
        bool hash_mismatch = false;
        for(unsigned ii = 0; ii < ((fse1.size()-1)/m_bs+1); ++ii)
        {
          if(fse1.getHash(ii) != fse2.getHash(ii))
          {
            std::cout << "  hash mismatch\n";
            hash_mismatch = true;
            break;
          }
        }
        if(!hash_mismatch)
        {
          std::cout << "is a dup of " + fse2.path() + "!\n";
          dup_found = true;
          fse1.emplace_back_dup(fse2.path());
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
      m_files.emplace_back(fse2.fsig());
    }
  }
private:
  const boost::uintmax_t m_minsize;
  const size_t m_bs;
  buf_t m_buf;
  std::vector<FSig> m_files;
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
