#pragma once

#include "hash.h"
#include <boost/filesystem/directory.hpp>
#include <boost/cstdint.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <regex>

/** @brief find duplicate files using checksums */
class Bayan
{
public:
    /**
     * @brief constructor
     * @param minsize minimal file size
     * @param blocksize size of the chunk for checksum calculations
     * @param rx regular expression to filter file names
     * @param hf ptr to hash function @see hash.h
     * */
  Bayan( boost::uintmax_t minsize, size_t blocksize, std::regex&& rx, hash::hash_func_t &hf);

    /**
     * @brief check file for dups
     * @param f2 path to file for check
     * */
  void processFile(const boost::filesystem::path &f);

    /**
     * @brief print path to files with same content
     * Print duplicated filenames in groups, separated by empty line
     * @param os output stream
     * */
  void printDups(std::ostream &os = std::cout) const;

private:
  using hash_t = hash::hash_t;
  using buf_t = hash::buf_t;

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes,modernize-pass-by-value)

    /**
     * @brief File signature.
     * FSig for each file is stored in Bayan::m_files
     * */
  struct FSig
  {
      /**
       * @brief Constructor initializes only path and size
       * @param path to file
       * @param _size filesize
       * */
    FSig(const std::string&_path, const boost::uintmax_t _size)
      : m_path(_path), m_size(_size)
    {}

    std::string              m_path; ///<! path to file
    boost::uintmax_t         m_size; ///<! size of the file
    std::vector<hash_t>      m_hash; ///<! list of hashes
    std::vector<std::string> m_dups; ///<! list of paths to duplicates
  };

    // NOLINTEND(misc-non-private-member-variables-in-classes,modernize-pass-by-value)

  /**
   * @brief File signature loader
   * */
  struct FSigLoader
  {
      /**
       * @brief Ctor initializes references to FSig storage and main class Bayan
       * @param fsig reference to FSig storage
       * @param bayan reference to class Bayan
       * */
    FSigLoader(FSig &fsig, Bayan &bayan);
      /**
       * @brief get hash of the chunk with number blocki
       * Chunk is first searched in FSig::m_hash, if not found then
       * chunk with offset blocki*Bayan::m_bs is loaded from file
       * and stored in FSig::m_hash
       * @param blocki chunk number
       * @return hash value for the chunk
       **/
    Bayan::hash_t getHash(unsigned blocki);

      /** @brief getter for FSig::m_path */
    std::string path() const { return m_fsig.m_path;}
      /** @brief getter for FSig::m_size */
    boost::uintmax_t size() const { return m_fsig.m_size;}
      /** @brief get FSig copy */
    FSig fsig() const { return m_fsig;}

      /**
       * @brief store path to duplicate in FSig::m_dups
       * */
    template<typename ...Args>
    void emplace_back_dup(Args && ...args) { m_fsig.m_dups.emplace_back(std::forward<Args>(args)...);}

  private:
    FSig &m_fsig; ///<! reference to FSig
    Bayan &m_bayan; ///<! reference to Bayan instance
    std::ifstream m_ifs; ///<! input file stream for that file
  };


  friend struct FSigLoader;
  const boost::uintmax_t m_minsize; ///<! minimal file size (usually == 1)
  const size_t m_bs;         ///<! block size (size of m_buf)
  buf_t m_buf;               ///<! buffer to read single file chunk
  std::vector<FSig> m_files; ///<! list of file signatures
  std::regex m_rx;           ///<! regex to filter filenames
  hash::hash_func_t m_hf;    ///<! pointer to hash function
};
