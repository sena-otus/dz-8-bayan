#include "hash64.h"
#include <boost/crc.hpp>
#include <boost/container_hash/hash.hpp>

#include <iostream>

namespace hash64
{
  const int hash_b_size = 64;

  hash_t boosthash(const buf_t &buf)
  {
    return boost::hash<buf_t>()(buf);
  }

  hash_t crc64(const buf_t &buf)
  {
    constexpr uint64_t polynomial = 0xAD93D23594C935A9;
    boost::crc_optimal<hash_b_size, polynomial, uint64_t(-1), uint64_t(0), true, true> crc_64;
    crc_64.process_bytes(buf.data(), buf.size());
    return crc_64.checksum();
  }


  using hashdict_t = const std::map<std::string, hash_func_t>;

  hashdict_t& getHashDict()
  {
    static hashdict_t hashdict = {
      {"boosthash", hash64::boosthash},
      {"crc64"    , hash64::crc64    },
    };
    return hashdict;
  }

  std::optional<hash_func_t> hashFuncByName(const std::string &name)
  {
    auto it = getHashDict().find(name);
    if(it == getHashDict().end())
    {
      return {};
    }
    return it->second;
  }

  void print_supported()
  {
    std::cout << "List of supported hash algorithms:\n";
    for(auto && h : getHashDict())
    {
      std::cout << h.first << "\n";
    }
  }

}
