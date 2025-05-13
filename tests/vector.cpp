#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <vector>

#include "alloc.h"
#include "type.h"
#include "vector.h"

struct TestInt {
    TestInt() : value{new uint32_t{0xC0FFEE}} {}

    TestInt(uint32_t value) : value{new uint32_t{value}} {}

    TestInt(const TestInt& other) : value{new uint32_t{*other.value}} {}

    TestInt(TestInt&& other) noexcept : value{new uint32_t{*other.value}} {
        *other.value = 0xC0FFEE;
    }

    ~TestInt() {
        delete value;
    }

    TestInt& operator=(const TestInt& other) {
        delete value;
        value = new uint32_t{*other.value};
        return *this;
    }

    TestInt& operator=(TestInt&& other) noexcept {
        delete value;
        value = new uint32_t{*other.value};
        *other.value = 0xC0FFEE;
        return *this;
    }

    friend bool operator==(const TestInt& lhs, const TestInt& rhs) {
        return *lhs.value == *rhs.value;
    }

    uint32_t* value;
};

void destroy_testint(void* _first, void* _last) {
    auto first = reinterpret_cast<TestInt*>(_first);
    auto last  = reinterpret_cast<TestInt*>(_last);

    ASSERT_EQ(0, (uintptr_t)first % alignof(TestInt))
        << "`first` alignment not upheld";

    ASSERT_EQ(0, (uintptr_t)last % alignof(TestInt))
        << "`last` alignment not upheld";

    std::destroy(first, last);
}

void move_testint(void* _first, void* _last, void* _dest) {
    auto first = reinterpret_cast<TestInt*>(_first);
    auto last  = reinterpret_cast<TestInt*>(_last);
    auto dest  = reinterpret_cast<TestInt*>(_dest);

    ASSERT_EQ(0, (uintptr_t)first % alignof(TestInt))
        << "`first` alignment not upheld";

    ASSERT_EQ(0, (uintptr_t)last % alignof(TestInt))
        << "`last` alignment not upheld";

    ASSERT_EQ(0, (uintptr_t)dest % alignof(TestInt))
        << "`dest` alignment not upheld";;

    std::uninitialized_move(first, last, dest);
}

void copy_testint(const void* _first, const void* _last, void* _dest) {
    auto first = reinterpret_cast<const TestInt*>(_first);
    auto last  = reinterpret_cast<const TestInt*>(_last);
    auto dest  = reinterpret_cast<TestInt*>(_dest);

    ASSERT_EQ(0, (uintptr_t)first % alignof(TestInt))
        << "`first` alignment not upheld";

    ASSERT_EQ(0, (uintptr_t)last % alignof(TestInt))
        << "`last` alignment not upheld";

    ASSERT_EQ(0, (uintptr_t)dest % alignof(TestInt))
        << "`dest` alignment not upheld";;

    std::uninitialized_copy(first, last, dest);
}

void fill_testint(void* _first, void* _last, const void* _value) {
    const auto& value = *reinterpret_cast<const TestInt*>(_value);

    auto first = reinterpret_cast<TestInt*>(_first);
    auto last  = reinterpret_cast<TestInt*>(_last);

    ASSERT_EQ(0, (uintptr_t)first % alignof(TestInt))
        << "`first` alignment not upheld";

    std::uninitialized_fill(first, last, value);
}

class VectorTest : public testing::Test {
protected:
    VectorTest() : real_int{0xDEADBEEF}, cstl_vec{}, copy{},
        cstl_int{reinterpret_cast<const void*>(&real_int.value)},
        alloc{nullptr}, type{} {}

    ~VectorTest() {
        CSTL_vector_destroy(&cstl_vec, type, &copy.move_type.drop_type, alloc);
    }

    void SetUp() override {
        type = CSTL_define_type(sizeof(TestInt), alignof(TestInt));

        ASSERT_NE(nullptr, type);

        copy = {
            {
                {
                    &destroy_testint
                },
                &move_testint
            },
            &copy_testint,
            &fill_testint
        };

        CSTL_vector_construct(&cstl_vec);
    }

