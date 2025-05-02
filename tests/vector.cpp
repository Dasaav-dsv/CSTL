#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

#include "alloc.h"
#include "type.h"
#include "vector.h"

struct TestInt {
    TestInt() : value{0xC0FFEE} {}

    TestInt(uint32_t value) : value{value} {}

    TestInt(const TestInt& other) : value{other.value} {}

    TestInt(TestInt&& other) noexcept : value{other.value} {
        other.value = 0xC0FFEE;
    }

    TestInt& operator=(const TestInt& other) {
        value = other.value;
        return *this;
    }

    TestInt& operator=(TestInt&& other) noexcept {
        value = other.value;
        other.value = 0xC0FFEE;
        return *this;
    }

    operator uint32_t() {
        return value;
    }

    uint32_t value;
};

void copy_from_testint(void* new_instance, const void* src) {
    ASSERT_EQ(0, (uintptr_t)new_instance % alignof(uint32_t))
        << "`new_instance` alignment not upheld";

    ASSERT_EQ(0, (uintptr_t)src % alignof(uint32_t))
        << "`src` alignment not upheld";;

    *reinterpret_cast<uint32_t*>(new_instance) = *reinterpret_cast<const uint32_t*>(src);
}

void move_from_testint(void* new_instance, void* src) {
    ASSERT_EQ(0, (uintptr_t)new_instance % alignof(uint32_t))
        << "`new_instance` alignment not upheld";

    ASSERT_EQ(0, (uintptr_t)src % alignof(uint32_t))
        << "`src` alignment not upheld";;

    uint32_t& value = *reinterpret_cast<uint32_t*>(src);

    EXPECT_EQ(0xDEADBEEF, value)
        << "moved value was not properly initialized";

    *reinterpret_cast<uint32_t*>(new_instance) = value;

    value = 0xC0FFEE;
}

void destroy_testint(void* instance) {
    ASSERT_EQ(0, (uintptr_t)instance % alignof(uint32_t))
        << "`instance` alignment not upheld";

    uint32_t& value = *reinterpret_cast<uint32_t*>(instance);

    EXPECT_TRUE(0xDEADBEEF == value || 0xC0FFEE == value)
        << "destroyed value was not properly initialized";

    value = 0xC0FFEE;
}

class VectorTest : public testing::Test {
protected:
    VectorTest() : real_int{0xDEADBEEF}, cstl_vec{ &val, &type },
        cstl_int{reinterpret_cast<const void*>(&real_int.value)},
        alloc{nullptr}, val{}, type{} {}

    ~VectorTest() {
        CSTL_vector_destroy(cstl_vec, alloc);
    }

    void SetUp() override {
        ASSERT_EQ(CSTL_TypeErr_Ok, CSTL_define_type(
            &type,
            sizeof(uint32_t),
            alignof(uint32_t),
            copy_from_testint,
            move_from_testint,
            destroy_testint
        ));

        CSTL_vector_construct(cstl_vec);
    }

    void vector_expect_size(size_t size) {
        EXPECT_EQ(size, CSTL_vector_size(cstl_vec))
            << "size of vector must be equal to " << size;
    
        CSTL_VectorIter first = CSTL_vector_begin(cstl_vec);
        CSTL_VectorIter last  = CSTL_vector_end(cstl_vec);
    
        EXPECT_EQ(size, CSTL_vector_iterator_distance(first, last))
            << "`[first, last)` must span exactly " << size << " elements";
    }

    void vector_assert_equal() {
        for (size_t i = 0; i < real_vec.size(); ++i) {
            ASSERT_LT(i, CSTL_vector_size(cstl_vec))
                << "vector should be bigger; i=" << i;
            
            uint32_t left  = real_vec[i].value;
            uint32_t right = *(uint32_t*)CSTL_vector_index(cstl_vec, i);

            EXPECT_EQ(left, right) << "must equal if correctly reallocated; i=" << i;
        }
    }

    // Genuine STL vector:
    std::vector<TestInt> real_vec;
    const TestInt real_int;

    // CSTL vector:
    CSTL_VectorCtx cstl_vec;
    const void* cstl_int;
    CSTL_Alloc* alloc;
    CSTL_VectorVal val;
    CSTL_Type type;
};

TEST_F(VectorTest, Default) {
    EXPECT_LT(0, CSTL_vector_max_size(&type))
        << "max vector size must be greater than 0";

    vector_expect_size(0);
}

