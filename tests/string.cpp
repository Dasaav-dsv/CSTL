#include <gtest/gtest.h>

#include <string>

#include "alloc.h"
#include "xstring.h"

class StringTest : public testing::Test {
protected:
    StringTest() : alloc{nullptr} {
        sample = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

        CSTL_string_construct(&cstl_str);
    }

    ~StringTest() {
        CSTL_string_destroy(&cstl_str, alloc);
    }

    void string_expect_equal() {
        std::string cstl_copy {
            CSTL_string_c_str(&cstl_str),
            CSTL_string_size(&cstl_str)
        };

        EXPECT_EQ(real_str, cstl_copy)
            << "strings should compare equal";
    }

    // Genuine STL string:
    std::string real_str;
    const char* sample;

    // CSTL string:
    CSTL_StringVal cstl_str;
    CSTL_Alloc* alloc;
};

TEST_F(StringTest, Default) {
    string_expect_equal();

    EXPECT_TRUE(CSTL_string_empty(&cstl_str))
        << "default constructed string must be empty";

    EXPECT_EQ(0, CSTL_string_size(&cstl_str))
        << "default constructed string should have a size of 0";

    EXPECT_EQ(CSTL_string_begin(&cstl_str), CSTL_string_end(&cstl_str))
        << "`first` should equal `last` when empty";

    EXPECT_STREQ(CSTL_string_c_str(&cstl_str), "")
        << "`c_str` must return a valid null-terminated empty string";
}

TEST_F(StringTest, AssignEq) {
    // "012"
    real_str.assign(sample, 3);
    CSTL_string_assign_n(&cstl_str, sample, 3, alloc);
    string_expect_equal();

    // "0123456"
    real_str.assign(sample, 7);
    CSTL_string_assign_n(&cstl_str, sample, 7, alloc);
    string_expect_equal();

    // "0123456789ABCDE"
    real_str.assign(sample, 15);
    CSTL_string_assign_n(&cstl_str, sample, 15, alloc);
    string_expect_equal();

    // "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    real_str.assign(sample, 36);
    CSTL_string_assign_n(&cstl_str, sample, 36, alloc);
    string_expect_equal();

    // Full sample string:
    real_str.assign(sample);
    CSTL_string_assign(&cstl_str, sample, alloc);
    string_expect_equal();

    // "0123456789ABCDE"
    real_str.assign(sample, 15);
    CSTL_string_assign_n(&cstl_str, sample, 15, alloc);
    string_expect_equal();

    // "0123456"
    real_str.assign(sample, 7);
    CSTL_string_assign_n(&cstl_str, sample, 7, alloc);
    string_expect_equal();

    // "012"
    real_str.assign(sample, 3);
    CSTL_string_assign_n(&cstl_str, sample, 3, alloc);
    string_expect_equal();
}

TEST_F(StringTest, Substring) {
    // "012"
    real_str.assign(sample, 3);
    CSTL_string_assign_n(&cstl_str, sample, 3, alloc);
    string_expect_equal();

    // "12"
    real_str = real_str.substr(1, 2);
    CSTL_string_substr(&cstl_str, &cstl_str, 1, 2, alloc);
    string_expect_equal();

    // "1"
    real_str = real_str.substr(0, 1);
    CSTL_string_substr(&cstl_str, &cstl_str, 0, 1, alloc);
    string_expect_equal();

    // "0123456"
    real_str.assign(sample, 7);
    CSTL_string_assign_n(&cstl_str, sample, 7, alloc);
    string_expect_equal();

    // "23456"
    real_str = real_str.substr(2, 5);
    CSTL_string_substr(&cstl_str, &cstl_str, 2, 5, alloc);
    string_expect_equal();
    
    // "234"
    real_str = real_str.substr(0, 3);
    CSTL_string_substr(&cstl_str, &cstl_str, 0, 3, alloc);
    string_expect_equal();

    // Full sample string:
    real_str.assign(sample);
    CSTL_string_assign(&cstl_str, sample, alloc);
    string_expect_equal();
    
    // "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    real_str = real_str.substr(10, 26);
    CSTL_string_substr(&cstl_str, &cstl_str, 10, 26, alloc);
    string_expect_equal();
    
    // "XYZ"
    real_str = real_str.substr(23, 3);
    CSTL_string_substr(&cstl_str, &cstl_str, 23, 3, alloc);
    string_expect_equal();
}

