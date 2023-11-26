#include <gtest/gtest.h>
#include <cstring>
#include <fstream>
#include "sum.hpp"
#include "util.hpp"

//---------------------------------------------------------------------------
TEST(SumTest, FindFirst)
/// test find_first()
{
    constexpr uint64_t bar_pattern = 0x7C7C7C7C7C7C7C7Cull;
    auto pos = find_first(bar_pattern, "|        ", 1);
    ASSERT_EQ(pos, 0);
    pos = find_first(bar_pattern, "   |        ", 4);
    ASSERT_EQ(pos, 3);
    pos = find_first(bar_pattern, "       |        ", 8);
    ASSERT_EQ(pos, 7);
    pos = find_first(bar_pattern, "           |        ", 12);
    ASSERT_EQ(pos, 11);
    pos = find_first(bar_pattern, "               |        ", 16);
    ASSERT_EQ(pos, 15);
}
//---------------------------------------------------------------------------
TEST(SumTest, ReadNumeric)
/// test find_first()
{
    auto value = read_numeric("29312.32        ", 8);
    ASSERT_EQ(value, 2931232);
}
//---------------------------------------------------------------------------
TEST(SumTest, SumExtendedPrice)
/// test sum_extendedprice()
{
    auto lineitem_file = getDir(__FILE__) + "/data/tpch/sf0_001/lineitem.tbl";
    std::ifstream in(lineitem_file);
    std::string contents((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());
    size_t full_size = contents.size() + 8;
    char* buffer = new char[full_size]();
    std::unique_ptr<char[]> guard(buffer);
    std::memcpy(buffer, contents.c_str(), contents.size());

    int64_t result;
    sum_extendedprice<true>(buffer, buffer, contents.size(), &result);
    ASSERT_EQ(result, 15277439838ul);
}
//---------------------------------------------------------------------------
