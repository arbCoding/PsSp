//-----------------------------------------------------------------------------
// Include statements
//-----------------------------------------------------------------------------
#include "sac_stream.hpp"
// Catch2 Amalgamated header
#include <catch_amalgamated.hpp>
// Standard Library stuff, https://en.cppreference.com/w/cpp/standard_library
#include <filesystem>
//-----------------------------------------------------------------------------
// End Include statements
//-----------------------------------------------------------------------------

// Test an empty sacstream
TEST_CASE("Empty SAC::SacStream")
{
    SECTION("Building an empty SAC::SacStream")
    {
        BENCHMARK("Construct and Destruct") { SAC::SacStream tmp_sac{}; return; };
    }
    SAC::SacStream test_sac{};
    // Might seem silly, but you never know!
    SECTION("Unset Floats are equal to SAC::unset_float (-12345.0f)")
    {
        REQUIRE(test_sac.f_delta == SAC::unset_float);
        REQUIRE(test_sac.depmin == SAC::unset_float);
        REQUIRE(test_sac.depmax == SAC::unset_float);
        REQUIRE(test_sac.odelta == SAC::unset_float);
        REQUIRE(test_sac.f_b == SAC::unset_float);
        REQUIRE(test_sac.f_e == SAC::unset_float);
        REQUIRE(test_sac.f_o == SAC::unset_float);
        REQUIRE(test_sac.f_a == SAC::unset_float);
        REQUIRE(test_sac.f_t0 == SAC::unset_float);
        REQUIRE(test_sac.f_t1 == SAC::unset_float);
        REQUIRE(test_sac.f_t2 == SAC::unset_float);
        REQUIRE(test_sac.f_t3 == SAC::unset_float);
        REQUIRE(test_sac.f_t4 == SAC::unset_float);
        REQUIRE(test_sac.f_t5 == SAC::unset_float);
        REQUIRE(test_sac.f_t6 == SAC::unset_float);
        REQUIRE(test_sac.f_t7 == SAC::unset_float);
        REQUIRE(test_sac.f_t8 == SAC::unset_float);
        REQUIRE(test_sac.f_t9 == SAC::unset_float);
        REQUIRE(test_sac.f_f == SAC::unset_float);
        REQUIRE(test_sac.resp0 == SAC::unset_float);
        REQUIRE(test_sac.resp1 == SAC::unset_float);
        REQUIRE(test_sac.resp2 == SAC::unset_float);
        REQUIRE(test_sac.resp3 == SAC::unset_float);
        REQUIRE(test_sac.resp4 == SAC::unset_float);
        REQUIRE(test_sac.resp5 == SAC::unset_float);
        REQUIRE(test_sac.resp6 == SAC::unset_float);
        REQUIRE(test_sac.resp7 == SAC::unset_float);
        REQUIRE(test_sac.resp8 == SAC::unset_float);
        REQUIRE(test_sac.resp9 == SAC::unset_float);
        REQUIRE(test_sac.f_stla == SAC::unset_float);
        REQUIRE(test_sac.f_stlo == SAC::unset_float);
        REQUIRE(test_sac.stel == SAC::unset_float);
        REQUIRE(test_sac.stdp == SAC::unset_float);
        REQUIRE(test_sac.f_evla == SAC::unset_float);
        REQUIRE(test_sac.f_evlo == SAC::unset_float);
        REQUIRE(test_sac.evel == SAC::unset_float);
        REQUIRE(test_sac.evdp == SAC::unset_float);
        REQUIRE(test_sac.mag == SAC::unset_float);
        REQUIRE(test_sac.user0 == SAC::unset_float);
        REQUIRE(test_sac.user1 == SAC::unset_float);
        REQUIRE(test_sac.user2 == SAC::unset_float);
        REQUIRE(test_sac.user3 == SAC::unset_float);
        REQUIRE(test_sac.user4 == SAC::unset_float);
        REQUIRE(test_sac.user5 == SAC::unset_float);
        REQUIRE(test_sac.user6 == SAC::unset_float);
        REQUIRE(test_sac.user7 == SAC::unset_float);
        REQUIRE(test_sac.user8 == SAC::unset_float);
        REQUIRE(test_sac.user9 == SAC::unset_float);
        REQUIRE(test_sac.dist == SAC::unset_float);
        REQUIRE(test_sac.az == SAC::unset_float);
        REQUIRE(test_sac.baz == SAC::unset_float);
        REQUIRE(test_sac.gcarc == SAC::unset_float);
        REQUIRE(test_sac.f_sb == SAC::unset_float);
        REQUIRE(test_sac.f_sdelta == SAC::unset_float);
        REQUIRE(test_sac.depmen == SAC::unset_float);
        REQUIRE(test_sac.cmpaz == SAC::unset_float);
        REQUIRE(test_sac.cmpinc == SAC::unset_float);
        REQUIRE(test_sac.xminimum == SAC::unset_float);
        REQUIRE(test_sac.xmaximum == SAC::unset_float);
        REQUIRE(test_sac.yminimum == SAC::unset_float);
        REQUIRE(test_sac.ymaximum == SAC::unset_float);
    }

    SECTION("Unset Ints are equal to SAC::unset_int (-12345)")
    {
        REQUIRE(test_sac.nzyear == SAC::unset_int);
        REQUIRE(test_sac.nzjday == SAC::unset_int);
        REQUIRE(test_sac.nzhour == SAC::unset_int);
        REQUIRE(test_sac.nzmin == SAC::unset_int);
        REQUIRE(test_sac.nzsec == SAC::unset_int);
        REQUIRE(test_sac.nzmsec == SAC::unset_int);
        REQUIRE(test_sac.nvhdr == 7);
        REQUIRE(test_sac.norid == SAC::unset_int);
        REQUIRE(test_sac.nevid == SAC::unset_int);
        REQUIRE(test_sac.npts == SAC::unset_int);
        REQUIRE(test_sac.nsnpts == SAC::unset_int);
        REQUIRE(test_sac.nwfid == SAC::unset_int);
        REQUIRE(test_sac.nxsize == SAC::unset_int);
        REQUIRE(test_sac.nysize == SAC::unset_int);
        REQUIRE(test_sac.iftype == SAC::unset_int);
        REQUIRE(test_sac.idep == SAC::unset_int);
        REQUIRE(test_sac.iztype == SAC::unset_int);
        REQUIRE(test_sac.iinst == SAC::unset_int);
        REQUIRE(test_sac.istreg == SAC::unset_int);
        REQUIRE(test_sac.ievreg == SAC::unset_int);
        REQUIRE(test_sac.ievtyp == SAC::unset_int);
        REQUIRE(test_sac.iqual == SAC::unset_int);
        REQUIRE(test_sac.isynth == SAC::unset_int);
        REQUIRE(test_sac.imagtyp == SAC::unset_int);
        REQUIRE(test_sac.imagsrc == SAC::unset_int);
        REQUIRE(test_sac.ibody == SAC::unset_int);
    }

    SECTION("Unset Bools are equal to SAC::unset_bool (0)")
    {
        REQUIRE(test_sac.leven == SAC::unset_bool);
        REQUIRE(test_sac.lpspol == SAC::unset_bool);
        REQUIRE(test_sac.lovrok == SAC::unset_bool);
        REQUIRE(test_sac.lcalda == SAC::unset_bool);
    }

    SECTION("Unset Strings are equal to SAC::unset_word (\"-12345\")")
    {
        REQUIRE(test_sac.kstnm == SAC::unset_word);
        REQUIRE(test_sac.kevnm == SAC::unset_word);
        REQUIRE(test_sac.khole == SAC::unset_word);
        REQUIRE(test_sac.ko == SAC::unset_word);
        REQUIRE(test_sac.ka == SAC::unset_word);
        REQUIRE(test_sac.kt0 == SAC::unset_word);
        REQUIRE(test_sac.kt1 == SAC::unset_word);
        REQUIRE(test_sac.kt2 == SAC::unset_word);
        REQUIRE(test_sac.kt3 == SAC::unset_word);
        REQUIRE(test_sac.kt4 == SAC::unset_word);
        REQUIRE(test_sac.kt5 == SAC::unset_word);
        REQUIRE(test_sac.kt6 == SAC::unset_word);
        REQUIRE(test_sac.kt7 == SAC::unset_word);
        REQUIRE(test_sac.kt8 == SAC::unset_word);
        REQUIRE(test_sac.kt9 == SAC::unset_word);
        REQUIRE(test_sac.kf == SAC::unset_word);
        REQUIRE(test_sac.kuser0 == SAC::unset_word);
        REQUIRE(test_sac.kuser1 == SAC::unset_word);
        REQUIRE(test_sac.kuser2 == SAC::unset_word);
        REQUIRE(test_sac.kcmpnm == SAC::unset_word);
        REQUIRE(test_sac.knetwk == SAC::unset_word);
        REQUIRE(test_sac.kdatrd == SAC::unset_word);
        REQUIRE(test_sac.kinst == SAC::unset_word);
    }

    SECTION("Unset Data Vectors are Empty")
    {
        REQUIRE(test_sac.data1.empty());
        REQUIRE(test_sac.data2.empty());
    }

    SECTION("Unset Doubles are equal to SAC::unset_double (-12345.0)")
    {
        REQUIRE(test_sac.delta == SAC::unset_double);
        REQUIRE(test_sac.b == SAC::unset_double);
        REQUIRE(test_sac.e == SAC::unset_double);
        REQUIRE(test_sac.o == SAC::unset_double);
        REQUIRE(test_sac.a == SAC::unset_double);
        REQUIRE(test_sac.t0 == SAC::unset_double);
        REQUIRE(test_sac.t1 == SAC::unset_double);
        REQUIRE(test_sac.t2 == SAC::unset_double);
        REQUIRE(test_sac.t3 == SAC::unset_double);
        REQUIRE(test_sac.t4 == SAC::unset_double);
        REQUIRE(test_sac.t5 == SAC::unset_double);
        REQUIRE(test_sac.t6 == SAC::unset_double);
        REQUIRE(test_sac.t7 == SAC::unset_double);
        REQUIRE(test_sac.t8 == SAC::unset_double);
        REQUIRE(test_sac.t9 == SAC::unset_double);
        REQUIRE(test_sac.f == SAC::unset_double);
        REQUIRE(test_sac.evlo == SAC::unset_double);
        REQUIRE(test_sac.evla == SAC::unset_double);
        REQUIRE(test_sac.stlo == SAC::unset_double);
        REQUIRE(test_sac.stla == SAC::unset_double);
        REQUIRE(test_sac.sb == SAC::unset_double);
        REQUIRE(test_sac.sdelta == SAC::unset_double);
    }
}

TEST_CASE("Input/Output")
{
    SAC::SacStream test_sac{};
    std::filesystem::path test_dir{std::filesystem::temp_directory_path()};
    std::filesystem::path test_file{test_dir / "test.SAC"};
    std::cout << "Test file: " << test_file << '\n';
    SECTION("Empty SAC::SacStream")
    {
        SECTION("Out")
        {
            BENCHMARK("Writing") { test_sac.write(test_file); return; };
            std::filesystem::remove(test_file);
        }
        SECTION("In")
        {
            test_sac.write(test_file);
            BENCHMARK("Reading") { SAC::SacStream in_sac = SAC::SacStream(test_file.string()); };
            std::filesystem::remove(test_file);
        }
        SECTION("Comparison Between Out and In")
        {
            test_sac.write(test_file);
            SAC::SacStream in_sac = SAC::SacStream(test_file.string());
            std::cout << test_sac.kstnm << ',' << in_sac.kstnm << '\n';
            std::cout << in_sac.kstnm << ',' << test_sac.kstnm << '\n';
            std::cout << test_sac.kstnm.size() << ',' << in_sac.kstnm.size() << '\n';
            REQUIRE(in_sac == test_sac);
            std::filesystem::remove(test_file);
        }
    }
}