TEST_F(StringTest, CopyAndMove) {
    CSTL_StringVal other;
    CSTL_string_construct(&other);

    // "012"
    real_str.assign(sample, 3);
    CSTL_string_assign_n(&cstl_str, sample, 3, alloc);
    CSTL_string_copy_assign(&other, alloc, &cstl_str, alloc, true);
    CSTL_string_move_assign(&cstl_str, alloc, &other, alloc, true);
    string_expect_equal();

    // "0123456"
    real_str.assign(sample, 7);
    CSTL_string_assign_n(&cstl_str, sample, 7, alloc);
    CSTL_string_copy_assign(&other, alloc, &cstl_str, alloc, true);
    CSTL_string_move_assign(&cstl_str, alloc, &other, alloc, true);
    string_expect_equal();

    // "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    real_str.assign(sample, 36);
    CSTL_string_assign_n(&cstl_str, sample, 36, alloc);
    CSTL_string_copy_assign(&other, alloc, &cstl_str, alloc, true);
    CSTL_string_move_assign(&cstl_str, alloc, &other, alloc, true);
    string_expect_equal();

    // Full sample string:
    real_str.assign(sample);
    CSTL_string_assign(&cstl_str, sample, alloc);
    CSTL_string_copy_assign(&other, alloc, &cstl_str, alloc, true);
    CSTL_string_move_assign(&cstl_str, alloc, &other, alloc, true);
    string_expect_equal();

    // "0123456789ABCDE"
    real_str.assign(sample, 15);
    CSTL_string_assign_n(&cstl_str, sample, 15, alloc);
    CSTL_string_copy_assign(&other, alloc, &cstl_str, alloc, true);
    CSTL_string_move_assign(&cstl_str, alloc, &other, alloc, true);
    string_expect_equal();

    // "0123456"
    real_str.assign(sample, 7);
    CSTL_string_assign_n(&cstl_str, sample, 7, alloc);
    CSTL_string_copy_assign(&other, alloc, &cstl_str, alloc, true);
    CSTL_string_move_assign(&cstl_str, alloc, &other, alloc, true);
    string_expect_equal();

    // "012"
    real_str.assign(sample, 3);
    CSTL_string_assign_n(&cstl_str, sample, 3, alloc);
    CSTL_string_copy_assign(&other, alloc, &cstl_str, alloc, true);
    CSTL_string_move_assign(&cstl_str, alloc, &other, alloc, true);
    string_expect_equal();

    CSTL_string_destroy(&other, alloc);
}

TEST_F(StringTest, IndexAt) {
    // "012"
    real_str.assign(sample, 3);
    CSTL_string_assign_n(&cstl_str, sample, 3, alloc);
    EXPECT_EQ('2', *CSTL_string_at(&cstl_str, 2));
    EXPECT_EQ(nullptr, CSTL_string_at(&cstl_str, 3));

    // "0123456"
    real_str.assign(sample, 7);
    CSTL_string_assign_n(&cstl_str, sample, 7, alloc);
    EXPECT_EQ('6', *CSTL_string_at(&cstl_str, 6));
    EXPECT_EQ(nullptr, CSTL_string_at(&cstl_str, 7));

    // "0123456789ABCDE"
    real_str.assign(sample, 15);
    CSTL_string_assign_n(&cstl_str, sample, 15, alloc);
    EXPECT_EQ('E', *CSTL_string_at(&cstl_str, 14));
    EXPECT_EQ(nullptr, CSTL_string_at(&cstl_str, 15));

    // "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    real_str.assign(sample, 36);
    CSTL_string_assign_n(&cstl_str, sample, 36, alloc);
    EXPECT_EQ('Z', *CSTL_string_at(&cstl_str, 35));
    EXPECT_EQ(nullptr, CSTL_string_at(&cstl_str, 36));

    // Full sample string:
    real_str.assign(sample);
    CSTL_string_assign(&cstl_str, sample, alloc);
    EXPECT_EQ('z', *CSTL_string_at(&cstl_str, 61));
    EXPECT_EQ(nullptr, CSTL_string_at(&cstl_str, 62));

    // "0123456789ABCDE"
    real_str.assign(sample, 15);
    CSTL_string_assign_n(&cstl_str, sample, 15, alloc);
    EXPECT_EQ('E', *CSTL_string_at(&cstl_str, 14));
    EXPECT_EQ(nullptr, CSTL_string_at(&cstl_str, 15));

    // "0123456"
    real_str.assign(sample, 7);
    CSTL_string_assign_n(&cstl_str, sample, 7, alloc);
    EXPECT_EQ('6', *CSTL_string_at(&cstl_str, 6));
    EXPECT_EQ(nullptr, CSTL_string_at(&cstl_str, 7));

    // "012"
    real_str.assign(sample, 3);
    CSTL_string_assign_n(&cstl_str, sample, 3, alloc);
    EXPECT_EQ('2', *CSTL_string_at(&cstl_str, 2));
    EXPECT_EQ(nullptr, CSTL_string_at(&cstl_str, 3));
}

