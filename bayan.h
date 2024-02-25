#pragma once

#include "hash32.h"
#include <boost/filesystem/directory.hpp>
#include <boost/cstdint.hpp>
#include <fstream>
#include <vector>
#include <string>
#include <regex>


class Bayan
{
public:
  Bayan( boost::uintmax_t minsize, size_t blocksize, std::regex&& rx, hash32::hash_func_t &hf);

  void processFile(boost::filesystem::directory_entry &f2_de);

private:
  using hash_t = hash32::hash_t;
  using buf_t = hash32::buf_t;

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes,modernize-pass-by-value)

    /** File signature, that part is stored for each file in Bayan::m_files */
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

    // NOLINTEND(misc-non-private-member-variables-in-classes,modernize-pass-by-value)

  /** File signature loader */
  struct FSigLoader
  {
    FSigLoader(FSig &fsig, Bayan &bayan);
    Bayan::hash_t getHash(unsigned blocki);

    std::string path() const { return m_fsig.m_path;}
    boost::uintmax_t size() const { return m_fsig.m_size;}
    FSig fsig() const { return m_fsig;}


    template<typename ...Args>
    void emplace_back_dup(Args && ...args) { m_fsig.m_dups.emplace_back(std::forward<Args>(args)...);}

  private:
    FSig &m_fsig;
    Bayan &m_bayan;
    std::ifstream m_ifs;
  };

  [[nodiscard]] hash_t hash() const;


  friend struct FSigLoader;
  const boost::uintmax_t m_minsize;
  const size_t m_bs;
  buf_t m_buf;
  std::vector<FSig> m_files;
  std::regex m_rx;
  hash32::hash_func_t m_hf;
};
