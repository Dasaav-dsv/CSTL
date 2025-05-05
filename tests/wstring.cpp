#include <gtest/gtest.h>

#include <string>

#include "alloc.h"
#include "xstring.h"

class WideStringTest : public testing::Test {
protected:
    WideStringTest() : alloc{nullptr} {
        sample = L"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

        CSTL_wstring_construct(&cstl_str);
    }

    ~WideStringTest() {
        CSTL_wstring_destroy(&cstl_str, alloc);
    }

    void string_expect_equal() {
        std::wstring cstl_copy {
            CSTL_wstring_c_str(&cstl_str),
            CSTL_wstring_size(&cstl_str)
        };

        EXPECT_EQ(real_str, cstl_copy)
            << "strings should compare equal";
    }

    // Genuine STL string:
    std::wstring real_str;
    const wchar_t* sample;

    // CSTL string:
    CSTL_WideStringVal cstl_str;
    CSTL_Alloc* alloc;
};

TEST_F(WideStringTest, Default) {
    string_expect_equal();

    EXPECT_TRUE(CSTL_wstring_empty(&cstl_str))
        << "default constructed string must be empty";

    EXPECT_EQ(0, CSTL_wstring_size(&cstl_str))
        << "default constructed string should have a size of 0";

    EXPECT_EQ(CSTL_wstring_begin(&cstl_str), CSTL_wstring_end(&cstl_str))
        << "`first` should equal `last` when empty";

    EXPECT_STREQ(CSTL_wstring_c_str(&cstl_str), L"")
        << "`c_str` must return a valid null-terminated empty string";
}

TEST_F(WideStringTest, AssignEq) {
    // "012"
    real_str.assign(sample, 3);
    CSTL_wstring_assign_n(&cstl_str, sample, 3, alloc);
    string_expect_equal();

    // "0123456"
    real_str.assign(sample, 7);
    CSTL_wstring_assign_n(&cstl_str, sample, 7, alloc);
    string_expect_equal();

    // "0123456789ABCDE"
    real_str.assign(sample, 15);
    CSTL_wstring_assign_n(&cstl_str, sample, 15, alloc);
    string_expect_equal();

    // "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    real_str.assign(sample, 36);
    CSTL_wstring_assign_n(&cstl_str, sample, 36, alloc);
    string_expect_equal();

    // Full sample string:
    real_str.assign(sample);
    CSTL_wstring_assign(&cstl_str, sample, alloc);
    string_expect_equal();

    // "0123456789ABCDE"
    real_str.assign(sample, 15);
    CSTL_wstring_assign_n(&cstl_str, sample, 15, alloc);
    string_expect_equal();

    // "0123456"
    real_str.assign(sample, 7);
    CSTL_wstring_assign_n(&cstl_str, sample, 7, alloc);
    string_expect_equal();

    // "012"
    real_str.assign(sample, 3);
    CSTL_wstring_assign_n(&cstl_str, sample, 3, alloc);
    string_expect_equal();
}

TEST_F(WideStringTest, Substring) {
    // "012"
    real_str.assign(sample, 3);
    CSTL_wstring_assign_n(&cstl_str, sample, 3, alloc);
    string_expect_equal();

    // "12"
    real_str = real_str.substr(1, 2);
    CSTL_wstring_substr(&cstl_str, &cstl_str, 1, 2, alloc);
    string_expect_equal();

    // "1"
    real_str = real_str.substr(0, 1);
    CSTL_wstring_substr(&cstl_str, &cstl_str, 0, 1, alloc);
    string_expect_equal();

    // "0123456"
    real_str.assign(sample, 7);
    CSTL_wstring_assign_n(&cstl_str, sample, 7, alloc);
    string_expect_equal();

    // "23456"
    real_str = real_str.substr(2, 5);
    CSTL_wstring_substr(&cstl_str, &cstl_str, 2, 5, alloc);
    string_expect_equal();
    
    // "234"
    real_str = real_str.substr(0, 3);
    CSTL_wstring_substr(&cstl_str, &cstl_str, 0, 3, alloc);
    string_expect_equal();

    // Full sample string:
    real_str.assign(sample);
    CSTL_wstring_assign(&cstl_str, sample, alloc);
    string_expect_equal();
    
    // "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    real_str = real_str.substr(10, 26);
    CSTL_wstring_substr(&cstl_str, &cstl_str, 10, 26, alloc);
    string_expect_equal();
    
    // "XYZ"
    real_str = real_str.substr(23, 3);
    CSTL_wstring_substr(&cstl_str, &cstl_str, 23, 3, alloc);
    string_expect_equal();
}

