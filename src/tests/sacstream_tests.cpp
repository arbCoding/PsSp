//-----------------------------------------------------------------------------
// Include statements
//-----------------------------------------------------------------------------
#include "sac_stream.hpp"
// Catch2 Standard headers
#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
// Good and fast random number generation
#include <XoshiroCpp.hpp>
// Standard Library stuff, https://en.cppreference.com/w/cpp/standard_library  
#include <chrono>
#include <filesystem>
// needed by Xoshiro to seed the png and to create the uniform distribution
// Xoshiro then randomly samples that distribution
#include <random>
//-----------------------------------------------------------------------------
// End Include statements
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Random number generation
//-----------------------------------------------------------------------------
namespace pssp
{
// Self-seeding
XoshiroCpp::Xoshiro256Plus init()
{
    // Random device for seeding
    std::random_device rd{};
    // Two runtime constants of current time
    const uint64_t t1{static_cast<uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count())};
    const uint64_t t2{static_cast<uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count())};
    // Seed the initial state
    const XoshiroCpp::Xoshiro256Plus::state_type initial_state =
    {
        XoshiroCpp::SplitMix64{t1}(),
        XoshiroCpp::SplitMix64{rd()}(),
        XoshiroCpp::SplitMix64{t2}(),
        XoshiroCpp::SplitMix64{rd()}()
    };
    return XoshiroCpp::Xoshiro256Plus{initial_state};
}

// build the PRNG
inline XoshiroCpp::Xoshiro256Plus xos{init()};

// give us the random double within the bounds [min, max] (inclusive)
inline double get(double min, double max) { std::uniform_real_distribution<> die{min, max}; return die(xos); }

void random_vector(std::vector<double>& data, const double minimum = -1.0, const double maximum = 1.0)
{
    std::uniform_real_distribution<> die{minimum, maximum};
    for (std::size_t i{0}; i < data.size(); ++i)
    {
        data[i] = die(pssp::xos);
    }
}
}
//-----------------------------------------------------------------------------
// Random number generation
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

// For debugging
void print_control_characters(const std::string& str)
{
    for (char c : str)
    {
        if (std::iscntrl(static_cast<unsigned char>(c))) { std::cout << "\\x" << std::hex << static_cast<int>(c); }
        else { std::cout << c; }
    }
    std::cout << '\n';
}

