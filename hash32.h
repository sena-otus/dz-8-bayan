#pragma once

#include <boost/crc.hpp>
#include <boost/container_hash/hash.hpp>
#include <vector>
#include <optional>

namespace hash32
{
  using hash_t = std::size_t;
  using buf_t = std::vector<char>;
  using hash_func_t = hash_t (*)(const buf_t &);

  std::optional<hash_func_t> hashFuncByName(const std::string &name);
  void print_supported();
}
