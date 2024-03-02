#pragma once

#include <vector>
#include <optional>
#include <string>

namespace hash64
{
  using hash_t = std::size_t;
  using buf_t = std::vector<char>;
  using hash_func_t = hash_t (*)(const buf_t &);

  std::optional<hash_func_t> hashFuncByName(const std::string &name);
  void print_supported();
}
