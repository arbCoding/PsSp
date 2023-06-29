#include "pssp_spectral.hpp"
// Catch2 Standard headers
#include <catch2/catch_test_macros.hpp>
// Floating-point comparisons
#include <catch2/matchers/catch_matchers_floating_point.hpp>
// Standard Library stuff, https://en.cppreference.com/w/cpp/standard_library  
#include <iostream>
#include <vector>
// All tests pass on AFRO (x86_64 Ubuntu 23.04)
// Normalized Butterworth polynomials: https://en.wikipedia.org/wiki/Butterworth_filter#Normalized_Butterworth_polynomials
TEST_CASE("Normalized Butterworth Coefficients")
{
    SECTION("Order 1:")
    {
        constexpr int n{1};
        const std::vector<double> coeffs{pssp::butterworth_coeffs(n)};
        // Symmetry
        for (int i{0}; i <= (n + 1) / 2; ++i) { CAPTURE(i); CAPTURE(n - i); REQUIRE(coeffs[i] == coeffs[n - i]); }
        // Normalization
        REQUIRE(coeffs[0] == 1.0);
    }
    SECTION("Order 2:")
    {
        constexpr int n{2};
        const std::vector<double> coeffs{pssp::butterworth_coeffs(n)};
        // Symmetry
        for (int i{0}; i <= (n + 1) / 2; ++i) { CAPTURE(i); CAPTURE(n - i); REQUIRE(coeffs[i] == coeffs[n - i]); }
        // Normalization
        REQUIRE(coeffs[0] == 1.0);
        // Internal
        REQUIRE_THAT(coeffs[1], Catch::Matchers::WithinAbs(1.4142135624, 1e-10));
    }
    SECTION("Order 3:")
    {
        constexpr int n{3};
        const std::vector<double> coeffs{pssp::butterworth_coeffs(n)};
        // Symmetry
        for (int i{0}; i <= (n + 1) / 2; ++i) { CAPTURE(i); CAPTURE(n - i); REQUIRE(coeffs[i] == coeffs[n - i]); }
        // Normalization
        REQUIRE(coeffs[0] == 1.0);
        // Internal
        REQUIRE_THAT(coeffs[1], Catch::Matchers::WithinAbs(2, 1e-10));
    }
    SECTION("Order 4:")
    {
        constexpr int n{4};
        const std::vector<double> coeffs{pssp::butterworth_coeffs(n)};
        // Symmetry
        for (int i{0}; i <= (n + 1) / 2; ++i) { CAPTURE(i); CAPTURE(n - i); REQUIRE(coeffs[i] == coeffs[n - i]); }
        // Normalization
        REQUIRE(coeffs[0] == 1.0);
        // Internal
        REQUIRE_THAT(coeffs[1], Catch::Matchers::WithinAbs(2.6131259298, 1e-10));
        REQUIRE_THAT(coeffs[2], Catch::Matchers::WithinAbs(3.4142135624, 1e-10));
    }
    SECTION("Order 5:")
    {
        constexpr int n{5};
        const std::vector<double> coeffs{pssp::butterworth_coeffs(n)};
        // Symmetry
        for (int i{0}; i <= (n + 1) / 2; ++i) { CAPTURE(i); CAPTURE(n - i); REQUIRE(coeffs[i] == coeffs[n - i]); }
        // Normalization
        REQUIRE(coeffs[0] == 1.0);
        // Internal
        REQUIRE_THAT(coeffs[1], Catch::Matchers::WithinAbs(3.2360679775, 1e-10));
        REQUIRE_THAT(coeffs[2], Catch::Matchers::WithinAbs(5.2360679775, 1e-10));
    }
    SECTION("Order 6:")
    {
        constexpr int n{6};
        const std::vector<double> coeffs{pssp::butterworth_coeffs(n)};
        // Symmetry
        for (int i{0}; i <= (n + 1) / 2; ++i) { CAPTURE(i); CAPTURE(n - i); REQUIRE(coeffs[i] == coeffs[n - i]); }
        // Normalization
        REQUIRE(coeffs[0] == 1.0);
        // Internal
        REQUIRE_THAT(coeffs[1], Catch::Matchers::WithinAbs(3.8637033052, 1e-10));
        REQUIRE_THAT(coeffs[2], Catch::Matchers::WithinAbs(7.4641016151, 1e-10));
        REQUIRE_THAT(coeffs[3], Catch::Matchers::WithinAbs(9.1416201727, 1e-10));
    }
    SECTION("Order 7:")
    {
        constexpr int n{7};
        const std::vector<double> coeffs{pssp::butterworth_coeffs(n)};
        // Symmetry
        for (int i{0}; i <= (n + 1) / 2; ++i) { CAPTURE(i); CAPTURE(n - i); REQUIRE(coeffs[i] == coeffs[n - i]); }
        // Normalization
        REQUIRE(coeffs[0] == 1.0);
        // Internal
        REQUIRE_THAT(coeffs[1], Catch::Matchers::WithinAbs(4.4939592074 , 1e-10));
        REQUIRE_THAT(coeffs[2], Catch::Matchers::WithinAbs(10.097834679, 1e-10));
        REQUIRE_THAT(coeffs[3], Catch::Matchers::WithinAbs(14.5917938865, 1e-10));
    }
    SECTION("Order 8:")
    {
        constexpr int n{8};
        const std::vector<double> coeffs{pssp::butterworth_coeffs(n)};
        // Symmetry
        for (int i{0}; i <= (n + 1) / 2; ++i) { CAPTURE(i); CAPTURE(n - i); REQUIRE(coeffs[i] == coeffs[n - i]); }
        // Normalization
        REQUIRE(coeffs[0] == 1.0);
        // Internal
        REQUIRE_THAT(coeffs[1], Catch::Matchers::WithinAbs(5.1258308955 , 1e-10));
        REQUIRE_THAT(coeffs[2], Catch::Matchers::WithinAbs(13.1370711845, 1e-10));
        REQUIRE_THAT(coeffs[3], Catch::Matchers::WithinAbs(21.8461509692, 1e-10));
        REQUIRE_THAT(coeffs[4], Catch::Matchers::WithinAbs(25.6883559315, 1e-10));
    }
    SECTION("Order 9:")
    {
        constexpr int n{9};
        const std::vector<double> coeffs{pssp::butterworth_coeffs(n)};
        // Symmetry
        for (int i{0}; i <= (n + 1) / 2; ++i) { CAPTURE(i); CAPTURE(n - i); REQUIRE(coeffs[i] == coeffs[n - i]); }
        // Normalization
        REQUIRE(coeffs[0] == 1.0);
        // Internal
        REQUIRE_THAT(coeffs[1], Catch::Matchers::WithinAbs(5.7587704831 , 1e-10));
        REQUIRE_THAT(coeffs[2], Catch::Matchers::WithinAbs(16.5817187388, 1e-10));
        REQUIRE_THAT(coeffs[3], Catch::Matchers::WithinAbs(31.1634374775, 1e-10));
        REQUIRE_THAT(coeffs[4], Catch::Matchers::WithinAbs(41.9863857331, 1e-10));
    }
    SECTION("Order 10:")
    {
        constexpr int n{10};
        const std::vector<double> coeffs{pssp::butterworth_coeffs(n)};
        // Symmetry
        for (int i{0}; i <= (n + 1) / 2; ++i) { CAPTURE(i); CAPTURE(n - i); REQUIRE(coeffs[i] == coeffs[n - i]); }
        // Normalization
        REQUIRE(coeffs[0] == 1.0);
        // Internal
        REQUIRE_THAT(coeffs[1], Catch::Matchers::WithinAbs(6.3924532215 , 1e-10));
        REQUIRE_THAT(coeffs[2], Catch::Matchers::WithinAbs(20.4317290945, 1e-10));
        REQUIRE_THAT(coeffs[3], Catch::Matchers::WithinAbs(42.8020610689, 1e-10));
        REQUIRE_THAT(coeffs[4], Catch::Matchers::WithinAbs(64.8823962703, 1e-10));
        REQUIRE_THAT(coeffs[5], Catch::Matchers::WithinAbs(74.2334292571, 1e-10));
    }
}

