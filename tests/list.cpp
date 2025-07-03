#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <list>
#include <memory>

#include "alloc.h"
#include "list.h"
#include "type.h"

static void* test_aligned_alloc(void* opaque, size_t size, size_t alignment) {
    (void)opaque;
#if defined(_CRTALLOCATOR) || defined(_CRT_ALLOCATION_DEFINED)
    return _aligned_malloc(size, alignment);
#else
    return aligned_alloc(alignment, size);
#endif
}

static void test_aligned_free(void* opaque, void* memory, size_t size, size_t alignment) {
    (void)opaque;
    (void)size;
    (void)alignment;
#if defined(_CRTALLOCATOR) || defined(_CRT_ALLOCATION_DEFINED)
    _aligned_free(memory);
#else
    free(memory);
#endif
}

struct TestAllocator {
    CSTL_Alloc cstl_alloc;
};

TestAllocator create_test_allocator() {
    TestAllocator ta;
    ta.cstl_alloc.opaque        = &ta;
    ta.cstl_alloc.aligned_alloc = test_aligned_alloc;
    ta.cstl_alloc.aligned_free  = test_aligned_free;
    return ta;
}

struct TestInt {
    TestInt() : value{new uint32_t{0xC0FFEE}} {
    }
    TestInt(uint32_t value) : value{new uint32_t{value}} {
    }
    TestInt(const TestInt& other) : value{new uint32_t{*other.value}} {
    }
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
        value        = new uint32_t{*other.value};
        *other.value = 0xC0FFEE;
        return *this;
    }
    friend bool operator==(const TestInt& lhs, const TestInt& rhs) {
        return *lhs.value == *rhs.value;
    }
    uint32_t* value;
};

static void destroy_testint(void* _first, void* _last) {
    auto first = reinterpret_cast<TestInt*>(_first);
    auto last  = reinterpret_cast<TestInt*>(_last);
    std::destroy(first, last);
}

static void move_testint(void* _first, void* _last, void* _dest) {
    auto first = reinterpret_cast<TestInt*>(_first);
    auto last  = reinterpret_cast<TestInt*>(_last);
    auto dest  = reinterpret_cast<TestInt*>(_dest);
    std::uninitialized_move(first, last, dest);
}

static void copy_testint(const void* _first, const void* _last, void* _dest) {
    auto first = reinterpret_cast<const TestInt*>(_first);
    auto last  = reinterpret_cast<const TestInt*>(_last);
    auto dest  = reinterpret_cast<TestInt*>(_dest);
    std::uninitialized_copy(first, last, dest);
}

static void fill_testint(void* _first, void* _last, const void* _value) {
    const auto& value = *reinterpret_cast<const TestInt*>(_value);
    auto first        = reinterpret_cast<TestInt*>(_first);
    auto last         = reinterpret_cast<TestInt*>(_last);
    std::uninitialized_fill(first, last, value);
}

class ListTest : public testing::Test {
  protected:
    ListTest() : real_list{}, real_int{0xDEADBEEF}, cstl_list_object{}, cstl_list{&cstl_list_object}, copy{}, cstl_int{reinterpret_cast<const void*>(&real_int)}, alloc{nullptr}, type{} {
    }

    ~ListTest() {
        CSTL_list_destroy(cstl_list, type, &copy.move_type.drop_type, alloc);
    }

    void SetUp() override {
        type = CSTL_define_type(sizeof(TestInt), alignof(TestInt));
        ASSERT_NE(nullptr, type);

        copy = {{{&destroy_testint}, &move_testint}, &copy_testint, &fill_testint};

        CSTL_list_construct(cstl_list, alloc);
    }

    void list_expect_size(size_t size) {
        EXPECT_EQ(size, CSTL_list_size(cstl_list)) << "size of list must be equal to " << size;

        CSTL_ListIter first = CSTL_list_begin(cstl_list);
        CSTL_ListIter last  = CSTL_list_end(cstl_list);

        EXPECT_EQ(size, CSTL_list_iterator_distance(first, last)) << "`[first, last)` must span exactly " << size << " elements";
    }

