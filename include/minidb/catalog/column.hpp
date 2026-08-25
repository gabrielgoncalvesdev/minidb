#pragma once 

#include <cstdint>
#include <string>
#include <utility>

#include "minidb/type/value.hpp"

namespace minidb {

    [[nodiscard]] inline constexpr std::uint32_t InlineTypeSize(TypeId type) noexcept {
        switch (type) {
            case TypeId::kBoolean: return 1;
            case TypeId::kInteger: return 4;
            case TypeId::kBigInt:  return 8;
            case TypeId::kDecimal: return 8;
            case TypeId::kVarchar: return 4;  
            case TypeId::kInvalid: return 0;
        }
        return 0;
    }

    class Column {
        public:
        Column(std::string name, TypeId type)
            : name_(std::move(name)), type_(type), fixed_size_(InlineTypeSize(type)) {}

        [[nodiscard]] const std::string& Name() const noexcept { return name_; }
        [[nodiscard]] TypeId Type() const noexcept { return type_; }
        [[nodiscard]] std::uint32_t FixedSize() const noexcept { return fixed_size_; }
        [[nodiscard]] std::uint32_t Offset() const noexcept { return offset_; }
        [[nodiscard]] bool IsInlined() const noexcept { return type_ != TypeId::kVarchar; }

        private:
        friend class Schema; 

        std::string name_;
        TypeId type_;
        std::uint32_t fixed_size_;
        std::uint32_t offset_ = 0;
    };
}