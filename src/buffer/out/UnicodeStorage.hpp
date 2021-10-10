/*++
Copyright (c) Microsoft Corporation
Licensed under the MIT license.

Module Name:
- UnicodeStorage.hpp

Abstract:
- dynamic storage location for glyphs that can't normally fit in the output buffer

Author(s):
- Austin Diviness (AustDi) 02-May-2018
--*/

#pragma once

#include <robin_hood.h>

namespace std
{
    template<>
    struct hash<COORD>
    {
        // We take COORD by value not just because it neatly fits into a register...
        // Reading unaligned pointers doesn't work
        constexpr size_t operator()(COORD coord) const noexcept
        {
            uint64_t x = til::bit_cast<uint32_t>(coord);
            x ^= x >> 33U;
            x *= UINT64_C(0xff51afd7ed558ccd);
            x ^= x >> 33U;
            return gsl::narrow_cast<size_t>(x);
        }
    };
}

class UnicodeStorage final
{
public:
    using key_type = COORD;
    using mapped_type = std::wstring;

    UnicodeStorage() noexcept;

    const mapped_type& GetText(const key_type key) const;
    void StoreGlyph(const key_type key, mapped_type&& glyph);
    void Erase(const key_type key) noexcept;
    void Remap(const std::unordered_map<SHORT, SHORT>& rowMap, const std::optional<SHORT> width);

private:
    robin_hood::unordered_map<key_type, mapped_type> _map;

#ifdef UNIT_TESTING
    friend class UnicodeStorageTests;
    friend class TextBufferTests;
#endif
};
