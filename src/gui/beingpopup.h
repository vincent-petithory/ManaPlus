/*
 *  The ManaPlus Client
 *  Copyright (C) 2010  The Mana Developers
 *  Copyright (C) 2011-2012  The ManaPlus Developers
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

#ifndef BEINGPOPUP_H
#define BEINGPOPUP_H

#include "gui/widgets/popup.h"

class Being;
class Label;

/**
 * A popup that displays information about a being.
 */
class BeingPopup : public Popup
{
    public:
        /**
         * Constructor. Initializes the being popup.
         */
        BeingPopup();

        /**
         * Destructor. Cleans up the being popup on deletion.
         */
        ~BeingPopup();

        /**
         * Sets the info to be displayed given a particular player.
         */
        void show(const int x, const int y, Being *const b);

        // TODO: Add a version for monsters, NPCs, etc?

    private:
        Label *mBeingName;
        Label *mBeingParty;
        Label *mBeingGuild;
        Label *mBeingRank;
        Label *mBeingComment;
};

#endif // BEINGPOPUP_H
