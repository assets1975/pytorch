#pragma once

#include <c10/macros/Macros.h>
#include <c10/util/C++17.h>
#include <c10/util/Optional.h>
#include <iostream>
#if defined(_MSC_VER)
#include <intrin.h>
#else
#include <strings.h>
#endif

namespace c10 {
namespace utils {

/**
 * This is a simple bitset class with sizeof(long long int) bits.
 * You can set bits, unset bits, query bits by index,
 * and query for the first set bit.
 * Before using this class, please also take a look at std::bitset,
 * which has more functionality and is more generic. It is probably
 * a better fit for your use case. The sole reason for c10::utils::bitset
 * to exist is that std::bitset misses a find_first_set() method.
 */
struct bitset final {
private:
  #if defined(_MSC_VER)
    // MSVCs _BitScanForward64 expects int64_t
    using bitset_type = int64_t;
  #else
    // POSIX ffsll expects long long int
    using bitset_type = long long int;
  #endif
public:
  static constexpr size_t NUM_BITS() {
    return 8 * sizeof(bitset_type);
  }

  constexpr bitset() noexcept : bitset_(0) {}
  constexpr bitset(const bitset&) noexcept = default;
  constexpr bitset(bitset&&) noexcept = default;
  constexpr bitset& operator=(const bitset&) noexcept = default;
  constexpr bitset& operator=(bitset&&) noexcept = default;

  constexpr void set(size_t index) noexcept {
    bitset_ |= (static_cast<long long int>(1) << index);
  }

  constexpr void unset(size_t index) noexcept {
    bitset_ &= ~(static_cast<long long int>(1) << index);
  }

  constexpr bool get(size_t index) const noexcept {
    return bitset_ & (static_cast<long long int>(1) << index);
  }

  // Call the given functor with the index of each bit that is set
  template <class Func>
  void for_each_set_bit(Func&& func) const {
    bitset cur = *this;
    size_t index = cur.find_first_set();
    while (0 != index) {
      // -1 because find_first_set() is not one-indiced.
      index -= 1;
      func(index);
      cur.unset(index);
      index = cur.find_first_set();
    }
  }

private:
  // Return the index of the first set bit. The returned index is one-indiced
  // (i.e. if the very first bit is set, this function returns '1'), and a return
  // of '0' means that there was no bit set.
  size_t find_first_set() const {
    #if defined(_MSC_VER)
      unsigned long result;
      bool has_bits_set = (0 == _BitScanForward64(&result, bitset_));
      if (!has_bits_set) {
        return 0;
      }
      return result + 1;
    #else
      return ffsll(bitset_);
    #endif
  }
  
  bitset_type bitset_;
};

} // namespace utils
} // namespace c10