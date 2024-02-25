/**
 * @file main.cpp
 * @brief Exercise 8, search dup files
 *  */

#include "bayan.h"
#include "hash32.h"
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <ios>
#include <iostream>
#include <stdexcept>
#include <regex>

namespace po = boost::program_options;
namespace fs = boost::filesystem;

const int generic_errorcode = 102;
const int nodir_errorcode = 103;



  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
int main(int argc, char const * argv[])
{
  try
  {
    std::vector<std::string> toscan;
    std::vector<std::string> toex;
    bool recursive{true};
    size_t minsize{1};
    std::string wc{".*"};
    size_t bs{512};
    std::string hashname{"boosthash"};
    po::options_description desc("Allowed options");
    desc.add_options()
      ("help,h"     ,                        "produce help message"                        )
      ("dirs,d"     , po::value(&toscan   ), "dir to scan"                                 )
      ("exclude,x"  , po::value(&toex     ), "dir to exclude"                              )
      ("recursive,r", po::value(&recursive), "1: scan subdirs recursively, 0: no recursion")
      ("minsize,m"  , po::value(&minsize  ), "min file size, must be positive (default 1)" )
      ("wildcard,w" , po::value(&wc       ), "wildcard for filenames"                      )
      ("blocksize,b", po::value(&bs       ), "blocksize, must be positive  (default 512)"  )
      ("hash"       , po::value(&hashname ), "hash to use (default is boosthash)"          )
      ("list-hash"  ,                        "list supported hash algorithms and exit"     )
      ;


    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    po::notify(vm);

    if((vm.count("help") != 0U) || minsize < 1)
    {
      std::cout << "Find duplicate files.\n\n"
                << "Usage: bayan -h\n"
                << "   or: bayan [-d <dir1> [-d <dir2> [...]]]  [-x <direx1> [ -x <direx2> [...]]] [-r <0|1>] [-m <minsize>] [-b <blocksize>] [-h <hash>]\n"
                << "   or: bayan --list-hash\n"
                << "\n\n"
                << desc;
      return 0;
    }

    if(vm.count("list-hash")!= 0U)
    {
      hash32::print_supported();
      return 0;
    }

    if(toscan.empty()) toscan.emplace_back(".");

    auto hashfunc = hash32::hashFuncByName(hashname);
    if(!hashfunc)
    {
      std::cerr << "Unknown hash name '" << hashname << "'\n";
      hash32::print_supported();
      return generic_errorcode;
    }



    Bayan bayan(minsize, bs, std::regex(wc), *hashfunc);

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

      if(recursive)
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