TEST_F(WideStringTest, CopyAndMove) {
    CSTL_WideStringVal other;
    CSTL_wstring_construct(&other);

    // "012"
    real_str.assign(sample, 3);
    CSTL_wstring_assign_n(&cstl_str, sample, 3, alloc);
    CSTL_wstring_copy_assign(&other, alloc, &cstl_str, alloc, true);
    CSTL_wstring_move_assign(&cstl_str, alloc, &other, alloc, true);
    string_expect_equal();

    // "0123456"
    real_str.assign(sample, 7);
    CSTL_wstring_assign_n(&cstl_str, sample, 7, alloc);
    CSTL_wstring_copy_assign(&other, alloc, &cstl_str, alloc, true);
    CSTL_wstring_move_assign(&cstl_str, alloc, &other, alloc, true);
    string_expect_equal();

    // "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    real_str.assign(sample, 36);
    CSTL_wstring_assign_n(&cstl_str, sample, 36, alloc);
    CSTL_wstring_copy_assign(&other, alloc, &cstl_str, alloc, true);
    CSTL_wstring_move_assign(&cstl_str, alloc, &other, alloc, true);
    string_expect_equal();

    // Full sample string:
    real_str.assign(sample);
    CSTL_wstring_assign(&cstl_str, sample, alloc);
    CSTL_wstring_copy_assign(&other, alloc, &cstl_str, alloc, true);
    CSTL_wstring_move_assign(&cstl_str, alloc, &other, alloc, true);
    string_expect_equal();

    // "0123456789ABCDE"
    real_str.assign(sample, 15);
    CSTL_wstring_assign_n(&cstl_str, sample, 15, alloc);
    CSTL_wstring_copy_assign(&other, alloc, &cstl_str, alloc, true);
    CSTL_wstring_move_assign(&cstl_str, alloc, &other, alloc, true);
    string_expect_equal();

    // "0123456"
    real_str.assign(sample, 7);
    CSTL_wstring_assign_n(&cstl_str, sample, 7, alloc);
    CSTL_wstring_copy_assign(&other, alloc, &cstl_str, alloc, true);
    CSTL_wstring_move_assign(&cstl_str, alloc, &other, alloc, true);
    string_expect_equal();

    // "012"
    real_str.assign(sample, 3);
    CSTL_wstring_assign_n(&cstl_str, sample, 3, alloc);
    CSTL_wstring_copy_assign(&other, alloc, &cstl_str, alloc, true);
    CSTL_wstring_move_assign(&cstl_str, alloc, &other, alloc, true);
    string_expect_equal();

    CSTL_wstring_destroy(&other, alloc);
}

TEST_F(WideStringTest, IndexAt) {
    // "012"
    real_str.assign(sample, 3);
    CSTL_wstring_assign_n(&cstl_str, sample, 3, alloc);
    EXPECT_EQ(L'2', *CSTL_wstring_at(&cstl_str, 2));
    EXPECT_EQ(nullptr, CSTL_wstring_at(&cstl_str, 3));

    // "0123456"
    real_str.assign(sample, 7);
    CSTL_wstring_assign_n(&cstl_str, sample, 7, alloc);
    EXPECT_EQ(L'6', *CSTL_wstring_at(&cstl_str, 6));
    EXPECT_EQ(nullptr, CSTL_wstring_at(&cstl_str, 7));

    // "0123456789ABCDE"
    real_str.assign(sample, 15);
    CSTL_wstring_assign_n(&cstl_str, sample, 15, alloc);
    EXPECT_EQ(L'E', *CSTL_wstring_at(&cstl_str, 14));
    EXPECT_EQ(nullptr, CSTL_wstring_at(&cstl_str, 15));

    // "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    real_str.assign(sample, 36);
    CSTL_wstring_assign_n(&cstl_str, sample, 36, alloc);
    EXPECT_EQ(L'Z', *CSTL_wstring_at(&cstl_str, 35));
    EXPECT_EQ(nullptr, CSTL_wstring_at(&cstl_str, 36));

    // Full sample string:
    real_str.assign(sample);
    CSTL_wstring_assign(&cstl_str, sample, alloc);
    EXPECT_EQ(L'z', *CSTL_wstring_at(&cstl_str, 61));
    EXPECT_EQ(nullptr, CSTL_wstring_at(&cstl_str, 62));

    // "0123456789ABCDE"
    real_str.assign(sample, 15);
    CSTL_wstring_assign_n(&cstl_str, sample, 15, alloc);
    EXPECT_EQ(L'E', *CSTL_wstring_at(&cstl_str, 14));
    EXPECT_EQ(nullptr, CSTL_wstring_at(&cstl_str, 15));

    // "0123456"
    real_str.assign(sample, 7);
    CSTL_wstring_assign_n(&cstl_str, sample, 7, alloc);
    EXPECT_EQ(L'6', *CSTL_wstring_at(&cstl_str, 6));
    EXPECT_EQ(nullptr, CSTL_wstring_at(&cstl_str, 7));

    // "012"
    real_str.assign(sample, 3);
    CSTL_wstring_assign_n(&cstl_str, sample, 3, alloc);
    EXPECT_EQ(L'2', *CSTL_wstring_at(&cstl_str, 2));
    EXPECT_EQ(nullptr, CSTL_wstring_at(&cstl_str, 3));
}