TEST_F(StringTest, FrontAndBack) {
    CSTL_string_push_back(&cstl_str, '0', alloc);
    EXPECT_EQ(CSTL_string_back(&cstl_str), CSTL_string_front(&cstl_str))
        << "`front` and `back` must point to the same element";

    CSTL_string_push_back(&cstl_str, '1', alloc);
    EXPECT_NE(CSTL_string_back(&cstl_str), CSTL_string_front(&cstl_str))
        << "`front` and `back` must not point to the same element";

    CSTL_string_pop_back(&cstl_str);
    EXPECT_EQ(CSTL_string_back(&cstl_str), CSTL_string_front(&cstl_str))
        << "`front` and `back` must point to the same element";
}

TEST_F(StringTest, Capacity) {
    ASSERT_FALSE(CSTL_string_reserve(&cstl_str, CSTL_string_max_size() + 1, alloc))
        << "must fail and return false";

    ASSERT_TRUE(CSTL_string_reserve(&cstl_str, 3, alloc))
        << "must succeed (does nothing)";

    char* data = CSTL_string_data(&cstl_str);

    CSTL_string_assign_n(&cstl_str, sample, 3, alloc);
    EXPECT_EQ(data, CSTL_string_data(&cstl_str))
        << "should not be invalidated with sufficient capacity";

    ASSERT_TRUE(CSTL_string_reserve(&cstl_str, 36, alloc))
        << "must succeed and reserve capacity";

    data = CSTL_string_data(&cstl_str);

    CSTL_string_assign_n(&cstl_str, sample, 36, alloc);
    EXPECT_EQ(data, CSTL_string_data(&cstl_str))
        << "should not be invalidated with sufficient capacity";

    // May invalidate `data`:
    CSTL_string_clear(&cstl_str);
    CSTL_string_shrink_to_fit(&cstl_str, alloc);
}

