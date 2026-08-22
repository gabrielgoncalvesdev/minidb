#include "minidb/type/value.hpp"

#include <cassert>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <variant>

namespace minidb {
    TypeId Value::Type() const noexcept {
        switch (value_.index()) {
            case 0: return TypeId::kInvalid;
            case 1: return TypeId::kBoolean;
            case 2: return TypeId::kInteger;
            case 3: return TypeId::kBigInt;
            case 4: return TypeId::kDecimal;
            case 5: return TypeId::kVarchar;
            default: return TypeId::kInvalid;
        }
    }

    bool Value::GetBool() const {
        assert(Type() == TypeId::kBoolean);
        return std::get<bool>(value_);
    }

    std::int32_t Value::GetInt() const {
        assert(Type() == TypeId::kInteger);
        return std::get<std::int32_t>(value_);
    }
    std::int64_t Value::GetBigInt() const  {
        assert(Type() == TypeId::kBigInt);
        return std::get<int64_t>(value_);
    }
    double Value::GetDecimal() const {
        assert(Type() == TypeId::kDecimal);
        return std::get<double>(value_);
    }
    const std::string& Value::GetVarChar() const {
        assert(Type() == TypeId::kVarchar);
        return std::get<std::string>(value_);
    }

    bool Value::Equals(const Value& other) const {
        if (IsNull() || other.IsNull()) {
            return false;
        }
        assert(Type() == other.Type() && "Equal has to have the same type");
        switch (Type()) {
        case TypeId::kBoolean: return GetBool() == other.GetBool();
        case TypeId::kInteger: return GetInt() == other.GetInt();
        case TypeId::kBigInt: return GetBigInt()  == other.GetBigInt();
        case TypeId::kDecimal: return GetDecimal() == other.GetDecimal();
        case TypeId::kVarchar: return GetVarChar() == other.GetVarChar();
        case TypeId::kInvalid: return false;
        }
        return false;
    }

    bool Value::LessThan(const Value& other) const {
        if (IsNull() || other.IsNull()) {
            return false;
        }
        assert(Type() == other.Type() && "Lessthan has to have the same type");
        switch (Type()) {
            case TypeId::kBoolean: return GetBool() < other.GetBool();
            case TypeId::kInteger: return GetInt() < other.GetInt();
            case TypeId::kBigInt: return GetBigInt() < other.GetBigInt();
            case TypeId::kDecimal: return GetDecimal() < other.GetDecimal();
            case TypeId::kVarchar: return GetVarChar() < other.GetVarChar();
            case TypeId::kInvalid: return false;
        }
        return false;
    }

    std::string Value::ToString() const {
        return std::visit(
            [](const auto& v) -> std::string {
                using T = std::decay_t<decltype(v)>;
                if constexpr (std::is_same_v<T, std::monostate>) {
                    return "NULL";
                } else if constexpr (std::is_same_v<T, bool>) {
                return v ? "true" : "false";
                } else if constexpr (std::is_same_v<T, std::string>) {
                    return v;
                } else {
                    return std::to_string(v); 
                }
            },
            value_);
    }
}