    void vector_expect_size(size_t size) {
        EXPECT_EQ(size, CSTL_vector_size(&cstl_vec, type))
            << "size of vector must be equal to " << size;
    
        CSTL_VectorIter first = CSTL_vector_begin(&cstl_vec, type);
        CSTL_VectorIter last  = CSTL_vector_end(&cstl_vec, type);
    
        EXPECT_EQ(size, CSTL_vector_iterator_distance(first, last))
            << "`[first, last)` must span exactly " << size << " elements";
    }

    void vector_assert_equal() {
        for (size_t i = 0; i < real_vec.size(); ++i) {
            ASSERT_LT(i, CSTL_vector_size(&cstl_vec, type))
                << "vector should be bigger; i=" << i;
            
            TestInt left  = real_vec[i];
            TestInt right = *(TestInt*)CSTL_vector_index(&cstl_vec, type, i);

            EXPECT_EQ(left, right) << "must equal if correctly reallocated; i=" << i;
        }
    }

    // Genuine STL vector:
    std::vector<TestInt> real_vec;
    const TestInt real_int;

    // CSTL vector:
    CSTL_VectorVal cstl_vec;
    CSTL_CopyType copy;
    const void* cstl_int;
    CSTL_Alloc* alloc;
    CSTL_Type type;
};

TEST_F(VectorTest, Default) {
    EXPECT_LT(0, CSTL_vector_max_size(type))
        << "max vector size must be greater than 0";

    vector_expect_size(0);
}

TEST_F(VectorTest, PushTen) {
    for (int i = 0; i < 10; ++i) {
        ASSERT_TRUE(CSTL_vector_copy_push_back(&cstl_vec, type, &copy, cstl_int, alloc))
            << "must return true on success";

        real_vec.push_back(real_int);
    }

    vector_expect_size(10);
    vector_assert_equal();
}

TEST_F(VectorTest, AssignCopies) {
    real_vec.assign(5, real_int);
    EXPECT_TRUE(CSTL_vector_assign_n(&cstl_vec, type, &copy, 5, cstl_int, alloc))
        << "must return true on success";

    vector_expect_size(5);
    vector_assert_equal();

    // Will cause a reallocation:
    real_vec.assign(12, real_int);
    EXPECT_TRUE(CSTL_vector_assign_n(&cstl_vec, type, &copy, 12, cstl_int, alloc))
        << "must return true on success";

    vector_expect_size(12);
    vector_assert_equal();

    // Will reuse storage and resize:
    real_vec.assign(7, real_int);
    EXPECT_TRUE(CSTL_vector_assign_n(&cstl_vec, type, &copy, 7, cstl_int, alloc))
        << "must return true on success";

    vector_expect_size(7);
    vector_assert_equal();

    // Must fail and do nothing (too many elements):
    EXPECT_FALSE(CSTL_vector_assign_n(&cstl_vec, type, &copy, SIZE_MAX, cstl_int, alloc))
        << "must fail due to exceeding `CSTL_vector_max_size(&cstl_vec, type)`";

    vector_expect_size(7);
    vector_assert_equal();

    // Will destroy all elements:
    real_vec.assign(0, real_int);
    EXPECT_TRUE(CSTL_vector_assign_n(&cstl_vec, type, &copy, 0, cstl_int, alloc))
        << "must return true on success";

    vector_expect_size(0);
}

