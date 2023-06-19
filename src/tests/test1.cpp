#include "sac_io.hpp"
#include <sac_stream.hpp>
#include <catch_amalgamated.hpp>

TEST_CASE("Empty SAC::SacStream")
{
    SAC::SacStream test_sac{};
    REQUIRE(test_sac.a == SAC::unset_double);
    REQUIRE(test_sac.data1.size() == 0);
}
