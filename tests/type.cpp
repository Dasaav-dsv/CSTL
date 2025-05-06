#include <gtest/gtest.h>

#include <cstdint>

#include "type.h"

TEST(TypeTest, PrimitiveTypes) {
    CSTL_Type type;

    EXPECT_EQ(CSTL_TypeErr_Ok, CSTL_define_type(&type, sizeof(char), alignof(char), nullptr, nullptr, nullptr))
        << "should be trivial";
    EXPECT_EQ(CSTL_TypeErr_Ok, CSTL_define_type(&type, sizeof(short), alignof(short), nullptr, nullptr, nullptr))
        << "should be trivial";
    EXPECT_EQ(CSTL_TypeErr_Ok, CSTL_define_type(&type, sizeof(int), alignof(int), nullptr, nullptr, nullptr))
        << "should be trivial";
    EXPECT_EQ(CSTL_TypeErr_Ok, CSTL_define_type(&type, sizeof(long), alignof(long), nullptr, nullptr, nullptr))
        << "should be trivial";
    EXPECT_EQ(CSTL_TypeErr_Ok, CSTL_define_type(&type, sizeof(long long), alignof(long long), nullptr, nullptr, nullptr))
        << "should be trivial";
    EXPECT_EQ(CSTL_TypeErr_Ok, CSTL_define_type(&type, sizeof(float), alignof(float), nullptr, nullptr, nullptr))
        << "should be trivial";
    EXPECT_EQ(CSTL_TypeErr_Ok, CSTL_define_type(&type, sizeof(double), alignof(double), nullptr, nullptr, nullptr))
        << "should be trivial";
    EXPECT_EQ(CSTL_TypeErr_Ok, CSTL_define_type(&type, sizeof(long double), alignof(long double), nullptr, nullptr, nullptr))
        << "should be trivial";
    EXPECT_EQ(CSTL_TypeErr_Ok, CSTL_define_type(&type, sizeof(void*), alignof(void*), nullptr, nullptr, nullptr))
        << "should be trivial";
}

TEST(TypeTest, SizeError) {
    CSTL_Type type;

    EXPECT_EQ(CSTL_TypeErr_BadSize, CSTL_define_type(&type, 0, 1, nullptr, nullptr, nullptr))
        << "should not be able to define a type with size 0";

    EXPECT_EQ(CSTL_TypeErr_BadSize, CSTL_define_type(&type, 1, 2, nullptr, nullptr, nullptr))
        << "size should be a multiple of alignment";

    EXPECT_EQ(CSTL_TypeErr_BadSize, CSTL_define_type(&type, 3, 2, nullptr, nullptr, nullptr))
        << "size should be a multiple of alignment";

    EXPECT_EQ(CSTL_TypeErr_BadSize, CSTL_define_type(&type, 3, 4, nullptr, nullptr, nullptr))
        << "size should be a multiple of alignment";

    EXPECT_EQ(CSTL_TypeErr_BadSize, CSTL_define_type(&type, SIZE_MAX / 2 + 1, 1, nullptr, nullptr, nullptr))
        << "size should not be able to exceed SIZE_MAX / 2 + 1";
}

TEST(TypeTest, AlignmentError) {
    CSTL_Type type;

    EXPECT_EQ(CSTL_TypeErr_Misaligned, CSTL_define_type(&type, 1, 0, nullptr, nullptr, nullptr))
        << "an alignment of 0 should not be legal";

    EXPECT_EQ(CSTL_TypeErr_Misaligned, CSTL_define_type(&type, 6, 3, nullptr, nullptr, nullptr))
        << "alignment should be a power of 2";

    EXPECT_EQ(CSTL_TypeErr_Misaligned, CSTL_define_type(&type, 3, 6, nullptr, nullptr, nullptr))
        << "alignment should be a power of 2; size should be a multiple of alignment";
}

TEST(TypeTest, NullError) {
    CSTL_Type type;

    EXPECT_EQ(CSTL_TypeErr_Ok, CSTL_define_type(&type, 1, 1, nullptr, nullptr, nullptr))
        << "should be trivial";

    EXPECT_EQ(CSTL_TypeErr_NullPointer, CSTL_define_type(nullptr, 1, 1, nullptr, nullptr, nullptr))
        << "null pointer should be handled";
}

TEST(TypeTest, MoveAndCopy) {
    CSTL_Type type;

    ASSERT_EQ(CSTL_TypeErr_Ok, CSTL_define_type(&type, 1, 1, nullptr, nullptr, nullptr))
        << "should be trivial";
    EXPECT_EQ(type.use_move_from, 0) << "move constructor not set";

    ASSERT_EQ(CSTL_TypeErr_Ok, CSTL_define_type(&type, 1, 1, [](void*, const void*){}, nullptr, nullptr))
        << "should be trivial";
    EXPECT_EQ(type.use_move_from, 0) << "move constructor not set";

    ASSERT_EQ(CSTL_TypeErr_Ok, CSTL_define_type(&type, 1, 1, nullptr, [](void*, void*){}, nullptr))
        << "should be trivial";
    EXPECT_EQ(type.use_move_from, 1) << "move constructor set";

    ASSERT_EQ(CSTL_TypeErr_Ok, CSTL_define_type(&type, 1, 1, [](void*, const void*){}, [](void*, void*){}, nullptr))
        << "should be trivial";
    EXPECT_EQ(type.use_move_from, 1) << "move constructor set";
}