TEST_F(StringTest, InsertCh) {
    // "333"
    real_str.insert(real_str.begin(), 3, '3');
    EXPECT_NE(nullptr, CSTL_string_insert_char(&cstl_str, CSTL_string_begin(&cstl_str), 3, '3', alloc))
        << "must return a valid iterator position for valid inputs";
    string_expect_equal();

    // "4444333"
    real_str.insert(real_str.begin(), 4, '4');
    EXPECT_NE(nullptr, CSTL_string_insert_char(&cstl_str, CSTL_string_begin(&cstl_str), 4, '4', alloc))
        << "must return a valid iterator position for valid inputs";
    string_expect_equal();

    // "888888884444333"
    real_str.insert(real_str.begin(), 8, '8');
    EXPECT_NE(nullptr, CSTL_string_insert_char(&cstl_str, CSTL_string_begin(&cstl_str), 8, '8', alloc))
        << "must return a valid iterator position for valid inputs";
    string_expect_equal();

    // "FFFFFFFFFFFFFFF888888884444333"
    real_str.insert(real_str.begin(), 15, 'F');
    EXPECT_NE(nullptr, CSTL_string_insert_char(&cstl_str, CSTL_string_begin(&cstl_str), 15, 'F', alloc))
        << "must return a valid iterator position for valid inputs";
    string_expect_equal();

    // "333"
    real_str.assign(3, '3');
    CSTL_string_assign_char(&cstl_str, 3, '3', alloc);
    string_expect_equal();

    // "4444333"
    real_str.insert(real_str.begin(), 4, '4');
    EXPECT_NE(nullptr, CSTL_string_insert_char(&cstl_str, CSTL_string_begin(&cstl_str), 4, '4', alloc))
        << "must return a valid iterator position for valid inputs";
    string_expect_equal();

    // "888888884444333"
    real_str.insert(real_str.begin(), 8, '8');
    EXPECT_NE(nullptr, CSTL_string_insert_char(&cstl_str, CSTL_string_begin(&cstl_str), 8, '8', alloc))
        << "must return a valid iterator position for valid inputs";
    string_expect_equal();

    // "FFFFFFFFFFFFFFF888888884444333"
    real_str.insert(real_str.begin(), 15, 'F');
    EXPECT_NE(nullptr, CSTL_string_insert_char(&cstl_str, CSTL_string_begin(&cstl_str), 15, 'F', alloc))
        << "must return a valid iterator position for valid inputs";
    string_expect_equal();
}

TEST_F(StringTest, InsertAtMid) {
	// "012"
    real_str.insert(0, sample, 3);
    EXPECT_TRUE(CSTL_string_insert_n_at(&cstl_str, 0, sample, 3, alloc))
        << "must return true with valid inputs";
    string_expect_equal();

    // "0012312"
    real_str.insert(real_str.size() / 2, sample, 4);
    EXPECT_TRUE(CSTL_string_insert_n_at(&cstl_str, CSTL_string_size(&cstl_str) / 2, sample, 4, alloc))
        << "must return true with valid inputs";
    string_expect_equal();

    // "001012345672312"
    real_str.insert(real_str.size() / 2, sample, 8);
    EXPECT_TRUE(CSTL_string_insert_n_at(&cstl_str, CSTL_string_size(&cstl_str) / 2, sample, 8, alloc))
        << "must return true with valid inputs";
    string_expect_equal();

    // "00101230123456789ABCDE45672312"
    real_str.insert(real_str.size() / 2, sample, 15);
    EXPECT_TRUE(CSTL_string_insert_n_at(&cstl_str, CSTL_string_size(&cstl_str) / 2, sample, 15, alloc))
        << "must return true with valid inputs";
    string_expect_equal();

    // "012"
    real_str.assign(sample, 3);
    CSTL_string_assign_n(&cstl_str, sample, 3, alloc);
    string_expect_equal();

    // "0012312"
    real_str.insert(real_str.size() / 2, sample, 4);
    EXPECT_TRUE(CSTL_string_insert_n_at(&cstl_str, CSTL_string_size(&cstl_str) / 2, sample, 4, alloc))
        << "must return true with valid inputs";
    string_expect_equal();

    // "001012345672312"
    real_str.insert(real_str.size() / 2, sample, 8);
    EXPECT_TRUE(CSTL_string_insert_n_at(&cstl_str, CSTL_string_size(&cstl_str) / 2, sample, 8, alloc))
        << "must return true with valid inputs";
    string_expect_equal();

    // "00101230123456789ABCDE45672312"
    real_str.insert(real_str.size() / 2, sample, 15);
    EXPECT_TRUE(CSTL_string_insert_n_at(&cstl_str, CSTL_string_size(&cstl_str) / 2, sample, 15, alloc))
        << "must return true with valid inputs";
    string_expect_equal();
}

