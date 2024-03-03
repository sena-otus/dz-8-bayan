#pragma once

#include <vector>
#include <optional>
#include <string>

namespace hash
{
    /** @brief hash length may vary depending on algorithm, so std::vector */
  using hash_t = std::vector<uint8_t>;
  using buf_t = std::vector<char>;
  using hash_func_t = hash_t (*)(const buf_t &);

    /** @brief return ptr to hash function by name */
  std::optional<hash_func_t> hashFuncByName(const std::string &name);
    /** @brief print list of supported hash algo on std::cout */
  void print_supported();
}
