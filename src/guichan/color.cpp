/*      _______   __   __   __   ______   __   __   _______   __   __
 *     / _____/\ / /\ / /\ / /\ / ____/\ / /\ / /\ / ___  /\ /  |\/ /\
 *    / /\____\// / // / // / // /\___\// /_// / // /\_/ / // , |/ / /
 *   / / /__   / / // / // / // / /    / ___  / // ___  / // /| ' / /
 *  / /_// /\ / /_// / // / // /_/_   / / // / // /\_/ / // / |  / /
 * /______/ //______/ //_/ //_____/\ /_/ //_/ //_/ //_/ //_/ /|_/ /
 * \______\/ \______\/ \_\/ \_____\/ \_\/ \_\/ \_\/ \_\/ \_\/ \_\/
 *
 * Copyright (c) 2004 - 2008 Olof Naess�n and Per Larsson
 * Copyright (C) 2011-2012  The ManaPlus Developers
 *
 *
 * Per Larsson a.k.a finalman
 * Olof Naess�n a.k.a jansem/yakslem
 *
 * Visit: http://guichan.sourceforge.net
 *
 * License: (BSD)
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of Guichan nor the names of its contributors may
 *    be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * For comments regarding functions please see the header file.
 */

#include "guichan/color.hpp"

#include "debug.h"

namespace gcn
{
    Color::Color() :
        r(0),
        g(0),
        b(0),
        a(255)
    {
    }

    Color::Color(const int color) :
        r((color >> 16) & 0xFF),
        g((color >>  8) & 0xFF),
        b(color         & 0xFF),
        a(255)
    {
    }

    Color::Color(const int ar, const int ag, const int ab, const int aa) :
        r(ar),
        g(ag),
        b(ab),
        a(aa)
    {
    }

    Color Color::operator+(const Color& color) const
    {
        Color result(r + color.r,
                     g + color.g,
                     b + color.b,
                     255);

        result.r = (result.r>255?255:(result.r<0?0:result.r));
        result.g = (result.g>255?255:(result.g<0?0:result.g));
        result.b = (result.b>255?255:(result.b<0?0:result.b));

        return result;
    }

    Color Color::operator-(const Color& color) const
    {
        Color result(r - color.r, 
                     g - color.g, 
                     b - color.b, 
                     255);

        result.r = (result.r > 255 ? 255 : (result.r < 0 ? 0 : result.r));
        result.g = (result.g > 255 ? 255 : (result.g < 0 ? 0 : result.g));
        result.b = (result.b > 255 ? 255 : (result.b < 0 ? 0 : result.b));

        return result;
    }

    Color Color::operator*(const float value) const
    {
        Color result(static_cast<int>(r * value), 
                     static_cast<int>(g * value), 
                     static_cast<int>(b * value), 
                     a);

        result.r = (result.r > 255 ? 255 : (result.r < 0 ? 0 : result.r));
        result.g = (result.g > 255 ? 255 : (result.g < 0 ? 0 : result.g));
        result.b = (result.b > 255 ? 255 : (result.b < 0 ? 0 : result.b));

        return result;
    }

    bool Color::operator==(const Color& color) const
    {
        return r == color.r && g == color.g && b == color.b && a == color.a;
    }

    bool Color::operator!=(const Color& color) const
    {
        return !(r == color.r && g == color.g && b == color.b && a == color.a);
    }

    std::ostream& operator<<(std::ostream& out,
                             const Color& color)
    {
        out << "Color [r = "
            << color.r
            << ", g = "
            << color.g
            << ", b = "
            << color.b
            << ", a = "
            << color.a
            << "]";

        return out;
    }
}