    void list_assert_equal() {
        ASSERT_EQ(real_list.size(), CSTL_list_size(cstl_list));

        auto real_it           = real_list.begin();
        CSTL_ListIter cstl_it  = CSTL_list_begin(cstl_list);
        CSTL_ListIter cstl_end = CSTL_list_end(cstl_list);

        for (; real_it != real_list.end(); ++real_it, cstl_it = CSTL_list_iterator_add(cstl_it, 1)) {
            ASSERT_FALSE(CSTL_list_iterator_eq(cstl_it, cstl_end));
            const TestInt& left  = *real_it;
            const TestInt& right = *static_cast<const TestInt*>(CSTL_list_iterator_deref(cstl_it));
            EXPECT_EQ(left, right);
        }
    }

    std::list<TestInt> real_list;
    const TestInt real_int;

    CSTL_ListVal cstl_list_object;
    CSTL_ListRef cstl_list;
    CSTL_CopyType copy;
    const void* cstl_int;
    CSTL_Alloc* alloc;
    CSTL_Type type;
};

TEST_F(ListTest, MemoryLayout) {
    EXPECT_EQ(sizeof(CSTL_Alloc*) + sizeof(cstl_list_object), sizeof(std::list<TestInt>));
}

TEST_F(ListTest, Default) {
    EXPECT_LT(0, CSTL_list_max_size(type)) << "max list size must be greater than 0";
    list_expect_size(0);
    EXPECT_TRUE(CSTL_list_empty(cstl_list));
}

TEST_F(ListTest, PushTen) {
    for (int i = 0; i < 10; ++i) {
        ASSERT_TRUE(CSTL_list_copy_push_back(cstl_list, type, &copy, cstl_int, alloc)) << "must return true on success";
        real_list.push_back(real_int);
    }

    list_expect_size(10);
    list_assert_equal();
}

TEST_F(ListTest, PushFrontTen) {
    for (int i = 0; i < 10; ++i) {
        ASSERT_TRUE(CSTL_list_copy_push_front(cstl_list, type, &copy, cstl_int, alloc)) << "must return true on success";
        real_list.push_front(real_int);
    }

    list_expect_size(10);
    list_assert_equal();
}

TEST_F(ListTest, PopBackAndFront) {
    for (int i = 0; i < 10; ++i) {
        CSTL_list_copy_push_back(cstl_list, type, &copy, cstl_int, alloc);
        real_list.push_back(real_int);
    }

    for (int i = 0; i < 5; ++i) {
        CSTL_list_pop_back(cstl_list, type, &copy.move_type.drop_type, alloc);
        real_list.pop_back();
    }
    list_expect_size(5);
    list_assert_equal();

    for (int i = 0; i < 5; ++i) {
        CSTL_list_pop_front(cstl_list, type, &copy.move_type.drop_type, alloc);
        real_list.pop_front();
    }
    list_expect_size(0);
    list_assert_equal();
}

TEST_F(ListTest, AssignCopies) {
    real_list.assign(5, real_int);
    EXPECT_TRUE(CSTL_list_assign_n(cstl_list, type, &copy, 5, cstl_int, alloc)) << "must return true on success";
    list_expect_size(5);
    list_assert_equal();

    real_list.assign(12, real_int);
    EXPECT_TRUE(CSTL_list_assign_n(cstl_list, type, &copy, 12, cstl_int, alloc)) << "must return true on success";
    list_expect_size(12);
    list_assert_equal();

    real_list.assign(7, real_int);
    EXPECT_TRUE(CSTL_list_assign_n(cstl_list, type, &copy, 7, cstl_int, alloc)) << "must return true on success";
    list_expect_size(7);
    list_assert_equal();

    EXPECT_FALSE(CSTL_list_assign_n(cstl_list, type, &copy, SIZE_MAX, cstl_int, alloc)) << "must fail due to exceeding max_size";
    list_expect_size(7);
    list_assert_equal();

    real_list.assign(0, real_int);
    EXPECT_TRUE(CSTL_list_assign_n(cstl_list, type, &copy, 0, cstl_int, alloc)) << "must return true on success";
    list_expect_size(0);
}

