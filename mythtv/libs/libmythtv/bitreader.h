/*
 * Copyright (c) 2022 Scott Theisen
 *
 * This file is part of MythTV.
 *
 * MythTV is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * MythTV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with MythTV; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * bitstream reader API header.  The interface is inspired by FFmpeg's internal
 * headers get_bits.h and golomb.h.
 */

#ifndef BIT_READER_H
#define BIT_READER_H

#include <cstdint>
#include <climits> // for CHAR_BIT

#if __has_include(<bit>) // C++20
#include <bit>
#endif

class BitReader
{
  public:
    BitReader(const uint8_t* buffer, uint64_t size) :
        m_buffer(buffer),
        m_buffer_end(buffer + size)
    {
    }
    ~BitReader() = default;

    void skip_bits(unsigned n = 1)
    {
        if (m_cache_size >= n)
        {
            m_cache <<= n;
            m_cache_size -= n;
        }
        else
        {
            m_cache_size = 0;
            m_bit_index += n;
            int quotient = m_bit_index / CHAR_BIT;
            m_bit_index %= CHAR_BIT;
            m_buffer    += quotient;
        }
    }
    uint32_t show_bits(unsigned n)
    {
        //assert(n <= 32);
        if (m_cache_size < n)
        {
            refill_cache(32);
        }
        return uint32_t(get_upper_bits(m_cache, n));
    }
    uint32_t show_bits64(unsigned n)
    {
        //assert(n <= 64);
        if (m_cache_size < n)
        {
            refill_cache(64);
        }
        return get_upper_bits(m_cache, n);
    }
    bool next_bit() { return get_bits(1) == 1; }
    /// @brief Read 0-32 bits.
    uint32_t get_bits(unsigned n = 1)
    {
        uint32_t ret = show_bits(n);
        skip_bits(n);
        return ret;
    }
    /// @brief Read 0-64 bits.
    uint64_t get_bits64(unsigned n)
    {
        uint64_t ret = show_bits64(n);
        skip_bits(n);
        return ret;
    }

    /**
     * Read an unsigned Exp-Golomb code in the range 0 to 8190 (2^13 - 2).
     *
     * @returns the read value or a negative error code.
     */
    int get_ue_golomb()
    {
        return get_ue_golomb(13);
    }

    /**
     * Read an unsigned Exp-Golomb code in the range 0 to UINT32_MAX-1.
     */
    unsigned get_ue_golomb_long()
    {
        unsigned buf    = show_bits(32);
        unsigned lz     = clz(buf);
        unsigned length = lz + 1;
        skip_bits(lz);

        return get_bits(length) - 1;
    }

    /**
     * read unsigned exp golomb code, constraint to a max of 31.
     * If the value encountered is not in 0..31, the return value
     * is outside the range 0..30.
     */
    int get_ue_golomb_31()
    {
        return get_ue_golomb(5);
    }

    /**
     * read signed exp golomb code.
     */
    int get_se_golomb()
    {
        uint32_t buf = get_ue_golomb_long() + 1;
        // revert the -1 offset to get the raw value

        if (buf & 1)
        {
            return -(buf >> 1);
        }
        return buf >> 1;
    }

    int64_t get_bits_left()
    {
        return m_cache_size + CHAR_BIT * int64_t(m_buffer_end - m_buffer) - m_bit_index;
    }

  private:
    int get_ue_golomb(unsigned max_length);
    unsigned clz(uint32_t v)
    {
#if defined(__cpp_lib_bitops) && __cpp_lib_bitops >= 201907L // C++20 <bit>
        return std::countl_zero(v);
#else
        // based on public domain log2_floor
        // http://graphics.stanford.edu/~seander/bithacks.html#IntegerLogDeBruijn
        if (v == 0)
        {
            return 32;
        }
        static constexpr int MultiplyDeBruijnBitPosition[32] = 
        {
             0,  9,  1, 10, 13, 21,  2, 29, 11, 14, 16, 18, 22, 25,  3, 30,
             8, 12, 20, 28, 15, 17, 24,  7, 19, 27, 23,  6, 26,  5,  4, 31
        };

        v |= v >> 1; // first round down to one less than a power of 2 
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;

        return 31 - MultiplyDeBruijnBitPosition[(uint32_t)(v * 0x07C4ACDDU) >> 27];
#endif
    }

    static constexpr uint64_t mask_upper(unsigned bits)
    {
        return ~((UINT64_C(1) << (64 - bits)) - 1);
    }

    static constexpr uint64_t get_upper_bits(uint64_t val, unsigned bits)
    {
        return (val & mask_upper(bits)) >> (64 - bits);
    }

    void refill_cache(unsigned min_bits);

    const uint8_t *m_buffer; ///< next memory location to read from
    const uint8_t * const m_buffer_end; ///< past the end pointer
    unsigned m_bit_index {0}; ///< index for next bit read from the MSB

    /** @brief cache for reads.

    This is updated such that the next bit to read is always the Most Significant
    Bit.

    */
    uint64_t m_cache {0};
    unsigned m_cache_size {0};

};

/**
Valid for max_length < 16.
*/
inline int BitReader::get_ue_golomb(unsigned max_length)
{
    unsigned buf    = show_bits(32);
    unsigned lz     = clz(buf);
    unsigned length = lz + 1;
    unsigned size   = lz + length; // total length

    buf >>= (CHAR_BIT * sizeof(buf)) - size; // remove irrelevant trailing bits

    skip_bits(size);

    if (length > max_length)
    {
        return -1;
    }

    return buf - 1; // offset for storing 0
}

inline void BitReader::refill_cache(unsigned min_bits)
{
    while (m_cache_size < min_bits && m_buffer < m_buffer_end)
    {
        int shift = 64 - m_cache_size;
        int bits = CHAR_BIT - m_bit_index;
        m_cache |= int64_t(*m_buffer & ((1 << bits) - 1)) << (shift - bits);
        if (shift > bits)
        {
            m_bit_index   = 0;
            m_buffer++;
            m_cache_size += bits;
        }
        else
        {
            m_bit_index  += shift;
            m_cache_size += shift;
        }

    }
}

#endif // BIT_READER_H