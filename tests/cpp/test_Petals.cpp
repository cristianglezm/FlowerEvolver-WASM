#include <catch2/catch_test_macros.hpp>

#include <fe/Petals.hpp>

namespace {
    bool hasAnyNonTransparentPixel(const fe::Image& img){
        for(std::size_t i = 3; i < img.imageData.size(); i += 4){
            if(img.imageData[i] != 0){
                return true;
            }
        }
        return false;
    }
}

TEST_CASE("getTimesDivisibleBy counts halvings until <= 1", "[Petals]"){
    REQUIRE(fe::getTimesDivisibleBy(64, 2) == 6);   // 64->32->16->8->4->2->1
    REQUIRE(fe::getTimesDivisibleBy(1, 2) == 0);
    REQUIRE(fe::getTimesDivisibleBy(0, 2) == 0);
    REQUIRE(fe::getTimesDivisibleBy(100, 3) == 4);  // 100->33->11->3->1
    REQUIRE(fe::getTimesDivisibleBy(-64, 2) == 6);  // abs()'d internally
}

TEST_CASE("getTimesDivisibleBy returns 1 for a non-dividing divisor (<=1), by design", "[Petals]"){
    REQUIRE(fe::getTimesDivisibleBy(5, 0) == 1);
    REQUIRE(fe::getTimesDivisibleBy(5, 1) == 1);
}

TEST_CASE("default constructor matches the documented defaults", "[Petals]"){
    fe::Petals p;
    REQUIRE(p.radius == 64);
    REQUIRE(p.numLayers == 3);
    REQUIRE(p.P == 6.0f);
    REQUIRE(p.bias == 1.0f);
    REQUIRE_FALSE(p.hasBloom);
}

TEST_CASE("sized constructor clamps radius/numLayers and allocates a radius*2 x radius*3 canvas", "[Petals]"){
    fe::Petals p(64, 3, 6.0f, 1.0f);
    REQUIRE(p.radius == 64);
    auto size = p.image.getSize();
    REQUIRE(size.x == 128);
    REQUIRE(size.y == 192);

    SECTION("radius is clamped to [4, 256]"){
        fe::Petals tooSmall(1, 1, 6.0f, 1.0f);
        REQUIRE(tooSmall.radius == 4);
        fe::Petals tooBig(9999, 1, 6.0f, 1.0f);
        REQUIRE(tooBig.radius == 256);
    }
    SECTION("numLayers is clamped to [1, getTimesDivisibleBy(radius, 2)]"){
        fe::Petals tooManyLayers(64, 9999, 6.0f, 1.0f);
        REQUIRE(tooManyLayers.numLayers == fe::getTimesDivisibleBy(64, 2));
        fe::Petals tooFewLayers(64, 0, 6.0f, 1.0f);
        REQUIRE(tooFewLayers.numLayers == 1);
    }
}

TEST_CASE("toJson / Petals(JsonBox::Object) round-trips the numeric fields", "[Petals]"){
    fe::Petals p(64, 3, 5.5f, -1.0f);
    fe::Petals fromJson(p.toJson().getObject());
    REQUIRE(fromJson.radius == p.radius);
    REQUIRE(fromJson.numLayers == p.numLayers);
    REQUIRE(fromJson.P == p.P);
    REQUIRE(fromJson.bias == p.bias);
}

TEST_CASE("draw(TrunkAndPetals, ...) paints into the canvas", "[Petals]"){
    fe::Petals p(32, 2, 6.0f, 1.0f);
    auto g = EvoAI::Genome(4, 4, false, true);
    fe::draw(fe::Petals::Type::TrunkAndPetals, p, g);
    REQUIRE(hasAnyNonTransparentPixel(p.image));
}

TEST_CASE("drawLayer paints into the canvas for a valid layer", "[Petals]"){
    fe::Petals p(32, 2, 6.0f, 1.0f);
    auto g = EvoAI::Genome(4, 4, false, true);
    fe::drawLayer(p, g, 0);
    REQUIRE(hasAnyNonTransparentPixel(p.image));
}

TEST_CASE("draw(Trunk, ...) paints the stem in a color matching the 3D stem material", "[Petals]"){
    fe::Petals p(32, 2, 6.0f, 1.0f);
    auto g = EvoAI::Genome(4, 4, false, true);
    fe::draw(fe::Petals::Type::Trunk, p, g);

    auto stemPixel = p.image.getPixel(static_cast<std::size_t>(p.radius), static_cast<std::size_t>(p.radius));
    REQUIRE(stemPixel == fe::Color(51, 153, 51));
}