TEST_CASE("Normalized Butterworth Polynomial")
{
    SECTION("s = 1 + 0i")
    {   
        constexpr std::complex<double> s{1.0, 0.0};
        SECTION("Order 1")
        {
            constexpr int n{1};
            const std::complex<double> bn_0{pssp::butterworth_laplace(pssp::butterworth_coeffs(n), s)};
            REQUIRE_THAT(bn_0.real(), Catch::Matchers::WithinAbs(2.0, 1e-10));
            REQUIRE(bn_0.imag() == 0.0);
        }
        SECTION("Order 2")
        {
            constexpr int n{2};
            const std::complex<double> bn_0{pssp::butterworth_laplace(pssp::butterworth_coeffs(n), s)};
            REQUIRE_THAT(bn_0.real(), Catch::Matchers::WithinAbs(3.4142135624, 1e-10));
            REQUIRE(bn_0.imag() == 0.0);
        }
        SECTION("Order 3")
        {
            constexpr int n{3};
            const std::complex<double> bn_0{pssp::butterworth_laplace(pssp::butterworth_coeffs(n), s)};
            REQUIRE_THAT(bn_0.real(), Catch::Matchers::WithinAbs(6.0, 1e-10));
            REQUIRE(bn_0.imag() == 0.0);
        }
        SECTION("Order 4")
        {
            constexpr int n{4};
            const std::complex<double> bn_0{pssp::butterworth_laplace(pssp::butterworth_coeffs(n), s)};
            REQUIRE_THAT(bn_0.real(), Catch::Matchers::WithinAbs(10.6404654219, 1e-10));
            REQUIRE(bn_0.imag() == 0.0);
        }
        SECTION("Order 5")
        {
            constexpr int n{5};
            const std::complex<double> bn_0{pssp::butterworth_laplace(pssp::butterworth_coeffs(n), s)};
            REQUIRE_THAT(bn_0.real(), Catch::Matchers::WithinAbs(18.94427191, 1e-10));
            REQUIRE(bn_0.imag() == 0.0);
        }
        SECTION("Order 6")
        {
            constexpr int n{6};
            const std::complex<double> bn_0{pssp::butterworth_laplace(pssp::butterworth_coeffs(n), s)};
            REQUIRE_THAT(bn_0.real(), Catch::Matchers::WithinAbs(33.7972300133, 1e-10));
            REQUIRE(bn_0.imag() == 0.0);
        }
        SECTION("Order 7")
        {
            constexpr int n{7};
            const std::complex<double> bn_0{pssp::butterworth_laplace(pssp::butterworth_coeffs(n), s)};
            REQUIRE_THAT(bn_0.real(), Catch::Matchers::WithinAbs(60.3671755459, 1e-10));
            REQUIRE(bn_0.imag() == 0.0);
        }
        SECTION("Order 8")
        {
            constexpr int n{8};
            const std::complex<double> bn_0{pssp::butterworth_laplace(pssp::butterworth_coeffs(n), s)};
            REQUIRE_THAT(bn_0.real(), Catch::Matchers::WithinAbs(107.9064620299, 1e-10));
            REQUIRE(bn_0.imag() == 0.0);
        }
        SECTION("Order 9")
        {
            constexpr int n{9};
            const std::complex<double> bn_0{pssp::butterworth_laplace(pssp::butterworth_coeffs(n), s)};
            REQUIRE_THAT(bn_0.real(), Catch::Matchers::WithinAbs(192.9806248652, 1e-10));
            REQUIRE(bn_0.imag() == 0.0);
        }
        SECTION("Order 10")
        {
            constexpr int n{10};
            const std::complex<double> bn_0{pssp::butterworth_laplace(pssp::butterworth_coeffs(n), s)};
            REQUIRE_THAT(bn_0.real(), Catch::Matchers::WithinAbs(345.2507085674, 1e-10));
            REQUIRE(bn_0.imag() == 0.0);
        }
    }
    SECTION("s = 0 + 1i")
    {   
        constexpr std::complex<double> s{0.0, 1.0};
        SECTION("Order 1")
        {
            constexpr int n{1};
            const std::complex<double> bn_0{pssp::butterworth_laplace(pssp::butterworth_coeffs(n), s)};
            REQUIRE_THAT(bn_0.real(), Catch::Matchers::WithinAbs(1.0, 1e-10));
            REQUIRE_THAT(bn_0.imag(), Catch::Matchers::WithinAbs(1.0, 1e-10));
        }
        SECTION("Order 2")
        {
            constexpr int n{2};
            const std::complex<double> bn_0{pssp::butterworth_laplace(pssp::butterworth_coeffs(n), s)};
            REQUIRE_THAT(bn_0.real(), Catch::Matchers::WithinAbs(0.0, 1e-10));
            REQUIRE_THAT(bn_0.imag(), Catch::Matchers::WithinAbs(1.4142135624, 1e-10));
        }
        SECTION("Order 3")
        {
            constexpr int n{3};
            const std::complex<double> bn_0{pssp::butterworth_laplace(pssp::butterworth_coeffs(n), s)};
            REQUIRE_THAT(bn_0.real(), Catch::Matchers::WithinAbs(-1.0, 1e-10));
            REQUIRE_THAT(bn_0.imag(), Catch::Matchers::WithinAbs(1.0, 1e-10));
        }
        SECTION("Order 4")
        {
            constexpr int n{4};
            const std::complex<double> bn_0{pssp::butterworth_laplace(pssp::butterworth_coeffs(n), s)};
            REQUIRE_THAT(bn_0.real(), Catch::Matchers::WithinAbs(-1.4142135624, 1e-10));
            REQUIRE_THAT(bn_0.imag(), Catch::Matchers::WithinAbs(0.0, 1e-10));
        }
        SECTION("Order 5")
        {
            constexpr int n{5};
            const std::complex<double> bn_0{pssp::butterworth_laplace(pssp::butterworth_coeffs(n), s)};
            REQUIRE_THAT(bn_0.real(), Catch::Matchers::WithinAbs(-1.0, 1e-10));
            REQUIRE_THAT(bn_0.imag(), Catch::Matchers::WithinAbs(-1.0, 1e-10));
        }
        SECTION("Order 6")
        {
            constexpr int n{6};
            const std::complex<double> bn_0{pssp::butterworth_laplace(pssp::butterworth_coeffs(n), s)};
            REQUIRE_THAT(bn_0.real(), Catch::Matchers::WithinAbs(0.0, 1e-10));
            REQUIRE_THAT(bn_0.imag(), Catch::Matchers::WithinAbs(-1.4142135624, 1e-10));
        }
        SECTION("Order 7")
        {
            constexpr int n{7};
            const std::complex<double> bn_0{pssp::butterworth_laplace(pssp::butterworth_coeffs(n), s)};
            REQUIRE_THAT(bn_0.real(), Catch::Matchers::WithinAbs(1.0, 1e-10));
            REQUIRE_THAT(bn_0.imag(), Catch::Matchers::WithinAbs(-1.0, 1e-10));
        }
        SECTION("Order 8")
        {
            constexpr int n{8};
            const std::complex<double> bn_0{pssp::butterworth_laplace(pssp::butterworth_coeffs(n), s)};
            REQUIRE_THAT(bn_0.real(), Catch::Matchers::WithinAbs(1.4142135624, 1e-10));
            REQUIRE_THAT(bn_0.imag(), Catch::Matchers::WithinAbs(0.0, 1e-10));
        }
        SECTION("Order 9")
        {
            constexpr int n{9};
            const std::complex<double> bn_0{pssp::butterworth_laplace(pssp::butterworth_coeffs(n), s)};
            REQUIRE_THAT(bn_0.real(), Catch::Matchers::WithinAbs(1.0, 1e-10));
            REQUIRE_THAT(bn_0.imag(), Catch::Matchers::WithinAbs(1.0, 1e-10));
        }
        SECTION("Order 10")
        {
            constexpr int n{10};
            const std::complex<double> bn_0{pssp::butterworth_laplace(pssp::butterworth_coeffs(n), s)};
            REQUIRE_THAT(bn_0.real(), Catch::Matchers::WithinAbs(0.0, 1e-10));
            REQUIRE_THAT(bn_0.imag(), Catch::Matchers::WithinAbs(1.4142135624, 1e-10));
        }
    }
}

