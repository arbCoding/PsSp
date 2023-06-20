//-----------------------------------------------------------------------------
// Include statements
//-----------------------------------------------------------------------------
#include "sac_io.hpp"
// Catch2 Amalgamated header
#include <catch_amalgamated.hpp>
// Standard Library stuff, https://en.cppreference.com/w/cpp/standard_library
#include <bitset>
#include <iomanip>
#include <limits>
//-----------------------------------------------------------------------------
// End Include statements
//-----------------------------------------------------------------------------

// Test an empty sacstream
TEST_CASE("Basic constants")
{
    SECTION("Constants (constexpr)")
    {
        REQUIRE(SAC::word_length == 4);
        REQUIRE(SAC::bits_per_byte == 8);
        REQUIRE(SAC::binary_word_size == 32);
        REQUIRE(SAC::data_word == 158);
        REQUIRE(SAC::unset_int == -12345);
        REQUIRE(SAC::unset_float == -12345.0f);
        REQUIRE(SAC::unset_double == -12345.0);
        REQUIRE(SAC::unset_bool == 0);
        REQUIRE(SAC::unset_word == "-12345");
    }
    SECTION("Constants (derived)") { REQUIRE(SAC::word_position(SAC::data_word) == 632); }
}

TEST_CASE("Binary Conversion")
{
    SECTION("Booleans")
    {
        REQUIRE(SAC::unset_bool == false);
        REQUIRE(!SAC::unset_bool == true);
        REQUIRE(SAC::binary_to_bool(SAC::bool_to_binary(SAC::unset_bool)) == false);
        REQUIRE(SAC::binary_to_bool(SAC::bool_to_binary(!SAC::unset_bool)) == true);
        BENCHMARK("Bool->Binary->Bool False")
        { return SAC::binary_to_bool(SAC::bool_to_binary(SAC::unset_bool)); };
    }
    SECTION("Integers")
    {
        SECTION("Zero")
        {
            REQUIRE(SAC::binary_to_int(SAC::int_to_binary(0)) == 0);
            REQUIRE(SAC::binary_to_int(SAC::int_to_binary(-0)) == 0);
            BENCHMARK("Int->Binary->Int 0")
            { return SAC::binary_to_int(SAC::int_to_binary(0)); };
        }
        SECTION("Negative")
        {
            REQUIRE(SAC::binary_to_int(SAC::int_to_binary(-1)) == -1);
            REQUIRE(SAC::binary_to_int(SAC::int_to_binary(-10)) == -10);
            REQUIRE(SAC::binary_to_int(SAC::int_to_binary(INT_MIN)) == INT_MIN);
            BENCHMARK("Int->Binary->Int INT_MIN")
            { return SAC::binary_to_int(SAC::int_to_binary(INT_MIN)); };
        }
        SECTION("Positive")
        {
            REQUIRE(SAC::binary_to_int(SAC::int_to_binary(1)) == 1);
            REQUIRE(SAC::binary_to_int(SAC::int_to_binary(10)) == 10);
            REQUIRE(SAC::binary_to_int(SAC::int_to_binary(INT_MAX)) == INT_MAX);
            BENCHMARK("Int->Binary->Int INT_MAX")
            { return SAC::binary_to_int(SAC::int_to_binary(INT_MAX)); };
        }
    }
    SECTION("Floats")
    {
        SECTION("Zero")
        {
            REQUIRE(SAC::binary_to_float(SAC::float_to_binary(0.0f)) == 0.0f);
            REQUIRE(SAC::binary_to_float(SAC::float_to_binary(-0.0f)) == 0.0f);
            BENCHMARK("Float->Binary->Float 0.0f")
            { return SAC::binary_to_float(SAC::float_to_binary(0.0f)); };
        }
        SECTION("Negative")
        {
            REQUIRE(SAC::binary_to_float(SAC::float_to_binary(-1.0f)) == -1.0f);
            REQUIRE(SAC::binary_to_float(SAC::float_to_binary(-0.1f)) == -0.1f);
            REQUIRE(SAC::binary_to_float(SAC::float_to_binary(-10.0f)) == -10.0f);
            REQUIRE(SAC::binary_to_float(SAC::float_to_binary(std::numeric_limits<float>::lowest())) == std::numeric_limits<float>::lowest());
            BENCHMARK("Float->Binary->Float std::numeric_limits<float>::lowest()")
            { return SAC::binary_to_float(SAC::float_to_binary(std::numeric_limits<float>::lowest())); };
            float neg_epsilon{-std::numeric_limits<float>::epsilon()};
            std::ostringstream oss;
            oss << std::setprecision(std::numeric_limits<float>::max_digits10) << neg_epsilon;
            std::string s_neg_epsilon{oss.str()};
            CAPTURE(s_neg_epsilon);
            REQUIRE(SAC::binary_to_float(SAC::float_to_binary(neg_epsilon)) == neg_epsilon);
            BENCHMARK("Float->Binary->Float negative std::numeric_limits<float>::epsilon()")
            { return SAC::binary_to_float(SAC::float_to_binary(neg_epsilon)); };
        }
        SECTION("Positive")
        {
            REQUIRE(SAC::binary_to_float(SAC::float_to_binary(1.0f)) == 1.0f);
            REQUIRE(SAC::binary_to_float(SAC::float_to_binary(0.1f)) == 0.1f);
            REQUIRE(SAC::binary_to_float(SAC::float_to_binary(10.0f)) == 10.0f);
            REQUIRE(SAC::binary_to_float(SAC::float_to_binary(std::numeric_limits<float>::max())) == std::numeric_limits<float>::max());
            BENCHMARK("Float->Binary->Float std::numeric_limits<float>::max()")
            { return SAC::binary_to_float(SAC::float_to_binary(std::numeric_limits<float>::max())); };
            float epsilon{std::numeric_limits<float>::epsilon()};
            std::ostringstream oss;
            oss << std::setprecision(std::numeric_limits<float>::max_digits10) << epsilon;
            std::string s_epsilon{oss.str()};
            CAPTURE(s_epsilon);
            REQUIRE(SAC::binary_to_float(SAC::float_to_binary(epsilon)) == epsilon);
            BENCHMARK("Float->Binary->Float std::numeric_limits<float>::epsilon()")
            { return SAC::binary_to_float(SAC::float_to_binary(epsilon)); };
        }
    }
    SECTION("Doubles")
    {
        SECTION("Zero")
        {
            REQUIRE(SAC::binary_to_double(SAC::double_to_binary(0.0)) == 0.0);
            REQUIRE(SAC::binary_to_double(SAC::double_to_binary(-0.0)) == 0.0);
            BENCHMARK("Double->Binary->Double 0.0")
            { return SAC::binary_to_double(SAC::double_to_binary(0.0)); };
        }
        SECTION("Negative")
        {
            REQUIRE(SAC::binary_to_double(SAC::double_to_binary(-1.0)) == -1.0);
            REQUIRE(SAC::binary_to_double(SAC::double_to_binary(-0.1)) == -0.1);
            REQUIRE(SAC::binary_to_double(SAC::double_to_binary(-10.0)) == -10.0);
            REQUIRE(SAC::binary_to_double(SAC::double_to_binary(std::numeric_limits<double>::lowest())) == std::numeric_limits<double>::lowest());
            BENCHMARK("Double->Binary->Double std::numeric_limits<double>::lowest()")
            { return SAC::binary_to_double(SAC::double_to_binary(std::numeric_limits<double>::lowest())); };
            float neg_epsilon{-std::numeric_limits<double>::epsilon()};
            std::ostringstream oss;
            oss << std::setprecision(std::numeric_limits<double>::max_digits10) << neg_epsilon;
            std::string s_neg_epsilon{oss.str()};
            CAPTURE(s_neg_epsilon);
            REQUIRE(SAC::binary_to_double(SAC::double_to_binary(neg_epsilon)) == neg_epsilon);
            BENCHMARK("Double->Binary->Double negative std::numeric_limits<double>::epsilon()")
            { return SAC::binary_to_double(SAC::double_to_binary(neg_epsilon)); };
        }
        SECTION("Positive")
        {
            REQUIRE(SAC::binary_to_double(SAC::double_to_binary(1.0)) == 1.0);
            REQUIRE(SAC::binary_to_double(SAC::double_to_binary(0.1)) == 0.1);
            REQUIRE(SAC::binary_to_double(SAC::double_to_binary(10.0)) == 10.0);
            REQUIRE(SAC::binary_to_double(SAC::double_to_binary(std::numeric_limits<double>::max())) == std::numeric_limits<double>::max());
            BENCHMARK("Double->Binary->Double std::numeric_limits<double>::max()")
            { return SAC::binary_to_double(SAC::double_to_binary(std::numeric_limits<double>::max())); };
            float epsilon{std::numeric_limits<double>::epsilon()};
            std::ostringstream oss;
            oss << std::setprecision(std::numeric_limits<double>::max_digits10) << epsilon;
            std::string s_epsilon{oss.str()};
            CAPTURE(s_epsilon);
            REQUIRE(SAC::binary_to_double(SAC::double_to_binary(epsilon)) == epsilon);
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
                REQUIRE(SAC::binary_to_string(SAC::string_to_binary(test_str)) == test_str);
                BENCHMARK("String->Binary->String Exact")
                { return SAC::binary_to_string(SAC::string_to_binary(test_str)); };
            }
            SECTION("Empty")
            {
                const std::string test_str{""};
                REQUIRE(SAC::binary_to_string(SAC::string_to_binary(test_str)) == test_str);
                BENCHMARK("String->Binary->String Empty")
                { return SAC::binary_to_string(SAC::string_to_binary(test_str)); };
            }
            SECTION("Small")
            {
                const std::string test_str{"0123"};
                REQUIRE(SAC::binary_to_string(SAC::string_to_binary(test_str)) == test_str);
                BENCHMARK("String->Binary->String Half")
                { return SAC::binary_to_string(SAC::string_to_binary(test_str)); };
            }
            SECTION("Overflow")
            {
                const std::string test_str{"0123456789ABCDEFG"};
                const std::string result_str{"01234567"};
                CAPTURE(test_str);
                REQUIRE(SAC::binary_to_string(SAC::string_to_binary(test_str)) == result_str);
                REQUIRE(SAC::binary_to_string(SAC::string_to_binary(test_str)) == test_str.substr(0, 8));
                BENCHMARK("String->Binary->String Overfull")
                { return SAC::binary_to_string(SAC::string_to_binary(test_str)); };
            }
        }
        SECTION("Long - Four Words")
        {
            SECTION("Perfect")
            {
                const std::string test_str{"0123456789ABCDEF"};
                REQUIRE(SAC::binary_to_long_string(SAC::long_string_to_binary(test_str)) == test_str);
                BENCHMARK("String->Binary->String Exact")
                { return SAC::binary_to_string(SAC::string_to_binary(test_str)); };
            }
            SECTION("Empty")
            {
                const std::string test_str{""};
                REQUIRE(SAC::binary_to_long_string(SAC::long_string_to_binary(test_str)) == test_str);
                BENCHMARK("String->Binary->String Empty")
                { return SAC::binary_to_string(SAC::string_to_binary(test_str)); };
            }
            SECTION("Small")
            {
                const std::string test_str{"01234567"};
                REQUIRE(SAC::binary_to_long_string(SAC::long_string_to_binary(test_str)) == test_str);
                BENCHMARK("String->Binary->String Half")
                { return SAC::binary_to_string(SAC::string_to_binary(test_str)); };
            }
            SECTION("Overflow")
            {
                const std::string test_str{"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"};
                const std::string result_str{"0123456789ABCDEF"};
                CAPTURE(test_str);
                REQUIRE(SAC::binary_to_long_string(SAC::long_string_to_binary(test_str)) == result_str);
                REQUIRE(SAC::binary_to_long_string(SAC::long_string_to_binary(test_str)) == test_str.substr(0, 16));
                BENCHMARK("String->Binary->String Overfull")
                { return SAC::binary_to_string(SAC::string_to_binary(test_str)); };
            }
        }
    }
}