TEST_F(ListTest, FrontAndBack) {
    const TestInt val1(111);
    real_list.push_back(val1);
    ASSERT_TRUE(CSTL_list_copy_push_back(cstl_list, type, &copy, &val1, alloc));
    list_assert_equal();
    EXPECT_EQ(*static_cast<const TestInt*>(CSTL_list_const_front(cstl_list)), real_list.front());
    EXPECT_EQ(*static_cast<const TestInt*>(CSTL_list_const_back(cstl_list)), real_list.back());

    const TestInt val2(222);
    real_list.push_back(val2);
    ASSERT_TRUE(CSTL_list_copy_push_back(cstl_list, type, &copy, &val2, alloc));
    list_assert_equal();
    EXPECT_EQ(*static_cast<const TestInt*>(CSTL_list_const_front(cstl_list)), real_list.front());
    EXPECT_EQ(*static_cast<const TestInt*>(CSTL_list_const_back(cstl_list)), real_list.back());
    EXPECT_NE(*static_cast<const TestInt*>(CSTL_list_const_front(cstl_list)), *static_cast<const TestInt*>(CSTL_list_const_back(cstl_list)));
}

TEST_F(ListTest, Clear) {
    real_list.assign(5, real_int);
    CSTL_list_assign_n(cstl_list, type, &copy, 5, cstl_int, alloc);

    real_list.clear();
    CSTL_list_clear(cstl_list, type, &copy.move_type.drop_type, alloc);

    list_expect_size(0);
    list_assert_equal();
}

TEST_F(ListTest, Insert) {
    for (int i = 0; i < 3; ++i) {
        CSTL_ListIter first = CSTL_list_begin(cstl_list);
        CSTL_ListIter pos   = CSTL_list_copy_insert(cstl_list, type, &copy, first, cstl_int, alloc);
        ASSERT_FALSE(CSTL_list_iterator_eq(pos, CSTL_list_end(cstl_list)));
    }
    real_list.assign(3, real_int);
    list_assert_equal();

    for (int i = 0; i < 3; ++i) {
        CSTL_ListIter last = CSTL_list_end(cstl_list);
        CSTL_ListIter pos  = CSTL_list_copy_insert(cstl_list, type, &copy, last, cstl_int, alloc);
        ASSERT_FALSE(CSTL_list_iterator_eq(pos, CSTL_list_end(cstl_list)));
    }
    real_list.insert(real_list.end(), 3, real_int);
    list_assert_equal();

    CSTL_ListIter first = CSTL_list_begin(cstl_list);
    CSTL_ListIter mid   = CSTL_list_iterator_add(first, 3);
    CSTL_ListIter pos   = CSTL_list_insert_n(cstl_list, type, &copy, mid, 4, cstl_int, alloc);
    ASSERT_FALSE(CSTL_list_iterator_eq(pos, CSTL_list_end(cstl_list)));

    auto real_mid = real_list.begin();
    std::advance(real_mid, 3);
    real_list.insert(real_mid, 4, real_int);

    list_expect_size(10);
    list_assert_equal();
}

