#include "minidb/storage/table/tuple.hpp"
#include "minidb/type/value.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>

namespace minidb {
    namespace {

        template <typename T>
        void WriteFixed(std::byte* dest, const T& v ) {
            std::memcpy(dest, &v, sizeof(T));
        }

        template <typename T>
        [[nodiscard]] T ReadFixed(const std::byte* src) {
            T v;
            std::memcpy(&v, src, sizeof(T));
            return v;
        }
    }

    Tuple::Tuple(std::span<const std::byte> data, RID rid)
        : rid_(rid), data_(data.begin(), data.end()) {};

    Tuple::Tuple(const std::vector<Value>& values, const Schema& schema) {
        assert(values.size() == schema.GetColumnCount());

        std::size_t total = schema.InlinedSize();
        for (std::uint32_t idx : schema.UninlinedColumns()) {
            assert(!values[idx].IsNull() && "Null tuple Not yet");
            total += sizeof(std::uint32_t) + values[idx].GetVarChar().size();
        }
        data_.assign(total, std::byte(0));

        std::uint32_t var_cursor = schema.InlinedSize();
        for (std::uint32_t i = 0; i < schema.GetColumnCount(); ++i) {
            const Column& col = schema.GetColumn(i);
            std::byte* slot = data_.data() + col.Offset();
            switch(col.Type()) {
                case TypeId::kBoolean: {
                    std::uint8_t b = 0;
                    if (values[i].GetBool()) {
                        b = 1;
                    }
                    WriteFixed(slot, b);
                    break;
                }
                case TypeId::kInteger: WriteFixed(slot, values[i].GetInt()); break;
                case TypeId::kBigInt: WriteFixed(slot, values[i].GetBigInt()); break;
                case TypeId::kDecimal: WriteFixed(slot, values[i].GetDecimal()); break;
                case TypeId::kVarchar: {
                    const std::string& s = values[i].GetVarChar();
                    const auto len = static_cast<std::uint32_t>(s.size());
                    WriteFixed<std::uint32_t>(slot, var_cursor);
                    WriteFixed<std::uint32_t>(data_.data() + var_cursor, len);
                    std::memcpy(data_.data() + var_cursor + sizeof(std::uint32_t), s.data(), s.size());
                    var_cursor += static_cast<std::uint32_t>(sizeof(std::uint32_t) + s.size());
                    break;
                }
                case TypeId::kInvalid: break;
            }
        }
    }

    Value Tuple::GetValue(const Schema& schema, std::uint32_t col_idx ) const {
        const Column& col = schema.GetColumn(col_idx);
        const std::byte* slot = data_.data() + col.Offset();
        switch (col.Type()) {
            case TypeId::kBoolean: return Value(ReadFixed<std::uint8_t>(slot) != 0);
            case TypeId::kInteger: return Value(ReadFixed<std::int32_t>(slot));
            case TypeId::kBigInt: return Value(ReadFixed<std::int64_t>(slot));
            case TypeId::kDecimal: return Value(ReadFixed<double>(slot));
            case TypeId::kVarchar: {
                const std::uint32_t off = ReadFixed<std::uint32_t>(slot);
                const std::uint32_t len = ReadFixed<std::uint32_t>(data_.data() + off);
                const char* chars = 
                    reinterpret_cast<const char*>(data_.data() + off + sizeof(std::uint32_t));
                    return Value(std::string(chars, len));
            }
            case TypeId::kInvalid: return Value();
        }
        return Value();
    }
}