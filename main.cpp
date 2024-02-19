/**
 * @file main.cpp
 * @brief Exercise 8, search dup files
 *  */


#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <iostream>


namespace po = boost::program_options;

const int generic_errorcode = 102;


  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
  // NOLINTNEXTLINE(hicpp-named-parameter,readability-named-parameter)
int main(int, char const *[])
{
  try
  {
    std::vector<std::string> toscan;
    po::options_description desc("Allowed options");
    desc.add_options()
      ("help,h"     , "produce help message"     )
      ("dirs,d"     , "dirs to scan"             )
      ("exclude,x"  , "dirs to excllude"         )
      ("recursive,r", "scan subdirs recursively" )
      ("minsize,m"  , "min file size (default 1)")
      ("wildcard,w" , "wildcard for filenames"   )
      ("blocksize,b", "blocksize (default 512)"  )
      ("hash,h"     , "hash to use (default md5)")
      ;

    std::cout << "hello" << std::endl;
  }
  catch(const std::exception &e)
  {
    std::cerr << e.what() << std::endl;
    return generic_errorcode;
  }
  return 0;
}
  // NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