TEST_F(VectorTest, AssignRange) {
    real_vec.assign(5, real_int);
    EXPECT_TRUE(CSTL_vector_copy_assign_range(&cstl_vec, type, &copy,
        (const void*)real_vec.data(), (const void*)(real_vec.data() + real_vec.size()), alloc))
        << "must return true on success";

    vector_expect_size(5);
    vector_assert_equal();

    // Will cause a reallocation:
    real_vec.assign(12, real_int);
    EXPECT_TRUE(CSTL_vector_copy_assign_range(&cstl_vec, type, &copy,
        (const void*)real_vec.data(), (const void*)(real_vec.data() + real_vec.size()), alloc))
        << "must return true on success";

    vector_expect_size(12);
    vector_assert_equal();

    // Will reuse storage and resize:
    real_vec.assign(7, real_int);
    EXPECT_TRUE(CSTL_vector_copy_assign_range(&cstl_vec, type, &copy,
        (const void*)real_vec.data(), (const void*)(real_vec.data() + real_vec.size()), alloc))
        << "must return true on success";

    vector_expect_size(7);
    vector_assert_equal();

    // Will destroy all elements:
    real_vec.assign(0, real_int);
    EXPECT_TRUE(CSTL_vector_copy_assign_range(&cstl_vec, type, &copy,
        (const void*)real_vec.data(), (const void*)(real_vec.data() + real_vec.size()), alloc))
        << "must return true on success";

    vector_expect_size(0);
}

TEST_F(VectorTest, VectorCopy) {
    CSTL_VectorVal other_vec;
    CSTL_vector_construct(&other_vec);

    EXPECT_TRUE(CSTL_vector_assign_n(&cstl_vec, type, &copy, 5, cstl_int, alloc))
        << "must return true on success";

    EXPECT_TRUE(CSTL_vector_assign_n(&other_vec, type, &copy, 7, cstl_int, alloc))
        << "must return true on success";

    // Will expand storage.
    EXPECT_TRUE(CSTL_vector_copy_assign(&cstl_vec, type, &copy, &other_vec, alloc, alloc, false))
        << "must return true on success";

    EXPECT_EQ(CSTL_vector_size(&cstl_vec, type), CSTL_vector_size(&other_vec, type))
        << "vectors should have equal size after being copied";

    EXPECT_TRUE(CSTL_vector_assign_n(&other_vec, type, &copy, 5, cstl_int, alloc))
        << "must return true on success";

    // Will reuse storage.
    EXPECT_TRUE(CSTL_vector_copy_assign(&cstl_vec, type, &copy, &other_vec, alloc, alloc, false))
        << "must return true on success";

    EXPECT_EQ(CSTL_vector_size(&cstl_vec, type), CSTL_vector_size(&other_vec, type))
        << "vectors should have equal size after being copied";

    CSTL_vector_destroy(&other_vec, type, &copy.move_type.drop_type, alloc);
}

TEST_F(VectorTest, VectorMove) {
    CSTL_VectorVal other_vec;
    CSTL_vector_construct(&other_vec);

    EXPECT_TRUE(CSTL_vector_assign_n(&cstl_vec, type, &copy, 5, cstl_int, alloc))
        << "must return true on success";

    EXPECT_TRUE(CSTL_vector_assign_n(&other_vec, type, &copy, 7, cstl_int, alloc))
        << "must return true on success";

    // Will expand storage.
    EXPECT_TRUE(CSTL_vector_move_assign(&cstl_vec, type, &copy.move_type, &other_vec, alloc, alloc, false))
        << "must return true on success";
    size_t left_size = CSTL_vector_size(&cstl_vec, type);
    EXPECT_TRUE(CSTL_vector_move_assign(&other_vec, type, &copy.move_type, &cstl_vec, alloc, alloc, false))
        << "must return true on success";
    size_t right_size = CSTL_vector_size(&other_vec, type);

    EXPECT_EQ(left_size, right_size) << "moving should keep size intact";

    EXPECT_TRUE(CSTL_vector_assign_n(&other_vec, type, &copy, 5, cstl_int, alloc))
        << "must return true on success";

    // Will reuse storage.
    EXPECT_TRUE(CSTL_vector_move_assign(&cstl_vec, type, &copy.move_type, &other_vec, alloc, alloc, false))
        << "must return true on success";
    left_size = CSTL_vector_size(&cstl_vec, type);
    EXPECT_TRUE(CSTL_vector_move_assign(&other_vec, type, &copy.move_type, &cstl_vec, alloc, alloc, false))
        << "must return true on success";
    right_size = CSTL_vector_size(&other_vec, type);

    EXPECT_EQ(left_size, right_size) << "moving should keep size intact";

    CSTL_vector_destroy(&other_vec, type, &copy.move_type.drop_type, alloc);
}