TEST_F(VectorTest, PushTen) {
    for (int i = 0; i < 10; ++i) {
        ASSERT_TRUE(CSTL_vector_push_const(cstl_vec, cstl_int, alloc))
            << "must return true on success";

            real_vec.push_back(real_int);
    }

    vector_expect_size(10);
    vector_assert_equal();
}

TEST_F(VectorTest, AssignCopies) {
    real_vec.assign(5, real_int);
    EXPECT_TRUE(CSTL_vector_assign(cstl_vec, 5, cstl_int, alloc))
        << "must return true on success";

    vector_expect_size(5);
    vector_assert_equal();

    // Will cause a reallocation:
    real_vec.assign(12, real_int);
    EXPECT_TRUE(CSTL_vector_assign(cstl_vec, 12, cstl_int, alloc))
        << "must return true on success";

    vector_expect_size(12);
    vector_assert_equal();

    // Will reuse storage and resize:
    real_vec.assign(7, real_int);
    EXPECT_TRUE(CSTL_vector_assign(cstl_vec, 7, cstl_int, alloc))
        << "must return true on success";

    vector_expect_size(7);
    vector_assert_equal();

    // Must fail and do nothing (too many elements):
    EXPECT_FALSE(CSTL_vector_assign(cstl_vec, SIZE_MAX, cstl_int, alloc))
        << "must fail due to exceeding `CSTL_vector_max_size(cstl_vec)`";

    vector_expect_size(7);
    vector_assert_equal();

    // Will destroy all elements:
    real_vec.assign(0, real_int);
    EXPECT_TRUE(CSTL_vector_assign(cstl_vec, 0, cstl_int, alloc))
        << "must return true on success";

    vector_expect_size(0);
}

TEST_F(VectorTest, AssignRange) {
    real_vec.assign(5, real_int);
    EXPECT_TRUE(CSTL_vector_assign_continuous_range(cstl_vec, 5, (const void*)real_vec.data(), alloc))
        << "must return true on success";

    vector_expect_size(5);
    vector_assert_equal();

    // Will cause a reallocation:
    real_vec.assign(12, real_int);
    EXPECT_TRUE(CSTL_vector_assign_continuous_range(cstl_vec, 12, (const void*)real_vec.data(), alloc))
        << "must return true on success";

    vector_expect_size(12);
    vector_assert_equal();

    // Will reuse storage and resize:
    real_vec.assign(7, real_int);
    EXPECT_TRUE(CSTL_vector_assign_continuous_range(cstl_vec, 7, (const void*)real_vec.data(), alloc))
        << "must return true on success";

    vector_expect_size(7);
    vector_assert_equal();

    // Will destroy all elements:
    real_vec.assign(0, real_int);
    EXPECT_TRUE(CSTL_vector_assign_continuous_range(cstl_vec, 0, (const void*)real_vec.data(), alloc))
        << "must return true on success";

    vector_expect_size(0);
}

TEST_F(VectorTest, VectorCopy) {
    CSTL_VectorVal other_val;
    CSTL_VectorCtx other_vec = {&other_val, &type};
    CSTL_vector_construct(other_vec);

    EXPECT_TRUE(CSTL_vector_assign(cstl_vec, 5, cstl_int, alloc))
        << "must return true on success";

    EXPECT_TRUE(CSTL_vector_assign(other_vec, 7, cstl_int, alloc))
        << "must return true on success";

    // Will expand storage.
    CSTL_vector_copy(cstl_vec, alloc, other_vec, alloc, false);

    EXPECT_EQ(CSTL_vector_size(cstl_vec), CSTL_vector_size(other_vec))
        << "vectors should have equal size after being copied";

    EXPECT_TRUE(CSTL_vector_assign(other_vec, 5, cstl_int, alloc))
        << "must return true on success";

    // Will reuse storage.
    CSTL_vector_copy(cstl_vec, alloc, other_vec, alloc, false);

    EXPECT_EQ(CSTL_vector_size(cstl_vec), CSTL_vector_size(other_vec))
        << "vectors should have equal size after being copied";

    CSTL_vector_destroy(other_vec, alloc);
}

