//-----------------------------------------------------------------------------
// Include statements
//-----------------------------------------------------------------------------
#include "sac_io.hpp"
// Catch2 Standard headers
#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
// Standard Library stuff, https://en.cppreference.com/w/cpp/standard_library
#include <bitset>
#include <iomanip>
#include <limits>
#include <sstream>
//-----------------------------------------------------------------------------
// End Include statements
//-----------------------------------------------------------------------------

TEST_CASE("Binary Conversion")
{
    SECTION("Booleans")
    {
        BENCHMARK("Bool->Binary False") 
        { return SAC::bool_to_binary(SAC::unset_bool); };
        BENCHMARK("Bool->Binary->Bool False")
        { return SAC::binary_to_bool(SAC::bool_to_binary(SAC::unset_bool)); };
    }
    SECTION("Integers")
    {
        SECTION("Zero")
        {
            BENCHMARK("Int->Binary 0") { return SAC::int_to_binary(0); };
            BENCHMARK("Int->Binary->Int 0")
            { return SAC::binary_to_int(SAC::int_to_binary(0)); };
        }
        SECTION("Negative")
        {
            BENCHMARK("Int->Binary INT_MIN") { return SAC::int_to_binary(INT_MIN); };
            BENCHMARK("Int->Binary->Int INT_MIN")
            { return SAC::binary_to_int(SAC::int_to_binary(INT_MIN)); };
        }
        SECTION("Positive")
        {
            BENCHMARK("Int->Binary INT_MAX") { return SAC::int_to_binary(INT_MAX); };
            BENCHMARK("Int->Binary->Int INT_MAX") 
            { return SAC::binary_to_int(SAC::int_to_binary(INT_MAX)); };
        }
    }
    SECTION("Floats")
    {
        SECTION("Zero")
        {
            BENCHMARK("Float->Binary 0.0f") { return SAC::float_to_binary(0.0f); };
            BENCHMARK("Float->Binary->Float 0.0f")
            { return SAC::binary_to_float(SAC::float_to_binary(0.0f)); };
        }
        SECTION("Negative")
        {
            BENCHMARK("Float->Binary->Float std::numeric_limits<float>::lowest()")
            { return SAC::binary_to_float(SAC::float_to_binary(std::numeric_limits<float>::lowest())); };
            float neg_epsilon{-std::numeric_limits<float>::epsilon()};
            std::ostringstream oss{};
            oss << std::setprecision(std::numeric_limits<float>::max_digits10) << neg_epsilon;
            std::string s_neg_epsilon{oss.str()};
            CAPTURE(s_neg_epsilon);
            BENCHMARK("Float->Binary->Float negative std::numeric_limits<float>::epsilon()")
            { return SAC::binary_to_float(SAC::float_to_binary(neg_epsilon)); };
        }
        SECTION("Positive")
        {
            BENCHMARK("Float->Binary->Float std::numeric_limits<float>::max()")
            { return SAC::binary_to_float(SAC::float_to_binary(std::numeric_limits<float>::max())); };
            float epsilon{std::numeric_limits<float>::epsilon()};
            std::ostringstream oss;
            oss << std::setprecision(std::numeric_limits<float>::max_digits10) << epsilon;
            std::string s_epsilon{oss.str()};
            CAPTURE(s_epsilon);
            BENCHMARK("Float->Binary->Float std::numeric_limits<float>::epsilon()")
            { return SAC::binary_to_float(SAC::float_to_binary(epsilon)); };
        }
    }
    SECTION("Doubles")
    {
        SECTION("Zero")
        {
            BENCHMARK("Double->Binary->Double 0.0")
            { return SAC::binary_to_double(SAC::double_to_binary(0.0)); };
        }
        SECTION("Negative")
        {
            BENCHMARK("Double->Binary->Double std::numeric_limits<double>::lowest()")
            { return SAC::binary_to_double(SAC::double_to_binary(std::numeric_limits<double>::lowest())); };
            float neg_epsilon{-std::numeric_limits<double>::epsilon()};
            std::ostringstream oss;
            oss << std::setprecision(std::numeric_limits<double>::max_digits10) << neg_epsilon;
            std::string s_neg_epsilon{oss.str()};
            CAPTURE(s_neg_epsilon);
            BENCHMARK("Double->Binary->Double negative std::numeric_limits<double>::epsilon()")
            { return SAC::binary_to_double(SAC::double_to_binary(neg_epsilon)); };
        }
        SECTION("Positive")
        {
            BENCHMARK("Double->Binary->Double std::numeric_limits<double>::max()")
            { return SAC::binary_to_double(SAC::double_to_binary(std::numeric_limits<double>::max())); };
            float epsilon{std::numeric_limits<double>::epsilon()};
            std::ostringstream oss;
            oss << std::setprecision(std::numeric_limits<double>::max_digits10) << epsilon;
            std::string s_epsilon{oss.str()};
            CAPTURE(s_epsilon);
            BENCHMARK("Double->Binary->Double std::numeric_limits<double>::epsilon()")
            { return SAC::binary_to_double(SAC::double_to_binary(epsilon)); };
        }
    }
    SECTION("Strings")
    {
        SECTION("Regular - Two Words")
        {
            SECTION("Perfect")
            {
                const std::string test_str{"01234567"};
                CAPTURE(test_str);
                BENCHMARK("String->Binary->String Exact")
                { return SAC::binary_to_string(SAC::string_to_binary(test_str)); };
            }
            SECTION("Empty")
            {
                const std::string test_str{""};
                BENCHMARK("String->Binary->String Empty")
                { return SAC::binary_to_string(SAC::string_to_binary(test_str)); };
            }
            SECTION("Small")
            {
                const std::string test_str{"0123"};
                BENCHMARK("String->Binary->String Half")
                { return SAC::binary_to_string(SAC::string_to_binary(test_str)); };
            }
            SECTION("Overflow")
            {
                const std::string test_str{"0123456789ABCDEFG"};
                CAPTURE(test_str);
                BENCHMARK("String->Binary->String Overfull")
                { return SAC::binary_to_string(SAC::string_to_binary(test_str)); };
            }
        }
        SECTION("Long - Four Words")
        {
            SECTION("Perfect")
            {
                const std::string test_str{"0123456789ABCDEF"};
                BENCHMARK("String->Binary->String Exact")
                { return SAC::binary_to_string(SAC::string_to_binary(test_str)); };
            }
            SECTION("Empty")
            {
                const std::string test_str{""};
                BENCHMARK("String->Binary->String Empty")
                { return SAC::binary_to_string(SAC::string_to_binary(test_str)); };
            }
            SECTION("Small")
            {
                const std::string test_str{"01234567"};
                BENCHMARK("String->Binary->String Half")
                { return SAC::binary_to_string(SAC::string_to_binary(test_str)); };
            }
            SECTION("Overflow")
            {
                const std::string test_str{"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"};
                CAPTURE(test_str);
                BENCHMARK("String->Binary->String Overfull")
                { return SAC::binary_to_string(SAC::string_to_binary(test_str)); };
            }
        }
    }
}