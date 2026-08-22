#pragma once 


#include <cstdint>
#include <string>
#include <variant>

namespace minidb {
    
    enum class TypeId { kInvalid, kBoolean, kInteger, kBigInt, kDecimal, kVarchar };

    class Value {
        public:
        using Storage = 
            std::variant<std::monostate, bool, std::int32_t, std::int64_t, double, std::string>;

        Value() = default;

        explicit Value(bool v) : value_(v) {}
        explicit Value(int32_t v) : value_(v) {}
        explicit Value(int64_t v) : value_(v) {}
        explicit Value(double v) : value_(v) {}
        explicit Value(std::string v ) : value_(std::move(v)) {}
        explicit Value(const char* v) : value_(std::string{v}) {}

        [[nodiscard]] TypeId Type() const noexcept;
        [[nodiscard]] bool IsNull() const noexcept { return value_.index() == 0; }

        [[nodiscard]] bool GetBool() const;
        [[nodiscard]] std::int32_t GetInt() const;
        [[nodiscard]] std::int64_t GetBigInt() const;
        [[nodiscard]] double GetDecimal() const;
        [[nodiscard]] const std::string& GetVarChar() const; 

        [[nodiscard]] bool Equals(const Value& other) const;
        [[nodiscard]] bool LessThan(const Value& other) const;

        [[nodiscard]] std::string ToString() const;

        private:
        Storage value_{};
    };
}