TEST_F(ListTest, Erase) {
    real_list.assign(5, real_int);
    CSTL_list_assign_n(cstl_list, type, &copy, 5, cstl_int, alloc);
    list_assert_equal();

    real_list.erase(real_list.begin());
    CSTL_ListIter first = CSTL_list_begin(cstl_list);
    CSTL_ListIter pos   = CSTL_list_erase(cstl_list, type, &copy.move_type.drop_type, alloc, first);
    ASSERT_TRUE(CSTL_list_iterator_eq(pos, CSTL_list_begin(cstl_list)));
    list_expect_size(4);
    list_assert_equal();

    real_list.erase(--real_list.end());
    CSTL_ListIter last = CSTL_list_iterator_sub(CSTL_list_end(cstl_list), 1);
    CSTL_ListIter end  = CSTL_list_erase(cstl_list, type, &copy.move_type.drop_type, alloc, last);
    ASSERT_TRUE(CSTL_list_iterator_eq(end, CSTL_list_end(cstl_list)));
    list_expect_size(3);
    list_assert_equal();
}

TEST_F(ListTest, EraseRange) {
    real_list.assign(5, real_int);
    CSTL_list_assign_n(cstl_list, type, &copy, 5, cstl_int, alloc);

    auto real_first = std::next(real_list.begin(), 1);
    auto real_last  = std::next(real_list.begin(), 4);
    real_list.erase(real_first, real_last);

    CSTL_ListIter first = CSTL_list_iterator_add(CSTL_list_begin(cstl_list), 1);
    CSTL_ListIter last  = CSTL_list_iterator_add(CSTL_list_begin(cstl_list), 4);
    CSTL_list_erase_range(cstl_list, type, &copy.move_type.drop_type, alloc, first, last);

    list_expect_size(2);
    list_assert_equal();
}

TEST_F(ListTest, Resize) {
    real_list.assign(5, real_int);
    CSTL_list_assign_n(cstl_list, type, &copy, 5, cstl_int, alloc);

    real_list.resize(3, real_int);
    EXPECT_TRUE(CSTL_list_resize(cstl_list, type, &copy, 3, cstl_int, alloc));
    list_expect_size(3);
    list_assert_equal();

    real_list.resize(10, real_int);
    EXPECT_TRUE(CSTL_list_resize(cstl_list, type, &copy, 10, cstl_int, alloc));
    list_expect_size(10);
    list_assert_equal();
}

TEST_F(ListTest, MovePushBack) {
    TestInt to_move{12345};
    real_list.push_back(std::move(to_move));

    TestInt cstl_to_move{12345};
    ASSERT_TRUE(CSTL_list_move_push_back(cstl_list, type, &copy.move_type, &cstl_to_move, alloc));

    list_expect_size(1);
    list_assert_equal();

    EXPECT_EQ(*cstl_to_move.value, 0xC0FFEE);
}

TEST_F(ListTest, MoveInsert) {
    real_list.assign(2, real_int);
    CSTL_list_assign_n(cstl_list, type, &copy, 2, cstl_int, alloc);

    TestInt to_move_front{111};
    real_list.insert(real_list.begin(), std::move(to_move_front));

    TestInt cstl_to_move_front{111};
    CSTL_ListIter cstl_begin = CSTL_list_begin(cstl_list);
    CSTL_list_move_insert(cstl_list, type, &copy.move_type, cstl_begin, &cstl_to_move_front, alloc);

    list_assert_equal();
    EXPECT_EQ(*cstl_to_move_front.value, 0xC0FFEE);

    TestInt to_move_mid{222};
    auto real_mid = std::next(real_list.begin(), 2);
    real_list.insert(real_mid, std::move(to_move_mid));

    TestInt cstl_to_move_mid{222};
    CSTL_ListIter cstl_mid = CSTL_list_iterator_add(CSTL_list_begin(cstl_list), 2);
    CSTL_list_move_insert(cstl_list, type, &copy.move_type, cstl_mid, &cstl_to_move_mid, alloc);

    list_assert_equal();
    EXPECT_EQ(*cstl_to_move_mid.value, 0xC0FFEE);
}

