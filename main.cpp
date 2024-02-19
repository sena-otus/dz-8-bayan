/**
 * @file main.cpp
 * @brief Exercise 8, search dup files
 *  */


#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <iostream>




const int generic_errorcode = 102;


  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
  // NOLINTNEXTLINE(hicpp-named-parameter,readability-named-parameter)
int main(int, char const *[])
{
  try
  {
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