TEST_F(VectorTest, VectorMove) {
    CSTL_VectorVal other_val;
    CSTL_VectorCtx other_vec = {&other_val, &type};
    CSTL_vector_construct(other_vec);

    EXPECT_TRUE(CSTL_vector_assign(cstl_vec, 5, cstl_int, alloc))
        << "must return true on success";

    EXPECT_TRUE(CSTL_vector_assign(other_vec, 7, cstl_int, alloc))
        << "must return true on success";

    // Will expand storage.
    CSTL_vector_move(cstl_vec, alloc, other_vec, alloc, false);
    size_t left_size = CSTL_vector_size(cstl_vec);
    CSTL_vector_move(other_vec, alloc, cstl_vec, alloc, false);
    size_t right_size = CSTL_vector_size(other_vec);

    EXPECT_EQ(left_size, right_size) << "moving should keep size intact";

    EXPECT_TRUE(CSTL_vector_assign(other_vec, 5, cstl_int, alloc))
        << "must return true on success";

    // Will reuse storage.
    CSTL_vector_move(cstl_vec, alloc, other_vec, alloc, true);
    left_size = CSTL_vector_size(cstl_vec);
    CSTL_vector_move(other_vec, alloc, cstl_vec, alloc, true);
    right_size = CSTL_vector_size(other_vec);

    EXPECT_EQ(left_size, right_size) << "moving should keep size intact";

    CSTL_vector_destroy(other_vec, alloc);
}

TEST_F(VectorTest, CheckedIndex) {
    for (int i = 0; i < 10; ++i) {
        ASSERT_EQ(nullptr, CSTL_vector_const_get_at(cstl_vec, i))
            << "the end of the vector must not have accesible elements";

        ASSERT_TRUE(CSTL_vector_push_const(cstl_vec, cstl_int, alloc))
            << "must return true on success";

        real_vec.push_back(real_int);
    }

    vector_expect_size(10);
    vector_assert_equal();

    for (int i = 0; i < 10; ++i) {
        ASSERT_NE(nullptr, CSTL_vector_const_get_at(cstl_vec, i))
            << "all vector elements must be accesible";
    }
}

TEST_F(VectorTest, FrontAndBack) {
    ASSERT_TRUE(CSTL_vector_push_const(cstl_vec, cstl_int, alloc))
        << "must return true on success";

    ASSERT_EQ(CSTL_vector_const_front(cstl_vec), CSTL_vector_const_back(cstl_vec))
        << "must equal when there is only one element";

    ASSERT_TRUE(CSTL_vector_push_const(cstl_vec, cstl_int, alloc))
        << "must return true on success";

    ASSERT_NE(CSTL_vector_const_front(cstl_vec), CSTL_vector_const_back(cstl_vec))
        << "must differ when there is more than one element";
}

TEST_F(VectorTest, DataRange) {
    for (int i = 0; i < 10; ++i) {
        // The range `[data(), data() + size)` must be valid.
        const uint32_t* first = (const uint32_t*)CSTL_vector_data(cstl_vec);
        const uint32_t* last  = first + CSTL_vector_size(cstl_vec);

        real_vec.assign(first, last);

        ASSERT_TRUE(CSTL_vector_push_const(cstl_vec, cstl_int, alloc))
            << "must return true on success";
    }

    vector_expect_size(10);
    vector_assert_equal();
}

TEST_F(VectorTest, Reserve) {
    CSTL_vector_reserve(cstl_vec, 10, alloc);

    CSTL_VectorIter first = CSTL_vector_begin(cstl_vec);

    for (int i = 0; i < 10; ++i) {
        ASSERT_TRUE(CSTL_vector_push_const(cstl_vec, cstl_int, alloc))
            << "must return true on success";

        ASSERT_TRUE(CSTL_vector_iterator_eq(first, CSTL_vector_begin(cstl_vec)))
            << "not exceeding reserved capacity should not invalidate iterators";
    }
}

TEST_F(VectorTest, Clear) {
    real_vec.assign(5, real_int);
    EXPECT_TRUE(CSTL_vector_assign(cstl_vec, 5, cstl_int, alloc))
        << "must return true on success";

    real_vec.clear();
    CSTL_vector_clear(cstl_vec);

    vector_expect_size(0);
    vector_assert_equal();
}

