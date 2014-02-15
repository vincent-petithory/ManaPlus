/*
 *  The ManaPlus Client
 *  Copyright (C) 2011-2014  The ManaPlus Developers
 *
 *  This file is part of The ManaPlus Client.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*      _______   __   __   __   ______   __   __   _______   __   __
 *     / _____/\ / /\ / /\ / /\ / ____/\ / /\ / /\ / ___  /\ /  |\/ /\
 *    / /\____\// / // / // / // /\___\// /_// / // /\_/ / // , |/ / /
 *   / / /__   / / // / // / // / /    / ___  / // ___  / // /| ' / /
 *  / /_// /\ / /_// / // / // /_/_   / / // / // /\_/ / // / |  / /
 * /______/ //______/ //_/ //_____/\ /_/ //_/ //_/ //_/ //_/ /|_/ /
 * \______\/ \______\/ \_\/ \_____\/ \_\/ \_\/ \_\/ \_\/ \_\/ \_\/
 *
 * Copyright (c) 2004 - 2008 Olof Naessén and Per Larsson
 *
 *
 * Per Larsson a.k.a finalman
 * Olof Naessén a.k.a jansem/yakslem
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

#ifndef GCN_KEYINPUT_HPP
#define GCN_KEYINPUT_HPP

#include "gui/base/key.hpp"

namespace gcn
{
    /**
     * Internal class that represents key input. Generally you won't have to
     * bother using this class unless you implement an Input class for
     * a back end.
     *
     * @since 0.1.0
     */
    class KeyInput
    {
    public:
        /**
         * Constructor.
         */
        KeyInput() :
            mKey(0),
            mType(0),
            mShiftPressed(false),
            mControlPressed(false),
            mAltPressed(false),
            mMetaPressed(false),
            mNumericPad(false)
        { }

        /**
         * Constructor.
         *
         * @param key The key of the key input.
         * @param type The type of key input.
         */
        KeyInput(const Key& key, unsigned int type);

        /**
         * Sets the type of the key input.
         *
         * @param type The type of key input.
         * @see getType
         */
        void setType(unsigned int type);

        /**
         * Gets the type of the key input.
         *
         * @return the input type.
         * @see setType
         */
        int getType() const A_WARN_UNUSED;

        /**
         * Sets the key of the key input.
         *
         * @param key The key of the key input.
         * @see getKey
         */
        void setKey(const Key& key);

        /**
         * Gets the key of the key input.
         *
         * @return The key of the key input.
         * @see setKey
         */
        const Key& getKey() const A_WARN_UNUSED;

        /**
         * Checks if shift is pressed.
         *
         * @return True if shift was pressed at the same 
         *         time as the key, false otherwise.
         * @see setShiftPressed
         * @since 0.6.0
         */
        bool isShiftPressed() const A_WARN_UNUSED;

        /**
         * Sets shift to be pressed at the same time as the key, 
         * or not. 
         *
         * @param pressed True if shift is pressed, false otherwise.
         * @see isShiftPressed
         * @since 0.6.0
         */
        void setShiftPressed(bool pressed);

        /**
         * Checks if control is pressed.
         *
         * @return True if control was pressed at the same 
         *         time as the key, false otherwise.
         * @see setControlPressed
         * @since 0.6.0
         */
        bool isControlPressed() const A_WARN_UNUSED;

        /**
         * Sets control to be pressed at the same time as the key, 
         * or not. 
         *
         * @param pressed True if control is pressed, false otherwise.
         * @see isControlPressed
         * @since 0.6.0
         */
        void setControlPressed(bool pressed);

        /**
         * Checks if alt is pressed.
         *
         * @return True if alt was pressed at the same 
         *         time as the key, false otherwise.
         * @see setAltPressed
         * @since 0.6.0
         */
        bool isAltPressed() const;

        /**
         * Sets the alt to be pressed at the same time as the key, 
         * or not. 
         *
         * @param pressed True if alt is pressed at the same 
         *                time as the key, , false otherwise.
         * @see isAltPressed
         * @since 0.6.0
         */
        void setAltPressed(bool pressed);

        /**
         * Checks if meta is pressed.
         *
         * @return True if meta was pressed at the same 
         *         time as the key, false otherwise.
         * @see setMetaPressed
         * @since 0.6.0
         */
        bool isMetaPressed() const A_WARN_UNUSED;

        /**
         * Sets meta to be pressed at the same time as the key, 
         * or not. 
         *
         * @param pressed True if meta is pressed at the same
         *                time as the key, false otherwise.
         * @see isMetaPressed
         * @since 0.6.0
         */
        void setMetaPressed(bool pressed);

        /**
         * Checks if the key was pressed at the numeric pad.
         *
         * @return True if key pressed at the numeric pad,
         *         false otherwise.
         * @setNumericPad
         * @since 0.6.0
         */
        bool isNumericPad() const A_WARN_UNUSED;

        /**
         * Sets the key to be pressed at the numeric pad.
         *
         * @param numpad True if the key was pressed at the numeric
         *               pad, false otherwise.
         * @see isNumericPad
         * @since 0.6.0
         */
        void setNumericPad(bool numpad);

        /**
         * Key input types. This enum corresponds to the enum with event
         * types on KeyEvent for easy mapping.
         */
        enum
        {
            PRESSED = 0,
            RELEASED
        };

    protected:
        /**
         * Holds the key of the key input.
         */
        Key mKey;

        /**
         * Holds the type of the key input.
         */
        unsigned int mType;

        /**
         * True if shift was pressed at the same time as the key,
         * false otherwise.
         */ 
        bool mShiftPressed;

        /**
         * True if control was pressed at the same time as the key,
         * false otherwise.
         */ 
        bool mControlPressed;

        /**
         * True if alt was pressed at the same time as the key,
         * false otherwise.
         */ 
        bool mAltPressed;

        /**
         * True if meta was pressed at the same time as the key,
         * false otherwise.
         */ 
        bool mMetaPressed;

        /**
         * True if the numeric pad was used when the key was pressed,
         * false otherwise.
         */ 
        bool mNumericPad;
    };
}  // namespace gcn

#endif  // end GCN_KEYINPUT_HPP