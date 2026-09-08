////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2023 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

#ifndef FE_COLOR_HPP
#define FE_COLOR_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <fe/config.hpp>


// from SFML/Config.hpp
namespace fe
{
    // All "common" platforms use the same size for char, short and int
    // (basically there are 3 types for 3 sizes, so no other match is possible),
    // we can use them without doing any kind of check

    // 8 bits integer types
    typedef signed   char Int8;
    typedef unsigned char Uint8;

    // 16 bits integer types
    typedef signed   short Int16;
    typedef unsigned short Uint16;

    // 32 bits integer types
    typedef signed   int Int32;
    typedef unsigned int Uint32;

    // 64 bits integer types
    #if defined(_MSC_VER)
        typedef signed   __int64 Int64;
        typedef unsigned __int64 Uint64;
    #else
        #if defined(__clang__)
            #pragma clang diagnostic push
            #pragma clang diagnostic ignored "-Wc++11-long-long"
        #endif
        typedef signed   long long Int64;
        typedef unsigned long long Uint64;
        #if defined(__clang__)
            #pragma clang diagnostic pop
        #endif
    #endif

} // namespace fe

namespace fe
{
////////////////////////////////////////////////////////////
/// \brief Utility class for manipulating RGBA colors
///
////////////////////////////////////////////////////////////
class FE_API Color
{
public:

    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    /// Constructs an opaque black color. It is equivalent to
    /// fe::Color(0, 0, 0, 255).
    ///
    ////////////////////////////////////////////////////////////
    Color();

    ////////////////////////////////////////////////////////////
    /// \brief Construct the color from its 4 RGBA components
    ///
    /// \param red   Red component (in the range [0, 255])
    /// \param green Green component (in the range [0, 255])
    /// \param blue  Blue component (in the range [0, 255])
    /// \param alpha Alpha (opacity) component (in the range [0, 255])
    ///
    ////////////////////////////////////////////////////////////
    Color(Uint8 red, Uint8 green, Uint8 blue, Uint8 alpha = 255);

    ////////////////////////////////////////////////////////////
    /// \brief Construct the color from 32-bit unsigned integer
    ///
    /// \param color Number containing the RGBA components (in that order)
    ///
    ////////////////////////////////////////////////////////////
    explicit Color(Uint32 color);

    ////////////////////////////////////////////////////////////
    /// \brief Retrieve the color as a 32-bit unsigned integer
    ///
    /// \return Color represented as a 32-bit unsigned integer
    ///
    ////////////////////////////////////////////////////////////
    Uint32 toInteger() const;