TEST_F(StringTest, FindAndErase) {
    // Full sample string:
    real_str.assign(sample);
    CSTL_string_assign(&cstl_str, sample, alloc);
    string_expect_equal();

    // find "ABC"
    size_t real_pos = real_str.find("ABC");
    size_t cstl_pos = CSTL_string_find(&cstl_str, "ABC", 0);

    ASSERT_NE(std::string::npos, real_pos) << "must have a match";
    ASSERT_EQ(real_pos, cstl_pos) << "matches must be equal";

    real_str.erase(real_str.begin() + real_pos, real_str.begin() + real_pos + 3);
    const char* first = CSTL_string_const_begin(&cstl_str) + real_pos;
    const char* last  = first + 3;
    EXPECT_NE(nullptr, CSTL_string_erase_substr(&cstl_str, first, last))
        << "must return a valid iterator position";
    string_expect_equal();

    // find "8DEF"
    real_pos = real_str.find("8DEF");
    cstl_pos = CSTL_string_find(&cstl_str, "8DEF", 0);

    ASSERT_EQ(std::string::npos, real_pos) << "must not have a match";
    ASSERT_EQ(real_pos, cstl_pos) << "(non)matches must be equal";

    // find '9'
    real_pos = real_str.find('9');
    cstl_pos = CSTL_string_find_char(&cstl_str, '9', 0);

    ASSERT_NE(std::string::npos, real_pos) << "must have a match";
    ASSERT_EQ(real_pos, cstl_pos) << "matches must be equal";

    real_str.erase(real_str.begin() + real_pos);
    EXPECT_NE(nullptr, CSTL_string_erase(&cstl_str, CSTL_string_const_begin(&cstl_str)
        + real_pos)) << "must return a valid iterator position";
    string_expect_equal();

    // find "8DEF"
    real_pos = real_str.find("8DEF");
    cstl_pos = CSTL_string_find(&cstl_str, "8DEF", 0);

    ASSERT_NE(std::string::npos, real_pos) << "must have a match";
    ASSERT_EQ(real_pos, cstl_pos) << "matches must be equal";

    real_str.erase(real_pos, 4);
    EXPECT_TRUE(CSTL_string_erase_substr_at(&cstl_str, real_pos, 4))
        << "must return true with valid inputs";
    string_expect_equal();

    // find "8DEF"
    real_pos = real_str.find("8DEF");
    cstl_pos = CSTL_string_find(&cstl_str, "8DEF", 0);

    ASSERT_EQ(std::string::npos, real_pos) << "must not have a match";
    ASSERT_EQ(real_pos, cstl_pos) << "(non)matches must be equal";
}

TEST_F(StringTest, TestAppend) {
	// "012"
    real_str.append(sample, 3);
    CSTL_string_append_n(&cstl_str, sample, 3, alloc);
    string_expect_equal();

    // "0121012"
    real_str.append(1, '1');
    CSTL_string_append_char(&cstl_str, 1, '1', alloc);
    real_str.append(sample, 3);
    CSTL_string_append_n(&cstl_str, sample, 3, alloc);
    string_expect_equal();

    // "012101210121012"
    real_str.append(1, '1');
    CSTL_string_append_char(&cstl_str, 1, '1', alloc);
    real_str.append(real_str, 0, 7);
    CSTL_string_append_substr(&cstl_str, &cstl_str, 0, 7, alloc);
    string_expect_equal();

    // "01210121012101210121012101210121"
    real_str.append(1, '1');
    CSTL_string_append_char(&cstl_str, 1, '1', alloc);
    real_str.append(real_str);
    CSTL_string_append_str(&cstl_str, &cstl_str, alloc);
    string_expect_equal();
	// "012"
    real_str.append(sample, 3);
    CSTL_string_append_n(&cstl_str, sample, 3, alloc);
    string_expect_equal();

    // "0121012"
    real_str.append(1, '1');
    CSTL_string_append_char(&cstl_str, 1, '1', alloc);
    real_str.append(sample, 3);
    CSTL_string_append_n(&cstl_str, sample, 3, alloc);
    string_expect_equal();

    // "012101210121012"
    real_str.append(1, '1');
    CSTL_string_append_char(&cstl_str, 1, '1', alloc);
    real_str.append(real_str, 0, 7);
    CSTL_string_append_substr(&cstl_str, &cstl_str, 0, 7, alloc);
    string_expect_equal();

    // "01210121012101210121012101210121"
    real_str.append(1, '1');
    CSTL_string_append_char(&cstl_str, 1, '1', alloc);
    real_str.append(real_str);
    CSTL_string_append_str(&cstl_str, &cstl_str, alloc);
    string_expect_equal();
}