TEST_F(VectorTest, CheckedIndex) {
    for (int i = 0; i < 10; ++i) {
        ASSERT_EQ(nullptr, CSTL_vector_const_at(&cstl_vec, type, i))
            << "the end of the vector must not have accesible elements";

        ASSERT_TRUE(CSTL_vector_copy_push_back(&cstl_vec, type, &copy, cstl_int, alloc))
            << "must return true on success";

        real_vec.push_back(real_int);
    }

    vector_expect_size(10);
    vector_assert_equal();

    for (int i = 0; i < 10; ++i) {
        ASSERT_NE(nullptr, CSTL_vector_const_at(&cstl_vec, type, i))
            << "all vector elements must be accesible";
    }
}

TEST_F(VectorTest, FrontAndBack) {
    ASSERT_TRUE(CSTL_vector_copy_push_back(&cstl_vec, type, &copy, cstl_int, alloc))
        << "must return true on success";

    ASSERT_EQ(CSTL_vector_const_front(&cstl_vec), CSTL_vector_const_back(&cstl_vec, type))
        << "must equal when there is only one element";

    ASSERT_TRUE(CSTL_vector_copy_push_back(&cstl_vec, type, &copy, cstl_int, alloc))
        << "must return true on success";

    ASSERT_NE(CSTL_vector_const_front(&cstl_vec), CSTL_vector_const_back(&cstl_vec, type))
        << "must equal when there is only one element";
}

TEST_F(VectorTest, DataRange) {
    for (int i = 0; i < 10; ++i) {
        // The range `[data(), data() + size)` must be valid.
        const TestInt* first = (const TestInt*)CSTL_vector_data(&cstl_vec);
        const TestInt* last  = first + CSTL_vector_size(&cstl_vec, type);

        real_vec.assign(first, last);

        ASSERT_TRUE(CSTL_vector_copy_push_back(&cstl_vec, type, &copy, cstl_int, alloc))
            << "must return true on success";
    }

    vector_expect_size(10);
    vector_assert_equal();
}

TEST_F(VectorTest, Reserve) {
    CSTL_vector_reserve(&cstl_vec, type, &copy.move_type, 10, alloc);

    CSTL_VectorIter first = CSTL_vector_begin(&cstl_vec, type);

    for (int i = 0; i < 10; ++i) {
        ASSERT_TRUE(CSTL_vector_copy_push_back(&cstl_vec, type, &copy, cstl_int, alloc))
            << "must return true on success";

        ASSERT_TRUE(CSTL_vector_iterator_eq(first, CSTL_vector_begin(&cstl_vec, type)))
            << "not exceeding reserved capacity should not invalidate iterators";
    }
}

TEST_F(VectorTest, Clear) {
    real_vec.assign(5, real_int);
    EXPECT_TRUE(CSTL_vector_assign_n(&cstl_vec, type, &copy, 5, cstl_int, alloc))
        << "must return true on success";

    real_vec.clear();
    CSTL_vector_clear(&cstl_vec, &copy.move_type.drop_type);

    vector_expect_size(0);
    vector_assert_equal();
}