    ////////////////////////////////////////////////////////////
    // Static member data
    ////////////////////////////////////////////////////////////
    static const Color Black;       //!< Black predefined color
    static const Color White;       //!< White predefined color
    static const Color Red;         //!< Red predefined color
    static const Color Green;       //!< Green predefined color
    static const Color Blue;        //!< Blue predefined color
    static const Color Yellow;      //!< Yellow predefined color
    static const Color Magenta;     //!< Magenta predefined color
    static const Color Cyan;        //!< Cyan predefined color
    static const Color Transparent; //!< Transparent (black) predefined color

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    Uint8 r; //!< Red component
    Uint8 g; //!< Green component
    Uint8 b; //!< Blue component
    Uint8 a; //!< Alpha (opacity) component
};

////////////////////////////////////////////////////////////
/// \relates Color
/// \brief Overload of the == operator
///
/// This operator compares two colors and check if they are equal.
///
/// \param left  Left operand
/// \param right Right operand
///
/// \return True if colors are equal, false if they are different
///
////////////////////////////////////////////////////////////
FE_API bool operator ==(const Color& left, const Color& right);

////////////////////////////////////////////////////////////
/// \relates Color
/// \brief Overload of the != operator
///
/// This operator compares two colors and check if they are different.
///
/// \param left  Left operand
/// \param right Right operand
///
/// \return True if colors are different, false if they are equal
///
////////////////////////////////////////////////////////////
FE_API bool operator !=(const Color& left, const Color& right);

////////////////////////////////////////////////////////////
/// \relates Color
/// \brief Overload of the binary + operator
///
/// This operator returns the component-wise sum of two colors.
/// Components that exceed 255 are clamped to 255.
///
/// \param left  Left operand
/// \param right Right operand
///
/// \return Result of \a left + \a right
///
////////////////////////////////////////////////////////////
FE_API Color operator +(const Color& left, const Color& right);

////////////////////////////////////////////////////////////
/// \relates Color
/// \brief Overload of the binary - operator
///
/// This operator returns the component-wise subtraction of two colors.
/// Components below 0 are clamped to 0.
///
/// \param left  Left operand
/// \param right Right operand
///
/// \return Result of \a left - \a right
///
////////////////////////////////////////////////////////////
FE_API Color operator -(const Color& left, const Color& right);

////////////////////////////////////////////////////////////
/// \relates Color
/// \brief Overload of the binary * operator
///
/// This operator returns the component-wise multiplication
/// (also called "modulation") of two colors.
/// Components are then divided by 255 so that the result is
/// still in the range [0, 255].
///
/// \param left  Left operand
/// \param right Right operand
///
/// \return Result of \a left * \a right
///
////////////////////////////////////////////////////////////
FE_API Color operator *(const Color& left, const Color& right);

////////////////////////////////////////////////////////////
/// \relates Color
/// \brief Overload of the binary += operator
///
/// This operator computes the component-wise sum of two colors,
/// and assigns the result to the left operand.
/// Components that exceed 255 are clamped to 255.
///
/// \param left  Left operand
/// \param right Right operand
///
/// \return Reference to \a left
///
////////////////////////////////////////////////////////////
FE_API Color& operator +=(Color& left, const Color& right);

////////////////////////////////////////////////////////////
/// \relates Color
/// \brief Overload of the binary -= operator
///
/// This operator computes the component-wise subtraction of two colors,
/// and assigns the result to the left operand.
/// Components below 0 are clamped to 0.
///
/// \param left  Left operand
/// \param right Right operand
///
/// \return Reference to \a left
///
////////////////////////////////////////////////////////////
FE_API Color& operator -=(Color& left, const Color& right);

////////////////////////////////////////////////////////////
/// \relates Color
/// \brief Overload of the binary *= operator
///
/// This operator returns the component-wise multiplication
/// (also called "modulation") of two colors, and assigns
/// the result to the left operand.
/// Components are then divided by 255 so that the result is
/// still in the range [0, 255].
///
/// \param left  Left operand
/// \param right Right operand
///
/// \return Reference to \a left
///
////////////////////////////////////////////////////////////
FE_API Color& operator *=(Color& left, const Color& right);

} // namespace fe


#endif // FE_COLOR_HPP


////////////////////////////////////////////////////////////
/// \class fe::Color
/// \ingroup graphics
///
/// fe::Color is a simple color class composed of 4 components:
/// \li Red
/// \li Green
/// \li Blue
/// \li Alpha (opacity)
///
/// Each component is a public member, an unsigned integer in
/// the range [0, 255]. Thus, colors can be constructed and
/// manipulated very easily:
///
/// \code
/// fe::Color color(255, 0, 0); // red
/// color.r = 0;                // make it black
/// color.b = 128;              // make it dark blue
/// \endcode
///
/// The fourth component of colors, named "alpha", represents
/// the opacity of the color. A color with an alpha value of
/// 255 will be fully opaque, while an alpha value of 0 will
/// make a color fully transparent, whatever the value of the
/// other components is.
///
/// The most common colors are already defined as static variables:
/// \code
/// fe::Color black       = fe::Color::Black;
/// fe::Color white       = fe::Color::White;
/// fe::Color red         = fe::Color::Red;
/// fe::Color green       = fe::Color::Green;
/// fe::Color blue        = fe::Color::Blue;
/// fe::Color yellow      = fe::Color::Yellow;
/// fe::Color magenta     = fe::Color::Magenta;
/// fe::Color cyan        = fe::Color::Cyan;
/// fe::Color transparent = fe::Color::Transparent;
/// \endcode
///
/// Colors can also be added and modulated (multiplied) using the
/// overloaded operators + and *.
///
////////////////////////////////////////////////////////////