TEST_F(VectorTest, EmplaceInsert) {
    real_vec.assign(10, real_int);

    for (int i = 0; i < 3; ++i) {
        CSTL_VectorIter first = CSTL_vector_begin(cstl_vec);
        CSTL_VectorIter pos   = CSTL_vector_emplace_const(cstl_vec, first, cstl_int, alloc);
        ASSERT_FALSE(CSTL_vector_iterator_eq(pos, CSTL_vector_end(cstl_vec)))
            << "emplace must return a dereferenceable iterator";
    }

    for (int i = 0; i < 3; ++i) {
        CSTL_VectorIter last  = CSTL_vector_end(cstl_vec);
        CSTL_VectorIter pos   = CSTL_vector_emplace_const(cstl_vec, last, cstl_int, alloc);
        ASSERT_FALSE(CSTL_vector_iterator_eq(pos, CSTL_vector_end(cstl_vec)))
            << "emplace must return a dereferenceable iterator";
    }

    CSTL_VectorIter first = CSTL_vector_begin(cstl_vec);
    CSTL_VectorIter last  = CSTL_vector_end(cstl_vec);
    CSTL_VectorIter mid   = CSTL_vector_iterator_add(first, CSTL_vector_iterator_distance(first, last) / 2);
    CSTL_VectorIter pos   = CSTL_vector_insert(cstl_vec, mid, 4, cstl_int, alloc);
    
    ASSERT_FALSE(CSTL_vector_iterator_eq(pos, CSTL_vector_end(cstl_vec)))
        << "insert must return a dereferenceable iterator";

    vector_expect_size(10);
    vector_assert_equal();
}

TEST_F(VectorTest, Erase) {
    real_vec.assign(5, real_int);

    EXPECT_TRUE(CSTL_vector_assign(cstl_vec, 5, cstl_int, alloc))
        << "must return true on success";

    vector_expect_size(5);
    vector_assert_equal();

    // Erase at front:
    real_vec.erase(real_vec.begin());
    CSTL_VectorIter first = CSTL_vector_begin(cstl_vec);
    CSTL_VectorIter pos   = CSTL_vector_erase(cstl_vec, first);
    ASSERT_TRUE(CSTL_vector_iterator_eq(pos, CSTL_vector_begin(cstl_vec)))
        << "erasing the first element should return a compatible iterator";

    vector_expect_size(4);
    vector_assert_equal();

    // Erase at back:
    real_vec.erase(real_vec.end() - 1);
    CSTL_VectorIter last = CSTL_vector_iterator_sub(CSTL_vector_end(cstl_vec), 1);
    CSTL_VectorIter end  = CSTL_vector_erase(cstl_vec, last);
    ASSERT_TRUE(CSTL_vector_iterator_eq(end, CSTL_vector_end(cstl_vec)))
        << "erasing the last element should return a past the end iterator";

    vector_expect_size(3);
    vector_assert_equal();
}

TEST_F(VectorTest, EraseRange) {
    real_vec.assign(5, real_int);

    EXPECT_TRUE(CSTL_vector_assign(cstl_vec, 5, cstl_int, alloc))
        << "must return true on success";

    vector_expect_size(5);
    vector_assert_equal();

    // Erase at `[1, 4)`:
    real_vec.erase(real_vec.begin() + 1, real_vec.end() - 1);
    CSTL_VectorIter first = CSTL_vector_iterator_add(CSTL_vector_begin(cstl_vec), 1);
    CSTL_VectorIter last  = CSTL_vector_iterator_sub(CSTL_vector_end(cstl_vec), 1);
    CSTL_vector_erase_range(cstl_vec, first, last);

    vector_expect_size(2);
    vector_assert_equal();
}

TEST_F(VectorTest, Resize) {
    real_vec.assign(5, real_int);

    EXPECT_TRUE(CSTL_vector_assign(cstl_vec, 5, cstl_int, alloc))
        << "must return true on success";

    vector_expect_size(5);
    vector_assert_equal();

    // Downsize:
    real_vec.resize(3, real_int);
    EXPECT_TRUE(CSTL_vector_resize(cstl_vec, 3, cstl_int, alloc))
        << "must return true on success";

    vector_expect_size(3);
    vector_assert_equal();

    // Upsize:
    real_vec.resize(10, real_int);
    EXPECT_TRUE(CSTL_vector_resize(cstl_vec, 10, cstl_int, alloc))
        << "must return true on success";

    vector_expect_size(10);
    vector_assert_equal();
}
