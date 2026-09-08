#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/tuple.h>

#include <fe/Image.hpp>

namespace nb = nanobind;
using namespace nb::literals;

namespace fe_py{

namespace{
    using RgbaTuple = std::tuple<int, int, int, int>;

    RgbaTuple colorToTuple(const fe::Color& c){
        return {c.r, c.g, c.b, c.a};
    }
}

void bind_image(nb::module_& m){
    nb::class_<fe::Image>(m, "Image",
        "A simple RGBA pixel buffer - what Flower.petals.image and every "
        "draw_*()/make_*() call's rendered output is. Pixel colors are "
        "plain (r, g, b, a) tuples of 0-255 ints, not a separate bound "
        "Color type.")
        .def(nb::init<>(), "Starts unallocated (width/height 0) -- call "
             "create() before using it, or just use a rendered Image "
             "returned from draw_flower()/Flower.petals.image instead of "
             "building one by hand.")
        .def("create", [](fe::Image& img, std::size_t width, std::size_t height, RgbaTuple color){
                img.create(width, height, fe::Color(
                    static_cast<std::uint8_t>(std::get<0>(color)),
                    static_cast<std::uint8_t>(std::get<1>(color)),
                    static_cast<std::uint8_t>(std::get<2>(color)),
                    static_cast<std::uint8_t>(std::get<3>(color))
                ));
            }, "width"_a, "height"_a, "color"_a,
            "Allocates a width x height buffer, filled with color (r, g, b, a).")
        .def("set_pixel", [](fe::Image& img, std::size_t x, std::size_t y, RgbaTuple color){
                img.setPixel(x, y, fe::Color(
                    static_cast<std::uint8_t>(std::get<0>(color)),
                    static_cast<std::uint8_t>(std::get<1>(color)),
                    static_cast<std::uint8_t>(std::get<2>(color)),
                    static_cast<std::uint8_t>(std::get<3>(color))
                ));
            }, "x"_a, "y"_a, "color"_a,
            "Sets the pixel at (x, y) to color (r, g, b, a). Out-of-range "
            "x/y are silently ignored (matches the C++ API -- see setPixel's "
            "own bounds-check comment in src/fe/Image.cpp).")
        .def("get_pixel", [](const fe::Image& img, std::size_t x, std::size_t y){
                return colorToTuple(img.getPixel(x, y));
            }, "x"_a, "y"_a,
            "Returns the (r, g, b, a) color at (x, y); (0, 0, 0, 0) if "
            "out of range.")
        .def_prop_ro("width", [](const fe::Image& img){ return img.mWidth; })
        .def_prop_ro("height", [](const fe::Image& img){ return img.mHeight; })
        .def("to_png_bytes", [](const fe::Image& img){
                auto png = fe::encodeImageToPngInMemory(img);
                return nb::bytes(reinterpret_cast<const char*>(png.data()), png.size());
            },
            "Encodes to an in-memory PNG and returns it as bytes -- "
            "e.g. `pathlib.Path('flower.png').write_bytes(image.to_png_bytes())`.")
        .def("__repr__", [](const fe::Image& img){
            return "Image(width=" + std::to_string(img.mWidth) +
                   ", height=" + std::to_string(img.mHeight) + ")";
        });
}

}
