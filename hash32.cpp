#include "hash32.h"
#include <iostream>

namespace hash32
{
  hash_t boosthash(const buf_t &buf)
  {
    return boost::hash<buf_t>()(buf);
  }

  hash_t crc32(const buf_t &buf)
  {
    boost::crc_32_type result;
    result.process_bytes(buf.data(), buf.size());
    return result.checksum();
  }

  using hashdict_t = const std::map<std::string, hash_func_t>;

  hashdict_t& getHashDict()
  {
    static hashdict_t hashdict = {
      {"boosthash", hash32::boosthash},
      {"crc32"    , hash32::crc32    },
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