TEST_F(WideStringTest, FrontAndBack) {
    CSTL_wstring_push_back(&cstl_str, L'0', alloc);
    EXPECT_EQ(CSTL_wstring_back(&cstl_str), CSTL_wstring_front(&cstl_str))
        << "`front` and `back` must point to the same element";

    CSTL_wstring_push_back(&cstl_str, L'1', alloc);
    EXPECT_NE(CSTL_wstring_back(&cstl_str), CSTL_wstring_front(&cstl_str))
        << "`front` and `back` must not point to the same element";

    CSTL_wstring_pop_back(&cstl_str);
    EXPECT_EQ(CSTL_wstring_back(&cstl_str), CSTL_wstring_front(&cstl_str))
        << "`front` and `back` must point to the same element";
}

TEST_F(WideStringTest, Capacity) {
    ASSERT_FALSE(CSTL_wstring_reserve(&cstl_str, CSTL_wstring_max_size() + 1, alloc))
        << "must fail and return false";

    ASSERT_TRUE(CSTL_wstring_reserve(&cstl_str, 3, alloc))
        << "must succeed (does nothing)";

    wchar_t* data = CSTL_wstring_data(&cstl_str);

    CSTL_wstring_assign_n(&cstl_str, sample, 3, alloc);
    EXPECT_EQ(data, CSTL_wstring_data(&cstl_str))
        << "should not be invalidated with sufficient capacity";

    ASSERT_TRUE(CSTL_wstring_reserve(&cstl_str, 36, alloc))
        << "must succeed and reserve capacity";

    data = CSTL_wstring_data(&cstl_str);

    CSTL_wstring_assign_n(&cstl_str, sample, 36, alloc);
    EXPECT_EQ(data, CSTL_wstring_data(&cstl_str))
        << "should not be invalidated with sufficient capacity";

    // May invalidate `data`:
    CSTL_wstring_clear(&cstl_str);
    CSTL_wstring_shrink_to_fit(&cstl_str, alloc);
}

