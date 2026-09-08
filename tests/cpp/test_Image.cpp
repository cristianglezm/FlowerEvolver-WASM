#include <catch2/catch_test_macros.hpp>

#include <fe/Image.hpp>

#include "test_helpers.hpp"

TEST_CASE("create allocates a transparent buffer of the requested size", "[Image]"){
    fe::Image img;
    img.create(4, 3, fe::Color::Transparent);

    auto size = img.getSize();
    REQUIRE(size.x == 4);
    REQUIRE(size.y == 3);
    REQUIRE(img.imageData.size() == 4u * 3u * 4u); // RGBA per pixel

    for(std::size_t y = 0; y < 3; ++y){
        for(std::size_t x = 0; x < 4; ++x){
            auto px = img.getPixel(x, y);
            REQUIRE(px.a == 0);
        }
    }
}

TEST_CASE("setPixel/getPixel round-trip a color", "[Image]"){
    fe::Image img;
    img.create(2, 2, fe::Color::Transparent);

    img.setPixel(1u, 0u, fe::Color(10, 20, 30, 255));
    auto px = img.getPixel(1, 0);
    REQUIRE(px.r == 10);
    REQUIRE(px.g == 20);
    REQUIRE(px.b == 30);
    REQUIRE(px.a == 255);

    // untouched pixels stay whatever create() set them to.
    auto untouched = img.getPixel(0, 0);
    REQUIRE(untouched.a == 0);
}

TEST_CASE("getPixel is bounds-safe and returns a default color out of range", "[Image]"){
    fe::Image img;
    img.create(2, 2, fe::Color::Transparent);
    auto px = img.getPixel(50, 50);
    REQUIRE(px == fe::Color());
}

TEST_CASE("getPixel does not alias x == mWidth into the next row", "[Image]"){
    fe::Image img;
    img.create(4, 4, fe::Color::Transparent);
    img.setPixel(0u, 1u, fe::Color(9, 9, 9, 9)); // row 1's first pixel
    REQUIRE(img.getPixel(4u, 0u) == fe::Color());
}

TEST_CASE("setPixel with a fe::Vector2f position matches the (x, y) overload", "[Image]"){
    fe::Image img;
    img.create(3, 3, fe::Color::Transparent);
    img.setPixel(fe::Vector2f(1.f, 1.f), fe::Color(1, 2, 3, 4));
    auto px = img.getPixel(1, 1);
    REQUIRE(px == fe::Color(1, 2, 3, 4));
}

TEST_CASE("setPixel is bounds-safe: out-of-range x/y must not write inside or outside imageData", "[Image]"){
    fe::Image img;
    img.create(4, 4, fe::Color::Transparent);
    auto before = img.imageData; // snapshot

    img.setPixel(4u, 0u, fe::Color(255, 255, 255, 255));  // x == mWidth on a non-last row -- would alias into row 1
    img.setPixel(0u, 4u, fe::Color(255, 255, 255, 255));  // y == mHeight, one past the last row
    img.setPixel(static_cast<std::size_t>(-1), 0u, fe::Color(255, 255, 255, 255)); // wrapped/huge x

    // none of the above should have touched any in-bounds pixel.
    fe_tests::requireBytesEqual(img.imageData, before);
}

TEST_CASE("encodeImageToPngInMemory produces a well-formed PNG byte stream", "[Image]"){
    fe::Image img;
    img.create(8, 8, fe::Color::White);

    auto png = fe::encodeImageToPngInMemory(img);
    REQUIRE_FALSE(png.empty());

    // PNG signature: 89 50 4E 47 0D 0A 1A 0A
    static const std::vector<std::uint8_t> pngSignature{0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    REQUIRE(png.size() >= pngSignature.size());
    REQUIRE(std::equal(pngSignature.begin(), pngSignature.end(), png.begin()));
}

TEST_CASE("encodeImageToPngInMemory handles an unallocated (0x0) image without crashing", "[Image]"){
    fe::Image img; // never create()'d
    auto png = fe::encodeImageToPngInMemory(img);
    static const std::vector<std::uint8_t> pngSignature{0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    REQUIRE(png.size() >= pngSignature.size());
    REQUIRE(std::equal(pngSignature.begin(), pngSignature.end(), png.begin()));
}

TEST_CASE("encodeToBase64 output is valid base64 (RFC 4648 alphabet + padding) and non-empty for real data", "[Image]"){
    fe::Image img;
    img.create(4, 4, fe::Color::Black);
    auto png = fe::encodeImageToPngInMemory(img);
    auto b64 = fe::encodeToBase64(png);

    REQUIRE_FALSE(b64.empty());
    // every char is in the base64 alphabet, with at most a trailing '=' pad.
    auto isB64Char = [](char c){
        return std::isalnum(static_cast<unsigned char>(c)) || c == '+' || c == '/';
    };
    std::size_t i = 0;
    for(; i < b64.size() && b64[i] != '='; ++i){
        REQUIRE(isB64Char(b64[i]));
    }
    for(; i < b64.size(); ++i){
        REQUIRE(b64[i] == '=');
    }
}

TEST_CASE("encodeToBase64 of empty input is empty", "[Image]"){
    REQUIRE(fe::encodeToBase64({}).empty());
}