TEST_CASE("Sampling Logspace")
{
    SECTION("SPower: 0.0, EPower: 1.0, N_Samples: 10")
    {
        const std::vector<double> logspace{pssp::logspace(0.0, 1.0, 10)};
        REQUIRE(logspace.size() == 10);
        REQUIRE_THAT(logspace[0], Catch::Matchers::WithinAbs(1.0, 1e-10));
        REQUIRE_THAT(logspace[9], Catch::Matchers::WithinAbs(10.0, 1e-10));
    }
    SECTION("SPower: 0.0, EPower: 2.0, N_Samples: 10")
    {
        const std::vector<double> logspace{pssp::logspace(0.0, 2.0, 10)};
        REQUIRE(logspace.size() == 10);
        REQUIRE_THAT(logspace[0], Catch::Matchers::WithinAbs(1.0, 1e-10));
        REQUIRE_THAT(logspace[9], Catch::Matchers::WithinAbs(100.0, 1e-10));
    }
    SECTION("SPower: -1.0, EPower: 0.0, N_Samples: 10")
    {
        const std::vector<double> logspace{pssp::logspace(-1.0, 0.0, 10)};
        REQUIRE(logspace.size() == 10);
        REQUIRE_THAT(logspace[0], Catch::Matchers::WithinAbs(0.1, 1e-10));
        REQUIRE_THAT(logspace[9], Catch::Matchers::WithinAbs(1.0, 1e-10));
    }
    SECTION("SPower: -2.0, EPower: 0.0, N_Samples: 10")
    {
        const std::vector<double> logspace{pssp::logspace(-2.0, 0.0, 10)};
        REQUIRE(logspace.size() == 10);
        REQUIRE_THAT(logspace[0], Catch::Matchers::WithinAbs(0.01, 1e-10));
        REQUIRE_THAT(logspace[9], Catch::Matchers::WithinAbs(1.0, 1e-10));
    }
    SECTION("SPower: -1.0, EPower: 1.0, N_Samples: 10")
    {
        const std::vector<double> logspace{pssp::logspace(-1.0, 1.0, 10)};
        REQUIRE(logspace.size() == 10);
        REQUIRE_THAT(logspace[0], Catch::Matchers::WithinAbs(0.1, 1e-10));
        REQUIRE_THAT(logspace[9], Catch::Matchers::WithinAbs(10.0, 1e-10));
    }
    SECTION("SPower: -2.0, EPower: 2.0, N_Samples: 10")
    {
        const std::vector<double> logspace{pssp::logspace(-2.0, 2.0, 10)};
        REQUIRE(logspace.size() == 10);
        REQUIRE_THAT(logspace[0], Catch::Matchers::WithinAbs(0.01, 1e-10));
        REQUIRE_THAT(logspace[9], Catch::Matchers::WithinAbs(100.0, 1e-10));
    }
    SECTION("SPower: -2.0, EPower: 2.0, N_Samples: 1000")
    {
        const std::vector<double> logspace{pssp::logspace(-2.0, 2.0, 1000)};
        REQUIRE(logspace.size() == 1000);
        REQUIRE_THAT(logspace[0], Catch::Matchers::WithinAbs(0.01, 1e-10));
        REQUIRE_THAT(logspace[999], Catch::Matchers::WithinAbs(100.0, 1e-10));
    }
}