TEST_F(WideStringTest, InsertCh) {
    // "333"
    real_str.insert(real_str.begin(), 3, L'3');
    EXPECT_NE(nullptr, CSTL_wstring_insert_char(&cstl_str, CSTL_wstring_begin(&cstl_str), 3, L'3', alloc))
        << "must return a valid iterator position for valid inputs";
    string_expect_equal();

    // "4444333"
    real_str.insert(real_str.begin(), 4, L'4');
    EXPECT_NE(nullptr, CSTL_wstring_insert_char(&cstl_str, CSTL_wstring_begin(&cstl_str), 4, L'4', alloc))
        << "must return a valid iterator position for valid inputs";
    string_expect_equal();

    // "888888884444333"
    real_str.insert(real_str.begin(), 8, L'8');
    EXPECT_NE(nullptr, CSTL_wstring_insert_char(&cstl_str, CSTL_wstring_begin(&cstl_str), 8, L'8', alloc))
        << "must return a valid iterator position for valid inputs";
    string_expect_equal();

    // "FFFFFFFFFFFFFFF888888884444333"
    real_str.insert(real_str.begin(), 15, L'F');
    EXPECT_NE(nullptr, CSTL_wstring_insert_char(&cstl_str, CSTL_wstring_begin(&cstl_str), 15, L'F', alloc))
        << "must return a valid iterator position for valid inputs";
    string_expect_equal();

    // "333"
    real_str.assign(3, L'3');
    CSTL_wstring_assign_char(&cstl_str, 3, L'3', alloc);
    string_expect_equal();

    // "4444333"
    real_str.insert(real_str.begin(), 4, L'4');
    EXPECT_NE(nullptr, CSTL_wstring_insert_char(&cstl_str, CSTL_wstring_begin(&cstl_str), 4, L'4', alloc))
        << "must return a valid iterator position for valid inputs";
    string_expect_equal();

    // "888888884444333"
    real_str.insert(real_str.begin(), 8, L'8');
    EXPECT_NE(nullptr, CSTL_wstring_insert_char(&cstl_str, CSTL_wstring_begin(&cstl_str), 8, L'8', alloc))
        << "must return a valid iterator position for valid inputs";
    string_expect_equal();

    // "FFFFFFFFFFFFFFF888888884444333"
    real_str.insert(real_str.begin(), 15, L'F');
    EXPECT_NE(nullptr, CSTL_wstring_insert_char(&cstl_str, CSTL_wstring_begin(&cstl_str), 15, L'F', alloc))
        << "must return a valid iterator position for valid inputs";
    string_expect_equal();
}

TEST_F(WideStringTest, InsertAtMid) {
	// "012"
    real_str.insert(0, sample, 3);
    EXPECT_TRUE(CSTL_wstring_insert_n_at(&cstl_str, 0, sample, 3, alloc))
        << "must return true with valid inputs";
    string_expect_equal();

    // "0012312"
    real_str.insert(real_str.size() / 2, sample, 4);
    EXPECT_TRUE(CSTL_wstring_insert_n_at(&cstl_str, CSTL_wstring_size(&cstl_str) / 2, sample, 4, alloc))
        << "must return true with valid inputs";
    string_expect_equal();

    // "001012345672312"
    real_str.insert(real_str.size() / 2, sample, 8);
    EXPECT_TRUE(CSTL_wstring_insert_n_at(&cstl_str, CSTL_wstring_size(&cstl_str) / 2, sample, 8, alloc))
        << "must return true with valid inputs";
    string_expect_equal();

    // "00101230123456789ABCDE45672312"
    real_str.insert(real_str.size() / 2, sample, 15);
    EXPECT_TRUE(CSTL_wstring_insert_n_at(&cstl_str, CSTL_wstring_size(&cstl_str) / 2, sample, 15, alloc))
        << "must return true with valid inputs";
    string_expect_equal();

    // "012"
    real_str.assign(sample, 3);
    CSTL_wstring_assign_n(&cstl_str, sample, 3, alloc);
    string_expect_equal();

    // "0012312"
    real_str.insert(real_str.size() / 2, sample, 4);
    EXPECT_TRUE(CSTL_wstring_insert_n_at(&cstl_str, CSTL_wstring_size(&cstl_str) / 2, sample, 4, alloc))
        << "must return true with valid inputs";
    string_expect_equal();

    // "001012345672312"
    real_str.insert(real_str.size() / 2, sample, 8);
    EXPECT_TRUE(CSTL_wstring_insert_n_at(&cstl_str, CSTL_wstring_size(&cstl_str) / 2, sample, 8, alloc))
        << "must return true with valid inputs";
    string_expect_equal();

    // "00101230123456789ABCDE45672312"
    real_str.insert(real_str.size() / 2, sample, 15);
    EXPECT_TRUE(CSTL_wstring_insert_n_at(&cstl_str, CSTL_wstring_size(&cstl_str) / 2, sample, 15, alloc))
        << "must return true with valid inputs";
    string_expect_equal();
}

