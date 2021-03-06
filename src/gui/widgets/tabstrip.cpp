/*
 *  The ManaPlus Client
 *  Copyright (C) 2012-2014  The ManaPlus Developers
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

#include "gui/widgets/tabstrip.h"

#include "gui/widgets/button.h"

#include "debug.h"

TabStrip::TabStrip(const Widget2 *const widget,
                   const std::string &group,
                   const int height,
                   const int spacing) :
    WidgetGroup(widget, group, height, spacing)
{
    mAllowLogic = false;
}

TabStrip::TabStrip(const Widget2 *const widget,
                   const int height,
                   const int spacing) :
    WidgetGroup(widget, "", height, spacing)
{
    mAllowLogic = false;
}

Widget *TabStrip::createWidget(const std::string &text) const
{
    Button *const widget = new Button(this);
    widget->setStick(true);
    widget->setCaption(text);
    widget->adjustSize();
    if (!mCount)
        widget->setPressed(true);
    return widget;
}

void TabStrip::action(const ActionEvent &event)
{
    WidgetGroup::action(event);
    if (event.getSource())
    {
        const Widget *const widget = event.getSource();
        if (static_cast<const Button*>(widget)->isPressed2())
        {
            FOR_EACH (WidgetListConstIterator, iter, mWidgets)
            {
                if (*iter != widget)
                {
                    Button *const button = static_cast<Button*>(*iter);
                    button->setPressed(false);
                }
            }
        }
    }
}
