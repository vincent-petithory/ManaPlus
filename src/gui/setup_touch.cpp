/*
 *  The ManaPlus Client
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

#include "gui/setup_touch.h"

#include "gui/widgets/layouthelper.h"
#include "gui/widgets/scrollarea.h"

#include "configuration.h"

#include "utils/gettext.h"

#include "debug.h"

static const int sizeListSize = 4;

static const char *const sizeList[] =
{
    N_("Small"),
    N_("Normal"),
    N_("Medium"),
    N_("Large")
};

Setup_Touch::Setup_Touch(const Widget2 *const widget) :
    SetupTabScroll(widget),
    mSizeList(new NamesModel),
    mActionsList(new TouchActionsModel)
{
    setName(_("Touch"));

    LayoutHelper h(this);
    ContainerPlacer place = h.getPlacer(0, 0);
    place(0, 0, mScroll, 10, 10);
    mSizeList->fillFromArray(&sizeList[0], sizeListSize);

    new SetupItemLabel(_("Onscreen keyboard"), "", this);

    new SetupItemCheckBox(_("Show onscreen keyboard icon"), "",
        "showScreenKeyboard", this, "showScreenKeyboardEvent");

    new SetupActionDropDown(_("Keyboard icon action"), "",
        "screenActionKeyboard", this, "screenActionKeyboardEvent",
        mActionsList, 250);


    new SetupItemLabel(_("Onscreen joystick"), "", this);

    new SetupItemCheckBox(_("Show onscreen joystick"), "",
        "showScreenJoystick", this, "showScreenJoystickEvent");

    new SetupItemDropDown(_("Joystick size"), "", "screenJoystickSize", this,
        "screenJoystickEvent", mSizeList, 100);


    new SetupItemLabel(_("Onscreen buttons"), "", this);

    new SetupItemCheckBox(_("Show onscreen buttons"), "",
        "showScreenButtons", this, "showScreenButtonsEvent");

    new SetupItemDropDown(_("Buttons size"), "", "screenButtonsSize", this,
        "screenButtonsSizeEvent", mSizeList, 100);

    new SetupActionDropDown(strprintf(_("Button %u action"), 1), "",
        "screenActionButton0", this, "screenActionButton0Event",
        mActionsList, 250);

    new SetupActionDropDown(strprintf(_("Button %u action"), 2), "",
        "screenActionButton1", this, "screenActionButton1Event",
        mActionsList, 250);

    setDimension(gcn::Rectangle(0, 0, 550, 350));
}

Setup_Touch::~Setup_Touch()
{
    delete mSizeList;
    mSizeList = nullptr;
}