TEST_F(WideStringTest, FindAndErase) {
    // Full sample string:
    real_str.assign(sample);
    CSTL_wstring_assign(&cstl_str, sample, alloc);
    string_expect_equal();

    // find "ABC"
    size_t real_pos = real_str.find(L"ABC");
    size_t cstl_pos = CSTL_wstring_find(&cstl_str, L"ABC", 0);

    ASSERT_NE(std::wstring::npos, real_pos) << "must have a match";
    ASSERT_EQ(real_pos, cstl_pos) << "matches must be equal";

    real_str.erase(real_str.begin() + real_pos, real_str.begin() + real_pos + 3);
    const wchar_t* first = CSTL_wstring_const_begin(&cstl_str) + real_pos;
    const wchar_t* last  = first + 3;
    EXPECT_NE(nullptr, CSTL_wstring_erase_substr(&cstl_str, first, last))
        << "must return a valid iterator position";
    string_expect_equal();

    // find "8DEF"
    real_pos = real_str.find(L"8DEF");
    cstl_pos = CSTL_wstring_find(&cstl_str, L"8DEF", 0);

    ASSERT_EQ(std::wstring::npos, real_pos) << "must not have a match";
    ASSERT_EQ(real_pos, cstl_pos) << "(non)matches must be equal";

    // find '9'
    real_pos = real_str.find(L'9');
    cstl_pos = CSTL_wstring_find_char(&cstl_str, L'9', 0);

    ASSERT_NE(std::wstring::npos, real_pos) << "must have a match";
    ASSERT_EQ(real_pos, cstl_pos) << "matches must be equal";

    real_str.erase(real_str.begin() + real_pos);
    EXPECT_NE(nullptr, CSTL_wstring_erase(&cstl_str, CSTL_wstring_const_begin(&cstl_str)
        + real_pos)) << "must return a valid iterator position";
    string_expect_equal();

    // find "8DEF"
    real_pos = real_str.find(L"8DEF");
    cstl_pos = CSTL_wstring_find(&cstl_str, L"8DEF", 0);

    ASSERT_NE(std::wstring::npos, real_pos) << "must have a match";
    ASSERT_EQ(real_pos, cstl_pos) << "matches must be equal";

    real_str.erase(real_pos, 4);
    EXPECT_TRUE(CSTL_wstring_erase_substr_at(&cstl_str, real_pos, 4))
        << "must return true with valid inputs";
    string_expect_equal();

    // find "8DEF"
    real_pos = real_str.find(L"8DEF");
    cstl_pos = CSTL_wstring_find(&cstl_str, L"8DEF", 0);

    ASSERT_EQ(std::wstring::npos, real_pos) << "must not have a match";
    ASSERT_EQ(real_pos, cstl_pos) << "(non)matches must be equal";
}

TEST_F(WideStringTest, TestAppend) {
	// "012"
    real_str.append(sample, 3);
    CSTL_wstring_append_n(&cstl_str, sample, 3, alloc);
    string_expect_equal();

    // "0121012"
    real_str.append(1, L'1');
    CSTL_wstring_append_char(&cstl_str, 1, L'1', alloc);
    real_str.append(sample, 3);
    CSTL_wstring_append_n(&cstl_str, sample, 3, alloc);
    string_expect_equal();

    // "012101210121012"
    real_str.append(1, L'1');
    CSTL_wstring_append_char(&cstl_str, 1, L'1', alloc);
    real_str.append(real_str, 0, 7);
    CSTL_wstring_append_substr(&cstl_str, &cstl_str, 0, 7, alloc);
    string_expect_equal();

    // "01210121012101210121012101210121"
    real_str.append(1, L'1');
    CSTL_wstring_append_char(&cstl_str, 1, L'1', alloc);
    real_str.append(real_str);
    CSTL_wstring_append_str(&cstl_str, &cstl_str, alloc);
    string_expect_equal();
	// "012"
    real_str.append(sample, 3);
    CSTL_wstring_append_n(&cstl_str, sample, 3, alloc);
    string_expect_equal();

    // "0121012"
    real_str.append(1, L'1');
    CSTL_wstring_append_char(&cstl_str, 1, L'1', alloc);
    real_str.append(sample, 3);
    CSTL_wstring_append_n(&cstl_str, sample, 3, alloc);
    string_expect_equal();

    // "012101210121012"
    real_str.append(1, L'1');
    CSTL_wstring_append_char(&cstl_str, 1, L'1', alloc);
    real_str.append(real_str, 0, 7);
    CSTL_wstring_append_substr(&cstl_str, &cstl_str, 0, 7, alloc);
    string_expect_equal();

    // "01210121012101210121012101210121"
    real_str.append(1, L'1');
    CSTL_wstring_append_char(&cstl_str, 1, L'1', alloc);
    real_str.append(real_str);
    CSTL_wstring_append_str(&cstl_str, &cstl_str, alloc);
    string_expect_equal();
}