TEST_CASE("Input/Output")
{
    SECTION("Empty SAC::SacStream")
    {
        SAC::SacStream test_sac{};
        std::filesystem::path test_dir{std::filesystem::temp_directory_path()};
        std::filesystem::path test_file{test_dir / "test.SAC"};
        std::cout << "Test file: " << test_file << '\n';
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
            REQUIRE(in_sac == test_sac);
            std::filesystem::remove(test_file);
        }
    }

    SECTION("Non-Empty SAC::SacStream")
    {
        // Building a SacStream manually because why not!
        // None of these values have any meaning in this context
        // The only requirement is that the `f_var` versions match their `var` counterparts
        // And that npts corresponds to the size of data1 (and data2 if appropriate)
        SAC::SacStream test_sac{};
        test_sac.f_delta = 0.025f;
        test_sac.delta = 0.025;
        test_sac.depmin = -1.11f;
        test_sac.depmax = 1.23f;
        test_sac.odelta = 0.026f;
        test_sac.f_b = 0.1f;
        test_sac.b = 0.1;
        test_sac.f_e = 0.0f;
        test_sac.e = 0.0f;
        test_sac.f_a = 5.78f;
        test_sac.a = 5.78;
        test_sac.f_t0 = 1.234f;
        test_sac.t0 = 1.234;
        test_sac.f_t1 = -1.234f;
        test_sac.t1 = -1.234;
        test_sac.f_t2 = 2.345f;
        test_sac.t2 = 2.345;
        test_sac.f_t3 = -2.345f;
        test_sac.t3 = -2.345;
        test_sac.f_t4 = 3.456f;
        test_sac.t4 = 3.456;
        test_sac.f_t5 = -3.456f;
        test_sac.t5 = -3.456;
        test_sac.f_t6 = 4.567f;
        test_sac.t6 = 4.567;
        test_sac.f_t7 = -4.567f;
        test_sac.t7 = -4.567;
        test_sac.f_t8 = 5.678f;
        test_sac.t8 = 5.678;
        test_sac.f_t9 = -5.678f;
        test_sac.t9 = -5.678;
        test_sac.f_f = 12345.0f;
        test_sac.f = 12345.0;
        test_sac.resp0 = 0.1f;
        test_sac.resp1 = -0.1f;
        test_sac.resp2 = 1.0f;
        test_sac.resp3 = -1.0f;
        test_sac.resp4 = 10.0f;
        test_sac.resp5 = -10.0f;
        test_sac.resp6 = 100.0f;
        test_sac.resp7 = -100.0f;
        test_sac.resp8 = 1000.0f;
        test_sac.resp9 = -1000.0f;
        test_sac.f_stla = 32.21f;
        test_sac.stla = 32.21;
        test_sac.f_stlo = -121.38f;
        test_sac.stlo = -121.38;
        test_sac.stel = 100.0f;
        test_sac.stdp = 50.0f;
        test_sac.f_evla = -81.35f;
        test_sac.evla = -81.35;
        test_sac.f_evlo = 85.37f;
        test_sac.evlo = 85.37;
        test_sac.evel = 0.5f;
        test_sac.evdp = 30.25f;
        test_sac.mag = 1.83f;
        test_sac.user0 = 9.0f;
        test_sac.user1 = -9.0f;
        test_sac.user2 = 8.0f;
        test_sac.user3 = -8.0f;
        test_sac.user4 = 7.0f;
        test_sac.user5 = -7.0f;
        test_sac.user6 = 6.0f;
        test_sac.user7 = -6.0f;
        test_sac.user8 = 5.0f;
        test_sac.user9 = -5.0f;
        test_sac.dist = 1300.0f;
        test_sac.az = 35.0f;
        test_sac.baz = -275.0f;
        test_sac.gcarc = 13.135f;
        test_sac.f_sb = -35.0f;
        test_sac.sb = -35.0;
        test_sac.f_sdelta = 0.125f;
        test_sac.sdelta = 0.125;
        test_sac.depmen = 0.1f;
        test_sac.cmpaz = 97.3f;
        test_sac.cmpinc = 273.0f;
        test_sac.xminimum = -5.0f;
        test_sac.xmaximum = 5.0f;
        test_sac.yminimum = -10.0f;
        test_sac.ymaximum = 10.0f;
        test_sac.nzyear = 2023;
        test_sac.nzjday = 123;
        test_sac.nzhour = 13;
        test_sac.nzmin = 57;
        test_sac.nzsec = 34;
        test_sac.nzmsec = 0;
        test_sac.nvhdr = 7;
        test_sac.norid = 1;
        test_sac.nevid = 2;
        test_sac.npts = 100000;
        test_sac.nsnpts = 50000;
        test_sac.nwfid = 3;
        test_sac.nxsize = 35;
        test_sac.nysize = 15;
        test_sac.iftype = 1;
        test_sac.idep = 50;
        test_sac.iztype = 12;
        test_sac.iinst = 83;
        test_sac.istreg = 82;
        test_sac.ievreg = 81;
        test_sac.ievtyp = 73;
        test_sac.iqual = 45;
        test_sac.isynth = 1;
        test_sac.imagtyp = 57;
        test_sac.imagsrc = 70;
        test_sac.ibody = 99;
        test_sac.leven = true;
        test_sac.lpspol = true;
        test_sac.lovrok = true;
        test_sac.lcalda = true;
        test_sac.kstnm = "Test1";
        test_sac.kevnm = "Fake Quake";
        test_sac.khole = "AB";
        test_sac.ko = "Hi";
        test_sac.ka = "Yup";
        test_sac.kt0 = "Zero 0";
        test_sac.kt1 = "One 1";
        test_sac.kt2 = "Two 2";
        test_sac.kt3 = "Three 3";
        test_sac.kt4 = "Four 4";
        test_sac.kt5 = "Five 5";
        test_sac.kt6 = "Six 6";
        test_sac.kt7 = "Seven 7";
        test_sac.kt8 = "Eight 8";
        test_sac.kt9 = "Nine 9";
        test_sac.kf = "Fini";
        test_sac.kuser0 = "User0 Ze";
        test_sac.kuser1 = "User1 On";
        test_sac.kuser2 = "User2 Tw";
        test_sac.kcmpnm = "HHZ";
        test_sac.knetwk = "ZZ";
        test_sac.kdatrd = "None";
        test_sac.kinst = "Not Real";
        // Resize data vectors to new size, zero-filled by default
        if (test_sac.npts > 0)
        {
            test_sac.data1.resize(test_sac.npts, 0.0);
            if (test_sac.leven == false || test_sac.iftype > 1) { test_sac.data2.resize(test_sac.npts, 0.0); }
        }
        // Done building
        std::filesystem::path test_dir{std::filesystem::temp_directory_path()};
        std::filesystem::path test_file{test_dir / "test.SAC"};
        std::cout << "Test file: " << test_file << '\n';
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
        SECTION("Comparison Between Out and In Zeros")
        {
            test_sac.write(test_file);
            SAC::SacStream in_sac = SAC::SacStream(test_file.string());
            REQUIRE(test_sac == in_sac);
            BENCHMARK("SacStream Comparison") { (void) (test_sac == in_sac); };
            std::filesystem::remove(test_file);
        }
        SECTION("Randomizing data")
        {
            BENCHMARK("Random vector generation.") { pssp::random_vector(test_sac.data1); };
        }
        SECTION("Comparison Between Out and In Random")
        {
            pssp::random_vector(test_sac.data1);
            if (test_sac.leven == false || test_sac.iftype > 1) { pssp::random_vector(test_sac.data2); }
            test_sac.write(test_file);
            SAC::SacStream in_sac = SAC::SacStream(test_file.string());
            // Note that this equality tests to equality within tolerance of what can be handled via a float
            // this is because binary SAC files use floats for the data values, not doubles
            REQUIRE(test_sac == in_sac);
            BENCHMARK("SacStream Comparison") { (void) (test_sac == in_sac); };
            std::filesystem::remove(test_file);
        }
    }
}
