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

#ifndef GUI_WIDGETS_TABS_SOCIALFRIENDSTAB_H
#define GUI_WIDGETS_TABS_SOCIALFRIENDSTAB_H

#include "gui/widgets/tabs/socialtab.h"

#include "utils/delete2.h"
#include "utils/gettext.h"

#include "localconsts.h"

namespace
{
    static class SortFriendsFunctor final
    {
        public:
            bool operator() (const Avatar *const m1,
                             const Avatar *const m2) const
            {
                if (!m1 || !m2)
                    return false;

                if (m1->getOnline() != m2->getOnline())
                    return m1->getOnline() > m2->getOnline();

                if (m1->getName() != m2->getName())
                {
                    std::string s1 = m1->getName();
                    std::string s2 = m2->getName();
                    toLower(s1);
                    toLower(s2);
                    return s1 < s2;
                }
                return false;
            }
    } friendSorter;
}  // namespace

class SocialFriendsTab final : public SocialTab
{
    public:
        SocialFriendsTab(const Widget2 *const widget,
                         std::string name,
                         const bool showBackground) :
            SocialTab(widget),
            mBeings(new BeingsListModel)
        {
            mList = new AvatarListBox(this, mBeings);
            mList->postInit();
            mScroll = new ScrollArea(this, mList, showBackground,
                "social_background.xml");

            mScroll->setHorizontalScrollPolicy(ScrollArea::SHOW_AUTO);
            mScroll->setVerticalScrollPolicy(ScrollArea::SHOW_ALWAYS);

            getPlayersAvatars();
            setCaption(name);
        }

        A_DELETE_COPY(SocialFriendsTab)

        ~SocialFriendsTab()
        {
            delete2(mList)
            delete2(mScroll)
            delete2(mBeings)
        }

        void updateList() override final
        {
            getPlayersAvatars();
        }

        void getPlayersAvatars()
        {
            if (!actorManager)
                return;

            std::vector<Avatar*> *const avatars = mBeings->getMembers();
            if (!avatars)
                return;

            std::vector<Avatar*>::iterator ia = avatars->begin();
            while (ia != avatars->end())
            {
                delete *ia;
                ++ ia;
            }
            avatars->clear();

            const StringVect *const players
                = player_relations.getPlayersByRelation(
                PlayerRelation::FRIEND);

            const std::set<std::string> &players2
                = whoIsOnline->getOnlineNicks();

            if (!players)
                return;

            int online = 0;
            int total = 0;

            FOR_EACHP (StringVectCIter, it, players)
            {
                Avatar *const ava = new Avatar(*it);
                if (actorManager->findBeingByName(*it, ActorType::PLAYER)
                    || players2.find(*it) != players2.end())
                {
                    ava->setOnline(true);
                    online ++;
                }
                total ++;
                avatars->push_back(ava);
            }
            std::sort(avatars->begin(), avatars->end(), friendSorter);
            delete players;

            // TRANSLATORS: social window label
            mCounterString = strprintf(_("Friends: %u/%u"),
                static_cast<uint32_t>(online),
                static_cast<uint32_t>(total));
            updateCounter();
        }

    private:
        BeingsListModel *mBeings;
};

#endif  // GUI_WIDGETS_TABS_SOCIALFRIENDSTAB_H