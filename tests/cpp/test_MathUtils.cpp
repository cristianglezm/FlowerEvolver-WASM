#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <fe/MathUtils.hpp>

using Catch::Matchers::WithinAbs;

TEST_CASE("magnitude", "[MathUtils]"){
    REQUIRE_THAT(fe::Math::magnitude({0.f, 0.f}), WithinAbs(0.f, 1e-5f));
    REQUIRE_THAT(fe::Math::magnitude({3.f, 4.f}), WithinAbs(5.f, 1e-5f));
    REQUIRE_THAT(fe::Math::magnitude({-3.f, -4.f}), WithinAbs(5.f, 1e-5f));
}

TEST_CASE("normalize", "[MathUtils]"){
    SECTION("zero vector stays zero instead of dividing by zero"){
        auto n = fe::Math::normalize({0.f, 0.f});
        REQUIRE_THAT(n.x, WithinAbs(0.f, 1e-5f));
        REQUIRE_THAT(n.y, WithinAbs(0.f, 1e-5f));
    }
    SECTION("result always has unit magnitude"){
        auto n = fe::Math::normalize({3.f, 4.f});
        REQUIRE_THAT(n.x, WithinAbs(0.6f, 1e-5f));
        REQUIRE_THAT(n.y, WithinAbs(0.8f, 1e-5f));
        REQUIRE_THAT(fe::Math::magnitude(n), WithinAbs(1.f, 1e-5f));
    }
}

TEST_CASE("degreesToRadians / radiansToDegrees round-trip", "[MathUtils]"){
    REQUIRE_THAT(fe::Math::degreesToRadians(180.f), WithinAbs(static_cast<float>(fe::Math::PI), 1e-4f));
    REQUIRE_THAT(fe::Math::degreesToRadians(0.f), WithinAbs(0.f, 1e-5f));
    for(float deg : {0.f, 33.f, 90.f, 180.f, 270.f, 315.31f}){
        auto roundTripped = fe::Math::radiansToDegrees(fe::Math::degreesToRadians(deg));
        REQUIRE_THAT(roundTripped, WithinAbs(deg, 1e-3f));
    }
}

TEST_CASE("angle matches the worked examples documented in MathUtils.hpp", "[MathUtils]"){
    // "angle is 281.31deg from 0,0 to 5,1 in clockwise direction"
    auto a1 = fe::Math::radiansToDegrees(fe::Math::angle({0.f, 0.f}, {5.f, 1.f}));
    REQUIRE_THAT(a1, WithinAbs(281.31f, 0.05f));
    // "angle is 101.31deg from 5,1 to 0,0 in clockwise direction"
    auto a2 = fe::Math::radiansToDegrees(fe::Math::angle({5.f, 1.f}, {0.f, 0.f}));
    REQUIRE_THAT(a2, WithinAbs(101.31f, 0.05f));
}

TEST_CASE("directedAngle matches the worked examples documented in MathUtils.hpp", "[MathUtils]"){
    auto a1 = fe::Math::radiansToDegrees(fe::Math::directedAngle({5.f, 1.f}, {3.f, 3.f}, {0.f, 0.f}));
    REQUIRE_THAT(a1, WithinAbs(326.31f, 0.05f));
    auto a2 = fe::Math::radiansToDegrees(fe::Math::directedAngle({3.f, 3.f}, {5.f, 1.f}, {0.f, 0.f}));
    REQUIRE_THAT(a2, WithinAbs(33.6901f, 0.05f));
    auto a3 = fe::Math::radiansToDegrees(fe::Math::directedAngle({5.f, 1.f}, {3.f, 3.f}, {3.f, 3.f}));
    REQUIRE_THAT(a3, WithinAbs(315.f, 0.05f));
}