TEST_F(ListTest, CopyAssign) {
    CSTL_ListVal other_list_obj;
    CSTL_ListRef other_list = &other_list_obj;
    CSTL_list_construct(other_list, alloc);
    CSTL_list_assign_n(other_list, type, &copy, 5, cstl_int, alloc);

    const TestInt val_copy(999);
    real_list.assign(10, val_copy);
    CSTL_list_assign_n(cstl_list, type, &copy, 10, &val_copy, alloc);

    std::list<TestInt> other_real_list;
    other_real_list.assign(5, real_int);
    real_list = other_real_list;
    ASSERT_TRUE(CSTL_list_copy_assign(cstl_list, type, &copy, other_list, alloc, alloc, false));

    list_assert_equal();

    ASSERT_EQ(CSTL_list_size(other_list), 5);
    const TestInt& val = *static_cast<const TestInt*>(CSTL_list_const_front(other_list));
    EXPECT_EQ(val, real_int);

    CSTL_list_destroy(other_list, type, &copy.move_type.drop_type, alloc);
}

TEST_F(ListTest, MoveAssign) {
    CSTL_ListVal other_list_obj;
    CSTL_ListRef other_list = &other_list_obj;
    CSTL_list_construct(other_list, alloc);
    CSTL_list_assign_n(other_list, type, &copy, 5, cstl_int, alloc);

    const TestInt val_move(999);
    real_list.assign(10, val_move);
    CSTL_list_assign_n(cstl_list, type, &copy, 10, &val_move, alloc);

    std::list<TestInt> other_real_list;
    other_real_list.assign(5, real_int);
    real_list = std::move(other_real_list);
    ASSERT_TRUE(CSTL_list_move_assign(cstl_list, type, &copy.move_type, other_list, alloc, alloc, false));

    list_assert_equal();
    EXPECT_EQ(CSTL_list_size(cstl_list), 5);

    EXPECT_TRUE(CSTL_list_empty(other_list));

    CSTL_list_destroy(other_list, type, &copy.move_type.drop_type, alloc);
}

TEST_F(ListTest, MoveAssignFromEmptyAndPush) {
    real_list.assign(5, real_int);
    CSTL_list_assign_n(cstl_list, type, &copy, 5, cstl_int, alloc);

    CSTL_ListVal other_list_obj;
    CSTL_ListRef other_list = &other_list_obj;
    CSTL_list_construct(other_list, alloc);

    real_list = std::list<TestInt>();
    ASSERT_TRUE(CSTL_list_move_assign(cstl_list, type, &copy.move_type, other_list, alloc, alloc, false));

    list_expect_size(0);

    ASSERT_TRUE(CSTL_list_copy_push_back(cstl_list, type, &copy, cstl_int, alloc));
    real_list.push_back(real_int);

    list_expect_size(1);
    list_assert_equal();

    CSTL_list_destroy(other_list, type, &copy.move_type.drop_type, alloc);
}

TEST_F(ListTest, Swap) {
    const TestInt val1(111);
    real_list.assign(3, val1);
    CSTL_list_assign_n(cstl_list, type, &copy, 3, &val1, alloc);

    const TestInt val2(222);
    std::list<TestInt> other_real_list;
    other_real_list.assign(7, val2);
    CSTL_ListVal other_list_obj;
    CSTL_ListRef other_list = &other_list_obj;
    CSTL_list_construct(other_list, alloc);
    CSTL_list_assign_n(other_list, type, &copy, 7, &val2, alloc);

    real_list.swap(other_real_list);
    CSTL_list_swap(cstl_list, other_list);

    list_assert_equal();

    ASSERT_EQ(other_real_list.size(), CSTL_list_size(other_list));
    auto real_it          = other_real_list.begin();
    CSTL_ListIter cstl_it = CSTL_list_begin(other_list);
    for (; real_it != other_real_list.end(); ++real_it, cstl_it = CSTL_list_iterator_add(cstl_it, 1)) {
        EXPECT_EQ(*real_it, *static_cast<const TestInt*>(CSTL_list_iterator_deref(cstl_it)));
    }

    CSTL_list_destroy(other_list, type, &copy.move_type.drop_type, alloc);
}

