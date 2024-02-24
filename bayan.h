#pragma once

#include <boost/filesystem/directory.hpp>
#include <boost/cstdint.hpp>
#include <fstream>
#include <vector>
#include <string>
#include <regex>


class Bayan
{
public:
  using hash_t = std::size_t;
  using buf_t = std::vector<char>;

  // NOLINTBEGIN(misc-non-private-member-variables-in-classes)

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
  // NOLINTEND(misc-non-private-member-variables-in-classes)

  /** File signature */
  struct FSigLoader
  {
    FSigLoader(FSig &fsig, size_t bs, buf_t &buf);
    Bayan::hash_t getHash(unsigned blocki);

    std::string path() const { return m_fsig.m_path;}
    boost::uintmax_t size() const { return m_fsig.m_size;}
    FSig fsig() const { return m_fsig;}


    template<typename ...Args>
    void emplace_back_dup(Args && ...args) { m_fsig.m_dups.emplace_back(std::forward<Args>(args)...);}

  private:
    FSig &m_fsig;
    const size_t m_bs;
    Bayan::buf_t &m_buf;
    std::ifstream m_ifs;
  };

  Bayan(const boost::uintmax_t minsize, const size_t blocksize, std::regex && rx)
    : m_minsize(minsize), m_bs(blocksize), m_buf(m_bs), m_rx(rx)
  {}


  void processFile(boost::filesystem::directory_entry &f2_de);

private:
  const boost::uintmax_t m_minsize;
  const size_t m_bs;
  buf_t m_buf;
  std::vector<FSig> m_files;
  std::regex m_rx;
};
