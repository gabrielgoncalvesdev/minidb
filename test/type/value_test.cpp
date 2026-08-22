#include "minidb/type/value.hpp"

#include <cstdint>
#include <string>


#include <gtest/gtest.h>

namespace minidb {
    namespace {

        TEST(ValueTest, NullByDefault) {
            Value v;
            EXPECT_TRUE(v.IsNull());
            EXPECT_EQ(v.Type(), TypeId::kInvalid);
            EXPECT_EQ(v.ToString(), "NULL");
        }

        TEST(ValueTest, ConstructsEachType) {
            EXPECT_EQ(Value(true).Type(), TypeId::kBoolean);
            EXPECT_EQ(Value(std::int32_t{5}).Type(), TypeId::kInteger);
            EXPECT_EQ(Value(std::int64_t{5}).Type(), TypeId::kBigInt);
            EXPECT_EQ(Value(2.5).Type(), TypeId::kDecimal);
            EXPECT_EQ(Value(std::string("Gabriel")).Type(), TypeId::kVarchar);
        }

        TEST(ValueTest, StringLiteralIsVarcharNotBool) {
            Value v("Hello");
            EXPECT_EQ( v.Type(), TypeId::kVarchar);
            EXPECT_EQ(v.GetVarChar(), "Hello");
        }

        TEST(ValueTest, TypedAccessors) {
            EXPECT_EQ(Value(true).GetBool(), true);
            EXPECT_EQ(Value(std::int32_t{5}).GetInt(), 5);
            EXPECT_EQ(Value(std::int64_t{67}).GetBigInt(), 67);
            EXPECT_EQ(Value(6.7).GetDecimal(), 6.7);
        }

        TEST(ValueTest, EqualsSameType) {
            EXPECT_TRUE(Value(std::int32_t{5}).Equals(Value(std::int32_t{5})));
            EXPECT_FALSE(Value(std::int32_t{5}).Equals(Value(std::int32_t{6})));
            EXPECT_TRUE(Value("a").Equals(Value("a")));
            EXPECT_FALSE(Value("a").Equals(Value("b")));
        }

        TEST(ValueTest, LessThan) {
            EXPECT_TRUE(Value(std::int32_t{5}).LessThan(Value(std::int32_t{6})));
            EXPECT_FALSE(Value(std::int32_t{6}).LessThan(Value(std::int32_t{4})));
            EXPECT_TRUE(Value("a").LessThan(Value("b")));
        }

        TEST(ValueTest, NullComparisonAreFalse) {
            EXPECT_FALSE(Value().Equals(Value(std::int32_t{5})));
            EXPECT_FALSE(Value(std::int32_t{5}).Equals(Value()));
            EXPECT_FALSE(Value().LessThan(Value()));
        }

        TEST(ValueTest, ToStringBasics) {
            EXPECT_EQ(Value(std::int32_t{32}).ToString(), "32");
            EXPECT_EQ(Value(true).ToString(), "true");
            EXPECT_EQ(Value(std::string("x")).ToString(), "x");
        }
    }
}