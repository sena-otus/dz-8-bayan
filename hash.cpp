#include "hash.h"
#include <boost/crc.hpp>
#include <boost/container_hash/hash.hpp>
#include <boost/uuid/detail/md5.hpp>
#include <boost/uuid/detail/sha1.hpp>

#include <iostream>

namespace hash
{
  const uint8_t maskFF = 0xFFU;
  const uint8_t byte_bit_size = 8U;
  const int uint64_bit_size = sizeof(uint64_t) * byte_bit_size;

    /** store unsigned in vector with offset */
  template<typename UINT>
  void uintToHash(UINT uint, hash_t &hash, unsigned offset = 0)
  {
    for(unsigned ii = 0; ii < sizeof(uint); ++ii)
    {
      hash[ii+offset] = uint & maskFF;
      uint = uint >> byte_bit_size;
    }
  }

    /** store unsigned in container */
  template<typename UINT>
  hash_t uintToHash(UINT uint)
  {
    hash_t hash(sizeof(uint));
    uintToHash(uint, hash);
    return hash;
  }

    /** boost hash */
  hash_t boosthash(const buf_t &buf)
  {
    return uintToHash(boost::hash<buf_t>()(buf));
  }

    /** crc64 */
  hash_t crc64(const buf_t &buf)
  {
    constexpr uint64_t polynomial = 0xAD93D23594C935A9;
    boost::crc_optimal<uint64_bit_size, polynomial, uint64_t(-1), uint64_t(0), true, true> crc_64;
    crc_64.process_bytes(buf.data(), buf.size());
    return uintToHash(crc_64.checksum());
  }

    /** md5 */
  hash_t md5sum(const buf_t &buf)
  {
    using boost::uuids::detail::md5;
    md5::digest_type digest;
    const size_t digest_size = sizeof(digest) / sizeof(digest[0]);
    hash_t hash(sizeof(digest));
    md5 md5sum;
    md5sum.process_bytes(buf.data(), buf.size());
    md5sum.get_digest(digest);
    for(unsigned ii = 0; ii < digest_size; ++ii)
    {
      uintToHash(digest[ii], hash, ii*sizeof(digest[0]));
    }
    return hash;
  }

    /** sha1 */
  hash_t sha1sum(const buf_t &buf)
  {
    using boost::uuids::detail::sha1;
    sha1::digest_type digest;
    const size_t digest_size = sizeof(digest) / sizeof(digest[0]);
    hash_t hash(sizeof(digest));
    sha1 sha1sum;
    sha1sum.process_bytes(buf.data(), buf.size());
    sha1sum.get_digest(digest);
    for(unsigned ii = 0; ii < digest_size; ++ii)
    {
      uintToHash(digest[ii], hash, ii*sizeof(digest[0]));
    }
    return hash;
  }


  using hashdict_t = const std::map<std::string, hash_func_t>;

    /** hash dictionary */
  hashdict_t& getHashDict()
  {
    static hashdict_t hashdict = {
      {"boosthash", boosthash},
      {"crc64"    , crc64    },
      {"md5"      , md5sum   },
      {"sha1"     , sha1sum   },
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
    std::string sep;
    for(auto && h : getHashDict())
    {
      std::cout << sep << h.first;
      sep = " ";
    }
    std::cout << "\n";
  }

}