TEST_F(WideStringTest, TestReplace) {
	// "00123"
    real_str.assign(sample, 3);
    CSTL_wstring_assign_n(&cstl_str, sample, 3, alloc);
    real_str.replace(real_str.begin() + 1, real_str.end(), sample, 4);
    const wchar_t* first = CSTL_wstring_const_begin(&cstl_str) + 1;
    const wchar_t* last  = CSTL_wstring_const_end(&cstl_str);
    ASSERT_TRUE(CSTL_wstring_replace_n(&cstl_str, first, last, sample, 4, alloc))
        << "must return true with valid inputs";
    string_expect_equal();

	// "000123123"
    real_str.replace(real_str.begin() + 1, real_str.begin() + 2, real_str);
    first = CSTL_wstring_const_begin(&cstl_str) + 1;
    last  = CSTL_wstring_const_begin(&cstl_str) + 2;
    ASSERT_TRUE(CSTL_wstring_replace_str(&cstl_str, first, last, &cstl_str, alloc))
        << "must return true with valid inputs";
    string_expect_equal();

	// "123123"
    real_str.replace(real_str.begin(), real_str.begin() + 3, 0, L'4');
    first = CSTL_wstring_const_begin(&cstl_str);
    last  = CSTL_wstring_const_begin(&cstl_str) + 3;
    ASSERT_TRUE(CSTL_wstring_replace_char(&cstl_str, first, last, 0, L'4', alloc))
        << "must return true with valid inputs";
    string_expect_equal();

	// "120123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    real_str.replace(2, std::wstring::npos, sample, 36);
    ASSERT_TRUE(CSTL_wstring_replace_n_at(&cstl_str, 2, std::wstring::npos, sample, 36, alloc))
        << "must return true with valid inputs";
    string_expect_equal();

	// "122222223456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    real_str.replace(2, 2, 5, L'2');
    ASSERT_TRUE(CSTL_wstring_replace_char_at(&cstl_str, 2, 2, 5, L'2', alloc))
        << "must return true with valid inputs";
    string_expect_equal();

	// "122222223ABCDEFGHIJKLMNOPQRSTUDEFGHIJKLMNOPQRSTUVWXYZ"
    real_str.replace(9, 9, real_str, 15, 21);
    ASSERT_TRUE(CSTL_wstring_replace_substr_at(&cstl_str, 9, 9, &cstl_str, 15, 21, alloc))
        << "must return true with valid inputs";
    string_expect_equal();
}

TEST_F(WideStringTest, CopyString) {
    real_str.assign(sample, 3);
    CSTL_wstring_assign_n(&cstl_str, sample, 3, alloc);
    EXPECT_EQ(0, CSTL_wstring_copy(&cstl_str, real_str.data(), 0, 0))
        << "returned correct number of characters written";
    EXPECT_EQ(2, CSTL_wstring_copy(&cstl_str, real_str.data(), std::wstring::npos, 1))
        << "returned correct number of characters written";

    real_str.assign(sample, 7);
    CSTL_wstring_assign_n(&cstl_str, sample, 7, alloc);
    EXPECT_EQ(3, CSTL_wstring_copy(&cstl_str, real_str.data(), 3, 2))
        << "returned correct number of characters written";

    real_str.assign(sample);
    CSTL_wstring_assign(&cstl_str, sample, alloc);
    EXPECT_EQ(real_str.size(), CSTL_wstring_copy(&cstl_str, real_str.data(), std::wstring::npos, 0))
        << "returned correct number of characters written";
}

TEST_F(WideStringTest, ResizeString) {
    real_str.resize(3, L'0');
    CSTL_wstring_resize(&cstl_str, 3, L'0', alloc);
    string_expect_equal();

    real_str.resize(32, L'1');
    CSTL_wstring_resize(&cstl_str, 32, L'1', alloc);
    string_expect_equal();

    real_str.assign(sample);
    CSTL_wstring_assign(&cstl_str, sample, alloc);
    real_str.resize(3, L'2');
    CSTL_wstring_resize(&cstl_str, 3, L'2', alloc);
    string_expect_equal();
}