TEST_F(StringTest, TestReplace) {
	// "00123"
    real_str.assign(sample, 3);
    CSTL_string_assign_n(&cstl_str, sample, 3, alloc);
    real_str.replace(real_str.begin() + 1, real_str.end(), sample, 4);
    const char* first = CSTL_string_const_begin(&cstl_str) + 1;
    const char* last  = CSTL_string_const_end(&cstl_str);
    ASSERT_TRUE(CSTL_string_replace_n(&cstl_str, first, last, sample, 4, alloc))
        << "must return true with valid inputs";
    string_expect_equal();

	// "000123123"
    real_str.replace(real_str.begin() + 1, real_str.begin() + 2, real_str);
    first = CSTL_string_const_begin(&cstl_str) + 1;
    last  = CSTL_string_const_begin(&cstl_str) + 2;
    ASSERT_TRUE(CSTL_string_replace_str(&cstl_str, first, last, &cstl_str, alloc))
        << "must return true with valid inputs";
    string_expect_equal();

	// "123123"
    real_str.replace(real_str.begin(), real_str.begin() + 3, 0, '4');
    first = CSTL_string_const_begin(&cstl_str);
    last  = CSTL_string_const_begin(&cstl_str) + 3;
    ASSERT_TRUE(CSTL_string_replace_char(&cstl_str, first, last, 0, '4', alloc))
        << "must return true with valid inputs";
    string_expect_equal();

	// "120123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    real_str.replace(2, std::string::npos, sample, 36);
    ASSERT_TRUE(CSTL_string_replace_n_at(&cstl_str, 2, std::string::npos, sample, 36, alloc))
        << "must return true with valid inputs";
    string_expect_equal();

	// "122222223456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    real_str.replace(2, 2, 5, '2');
    ASSERT_TRUE(CSTL_string_replace_char_at(&cstl_str, 2, 2, 5, '2', alloc))
        << "must return true with valid inputs";
    string_expect_equal();

	// "122222223ABCDEFGHIJKLMNOPQRSTUDEFGHIJKLMNOPQRSTUVWXYZ"
    real_str.replace(9, 9, real_str, 15, 21);
    ASSERT_TRUE(CSTL_string_replace_substr_at(&cstl_str, 9, 9, &cstl_str, 15, 21, alloc))
        << "must return true with valid inputs";
    string_expect_equal();
}

TEST_F(StringTest, CopyString) {
    real_str.assign(sample, 3);
    CSTL_string_assign_n(&cstl_str, sample, 3, alloc);
    EXPECT_EQ(0, CSTL_string_copy(&cstl_str, real_str.data(), 0, 0))
        << "returned correct number of characters written";
    EXPECT_EQ(2, CSTL_string_copy(&cstl_str, real_str.data(), std::string::npos, 1))
        << "returned correct number of characters written";

    real_str.assign(sample, 7);
    CSTL_string_assign_n(&cstl_str, sample, 7, alloc);
    EXPECT_EQ(3, CSTL_string_copy(&cstl_str, real_str.data(), 3, 2))
        << "returned correct number of characters written";

    real_str.assign(sample);
    CSTL_string_assign(&cstl_str, sample, alloc);
    EXPECT_EQ(real_str.size(), CSTL_string_copy(&cstl_str, real_str.data(), std::string::npos, 0))
        << "returned correct number of characters written";
}

TEST_F(StringTest, ResizeString) {
    real_str.resize(3, '0');
    CSTL_string_resize(&cstl_str, 3, '0', alloc);
    string_expect_equal();

    real_str.resize(32, '1');
    CSTL_string_resize(&cstl_str, 32, '1', alloc);
    string_expect_equal();

    real_str.assign(sample);
    CSTL_string_assign(&cstl_str, sample, alloc);
    real_str.resize(3, '2');
    CSTL_string_resize(&cstl_str, 3, '2', alloc);
    string_expect_equal();
}
