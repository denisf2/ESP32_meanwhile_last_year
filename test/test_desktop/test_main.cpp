#include "unity.h"

auto setUp(void) -> void
{
    // set stuff up here
}

auto tearDown(void) -> void
{
    // clean stuff up here
}

auto test_example(void) -> void
{
    TEST_ASSERT_EQUAL(4, 2 + 2);
}

auto RUN_UNITY_TESTS() -> void
{
    UNITY_BEGIN();
    RUN_TEST(test_example);
    UNITY_END();
}

auto main() -> int
{
    RUN_UNITY_TESTS();
    return 0;
}