TEST_F(VectorTest, EmplaceInsert) {
    real_vec.assign(10, real_int);

    for (int i = 0; i < 3; ++i) {
        CSTL_VectorIter first = CSTL_vector_begin(&cstl_vec, type);
        CSTL_VectorIter pos   = CSTL_vector_copy_insert(&cstl_vec, &copy, first, cstl_int, alloc);
        ASSERT_FALSE(CSTL_vector_iterator_eq(pos, CSTL_vector_end(&cstl_vec, type)))
            << "emplace must return a dereferenceable iterator";
    }

    for (int i = 0; i < 3; ++i) {
        CSTL_VectorIter last  = CSTL_vector_end(&cstl_vec, type);
        CSTL_VectorIter pos   = CSTL_vector_copy_insert(&cstl_vec, &copy, last, cstl_int, alloc);
        ASSERT_FALSE(CSTL_vector_iterator_eq(pos, CSTL_vector_end(&cstl_vec, type)))
            << "emplace must return a dereferenceable iterator";
    }

    CSTL_VectorIter first = CSTL_vector_begin(&cstl_vec, type);
    CSTL_VectorIter last  = CSTL_vector_end(&cstl_vec, type);
    CSTL_VectorIter mid   = CSTL_vector_iterator_add(first, CSTL_vector_iterator_distance(first, last) / 2);
    CSTL_VectorIter pos   = CSTL_vector_insert_n(&cstl_vec, &copy, mid, 4, cstl_int, alloc);
    
    ASSERT_FALSE(CSTL_vector_iterator_eq(pos, CSTL_vector_end(&cstl_vec, type)))
        << "insert must return a dereferenceable iterator";

    vector_expect_size(10);
    vector_assert_equal();
}

TEST_F(VectorTest, Erase) {
    real_vec.assign(5, real_int);

    EXPECT_TRUE(CSTL_vector_assign_n(&cstl_vec, type, &copy, 5, cstl_int, alloc))
        << "must return true on success";

    vector_expect_size(5);
    vector_assert_equal();

    // Erase at front:
    real_vec.erase(real_vec.begin());
    CSTL_VectorIter first = CSTL_vector_begin(&cstl_vec, type);
    CSTL_VectorIter pos   = CSTL_vector_erase(&cstl_vec, &copy.move_type, first);
    ASSERT_TRUE(CSTL_vector_iterator_eq(pos, CSTL_vector_begin(&cstl_vec, type)))
        << "erasing the first element should return a compatible iterator";

    vector_expect_size(4);
    vector_assert_equal();

    // Erase at back:
    real_vec.erase(real_vec.end() - 1);
    CSTL_VectorIter last = CSTL_vector_iterator_sub(CSTL_vector_end(&cstl_vec, type), 1);
    CSTL_VectorIter end  = CSTL_vector_erase(&cstl_vec, &copy.move_type, last);
    ASSERT_TRUE(CSTL_vector_iterator_eq(end, CSTL_vector_end(&cstl_vec, type)))
        << "erasing the last element should return a past the end iterator";

    vector_expect_size(3);
    vector_assert_equal();
}

TEST_F(VectorTest, EraseRange) {
    real_vec.assign(5, real_int);

    EXPECT_TRUE(CSTL_vector_assign_n(&cstl_vec, type, &copy, 5, cstl_int, alloc))
        << "must return true on success";

    vector_expect_size(5);
    vector_assert_equal();

    // Erase at `[1, 4)`:
    real_vec.erase(real_vec.begin() + 1, real_vec.end() - 1);
    CSTL_VectorIter first = CSTL_vector_iterator_add(CSTL_vector_begin(&cstl_vec, type), 1);
    CSTL_VectorIter last  = CSTL_vector_iterator_sub(CSTL_vector_end(&cstl_vec, type), 1);
    CSTL_vector_erase_range(&cstl_vec, &copy.move_type, first, last);

    vector_expect_size(2);
    vector_assert_equal();
}

TEST_F(VectorTest, Resize) {
    real_vec.assign(5, real_int);

    EXPECT_TRUE(CSTL_vector_assign_n(&cstl_vec, type, &copy, 5, cstl_int, alloc))
        << "must return true on success";

    vector_expect_size(5);
    vector_assert_equal();

    // Downsize:
    real_vec.resize(3, real_int);
    EXPECT_TRUE(CSTL_vector_resize(&cstl_vec, type, &copy, 3, cstl_int, alloc))
        << "must return true on success";

    vector_expect_size(3);
    vector_assert_equal();

    // Upsize:
    real_vec.resize(10, real_int);
    EXPECT_TRUE(CSTL_vector_resize(&cstl_vec, type, &copy, 10, cstl_int, alloc))
        << "must return true on success";

    vector_expect_size(10);
    vector_assert_equal();
}
