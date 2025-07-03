#include <gtest/gtest.h>

#include "alloc.h"

struct TestAllocator
{
    CSTL_Alloc cstl_alloc;
    int id;
};

TestAllocator create_test_allocator(int id)
{
    TestAllocator ta;
    ta.id = id;
    ta.cstl_alloc.opaque = &ta;
    ta.cstl_alloc.aligned_alloc = NULL;
    ta.cstl_alloc.aligned_free = NULL;
    return ta;
}

class AllocTest : public testing::Test
{
protected:
    TestAllocator alloc1_obj = create_test_allocator(1);
    TestAllocator alloc2_obj = create_test_allocator(2);
    TestAllocator alloc1_copy_obj = create_test_allocator(1);

    CSTL_Alloc *alloc1 = &alloc1_obj.cstl_alloc;
    CSTL_Alloc *alloc2 = &alloc2_obj.cstl_alloc;
    CSTL_Alloc *alloc1_copy = &alloc1_copy_obj.cstl_alloc;
};

TEST_F(AllocTest, IsEqual)
{
    EXPECT_TRUE(CSTL_alloc_is_equal(alloc1, alloc1));

    EXPECT_FALSE(CSTL_alloc_is_equal(alloc1, alloc1_copy));
    EXPECT_FALSE(CSTL_alloc_is_equal(alloc1, alloc2));

    EXPECT_FALSE(CSTL_alloc_is_equal(alloc1, nullptr));
    EXPECT_FALSE(CSTL_alloc_is_equal(nullptr, alloc2));

    EXPECT_TRUE(CSTL_alloc_is_equal(nullptr, nullptr));
}