TEST_F(ListTest, CopyAssignWithAllocators) {
    TestAllocator alloc1_obj = create_test_allocator();
    TestAllocator alloc2_obj = create_test_allocator();
    CSTL_Alloc* alloc1       = &alloc1_obj.cstl_alloc;
    CSTL_Alloc* alloc2       = &alloc2_obj.cstl_alloc;

    std::list<TestInt> other_real_list;
    other_real_list.assign(5, real_int);
    CSTL_ListVal other_list_obj;
    CSTL_ListRef other_list = &other_list_obj;
    CSTL_list_construct(other_list, alloc2);
    CSTL_list_assign_n(other_list, type, &copy, 5, cstl_int, alloc2);

    const TestInt val(999);
    real_list.assign(10, val);
    CSTL_list_assign_n(cstl_list, type, &copy, 10, &val, alloc1);

    real_list = other_real_list;
    CSTL_list_copy_assign(cstl_list, type, &copy, other_list, alloc1, alloc2, true);
    EXPECT_TRUE(CSTL_alloc_is_equal(alloc1, alloc2)) << "Allocator should have been copied";
    list_assert_equal();

    TestAllocator new_alloc1_obj = create_test_allocator();
    alloc1                       = &new_alloc1_obj.cstl_alloc;
    real_list.assign(10, val);
    CSTL_list_assign_n(cstl_list, type, &copy, 10, &val, alloc1);

    real_list = other_real_list;
    CSTL_list_copy_assign(cstl_list, type, &copy, other_list, alloc1, alloc2, false);
    EXPECT_FALSE(CSTL_alloc_is_equal(alloc1, alloc2)) << "Allocator should NOT have been copied";
    list_assert_equal();

    CSTL_list_destroy(other_list, type, &copy.move_type.drop_type, alloc2);
}

TEST_F(ListTest, MoveAssignWithAllocators) {
    TestAllocator alloc1_obj = create_test_allocator();
    TestAllocator alloc2_obj = create_test_allocator();
    CSTL_Alloc* alloc1       = &alloc1_obj.cstl_alloc;
    CSTL_Alloc* alloc2       = &alloc2_obj.cstl_alloc;

    std::list<TestInt> other_real_list;
    other_real_list.assign(5, real_int);
    CSTL_ListVal other_list_obj;
    CSTL_ListRef other_list = &other_list_obj;
    CSTL_list_construct(other_list, alloc2);
    CSTL_list_assign_n(other_list, type, &copy, 5, cstl_int, alloc2);

    const TestInt val(999);
    real_list.assign(10, val);
    CSTL_list_assign_n(cstl_list, type, &copy, 10, &val, alloc1);

    real_list = std::move(other_real_list);
    CSTL_list_move_assign(cstl_list, type, &copy.move_type, other_list, alloc1, alloc2, false);
    EXPECT_FALSE(CSTL_alloc_is_equal(alloc1, alloc2)) << "Allocator should NOT have been copied";
    list_assert_equal();
    EXPECT_TRUE(CSTL_list_empty(other_list)) << "Source should be empty after per-element move";

    other_real_list.assign(5, real_int);
    CSTL_list_assign_n(other_list, type, &copy, 5, cstl_int, alloc2);
    TestAllocator new_alloc1_obj = create_test_allocator();
    alloc1                       = &new_alloc1_obj.cstl_alloc;
    real_list.assign(10, val);
    CSTL_list_assign_n(cstl_list, type, &copy, 10, &val, alloc1);

    real_list = std::move(other_real_list);
    CSTL_list_move_assign(cstl_list, type, &copy.move_type, other_list, alloc1, alloc2, true);
    EXPECT_TRUE(CSTL_alloc_is_equal(alloc1, alloc2)) << "Allocator should have been copied";
    list_assert_equal();
    EXPECT_TRUE(CSTL_list_empty(other_list)) << "Source should be empty after memory steal";

    CSTL_list_destroy(other_list, type, &copy.move_type.drop_type, alloc2);
}
