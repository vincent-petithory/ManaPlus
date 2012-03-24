/*
 *  Custom keyboard shortcuts configuration
 *  Copyright (C) 2007  Joshua Langley <joshlangley@optusnet.com.au>
 *  Copyright (C) 2009-2010  The Mana Developers
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

#include "keyboardconfig.h"

#include "configuration.h"
#include "inputevent.h"
#include "inputmanager.h"
#include "logger.h"

#include "gui/sdlinput.h"
#include "gui/setup_keyboard.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <SDL_events.h>

#include <algorithm>

#include "debug.h"

class KeyFunctor
{
    public:
        bool operator() (int key1, int key2)
        {
            return keys[key1].priority >= keys[key2].priority;
        }

        KeyFunction *keys;

} keySorter;

struct KeyData
{
    const char *configField;
    int defaultValue;
    std::string caption;
    int grp;
    ActionFuncPtr action;
    int modKeyIndex;
    int priority;
    int condition;
};

// keyData must be in same order as enum keyAction.
static KeyData const keyData[KeyboardConfig::KEY_TOTAL] = {
    {"", 0,
        N_("Basic Keys"),
        0,
        nullptr,
        KeyboardConfig::KEY_NO_VALUE, 0,
        0},
    {"keyMoveUp", SDLK_UP,
        N_("Move Up"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::moveUp,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME2},
    {"keyMoveDown", SDLK_DOWN,
        N_("Move Down"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::moveDown,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME2},
    {"keyMoveLeft", SDLK_LEFT,
        N_("Move Left"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::moveLeft,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME},
    {"keyMoveRight", SDLK_RIGHT,
        N_("Move Right"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::moveRight,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME},
    {"keyAttack", SDLK_LCTRL,
        N_("Attack"),
        KeyboardConfig::GRP_DEFAULT,
        nullptr,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_DEFAULT},
    {"keyTargetAttack", SDLK_x,
        N_("Target & Attack"),
        KeyboardConfig::GRP_DEFAULT,
        nullptr,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_DEFAULT},
    {"keyMoveToTarget", SDLK_v,
        N_("Move to Target"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::moveToTarget,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_VALIDSPEED},
    {"keyChangeMoveToTarget", SDLK_PERIOD,
        N_("Change Move to Target type"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::changeMoveToTarget,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_VALIDSPEED | COND_EMODS},
    {"keyMoveToHome", SDLK_d,
        N_("Move to Home location"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::moveToHome,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_VALIDSPEED},
    {"keySetHome", SDLK_KP5,
        N_("Set home location"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::setHome,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_VALIDSPEED},
    {"keyMoveToPoint", SDLK_RSHIFT, 
        N_("Move to navigation point"),
        KeyboardConfig::GRP_DEFAULT,
        nullptr,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_DEFAULT},
    {"keyTalk", SDLK_t,
        N_("Talk"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::talk,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME},
    {"keyTarget", SDLK_LSHIFT,
        N_("Stop Attack"),
        KeyboardConfig::GRP_DEFAULT,
        nullptr,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME},
    {"keyUnTarget", KeyboardConfig::KEY_NO_VALUE,
        N_("Untarget"),
        KeyboardConfig::GRP_DEFAULT,
        nullptr,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_DEFAULT},
    {"keyTargetClosest", SDLK_a,
        N_("Target Closest"),
        KeyboardConfig::GRP_DEFAULT,
        nullptr,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_DEFAULT},
    {"keyTargetNPC", SDLK_n,
        N_("Target NPC"),
        KeyboardConfig::GRP_DEFAULT,
        nullptr,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_DEFAULT},
    {"keyTargetPlayer", SDLK_q,
        N_("Target Player"),
        KeyboardConfig::GRP_DEFAULT,
        nullptr,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_DEFAULT},
    {"keyPickup", SDLK_z,
        N_("Pickup"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::pickup,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_NOTARGET},
    {"keyChangePickupType", SDLK_o,
        N_("Change Pickup Type"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::changePickupType,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_VALIDSPEED | COND_EMODS},
    {"keyHideWindows", SDLK_h,
        N_("Hide Windows"),
        KeyboardConfig::GRP_DEFAULT | KeyboardConfig::GRP_GUI,
        &ActionManager::hideWindows,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_NOTARGET},
    {"keyBeingSit", SDLK_s,
        N_("Sit"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::sit,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_NOTARGET},
    {"keyScreenshot", SDLK_p,
        N_("Screenshot"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::screenshot,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_NOTARGET},
    {"keyTrade", SDLK_r,
        N_("Enable/Disable Trading"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::changeTrade,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_NOTARGET},
    {"keyPathfind", SDLK_f,
        N_("Change Map View Mode"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::changeMapMode,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_NOTARGET | COND_EMODS},
    {"keyOK", SDLK_SPACE,
        N_("Select OK"),
        KeyboardConfig::GRP_DEFAULT | KeyboardConfig::GRP_GUI,
        &ActionManager::ok,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_NOMODAL | COND_NOAWAY | COND_NONPCINPUT},
    {"keyQuit", SDLK_ESCAPE,
        N_("Quit"),
        KeyboardConfig::GRP_DEFAULT,
//        nullptr,
        &ActionManager::quit,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_DEFAULT},
    {"", 0,
        N_("Shortcuts Keys"),
        0,
        nullptr,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_DEFAULT},
    {"keyShortcutsKey", SDLK_MENU,
        N_("Item Shortcuts Key"),
        KeyboardConfig::GRP_DEFAULT,
        nullptr,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_DEFAULT},
    {"keyShortcut1", SDLK_1,
        strprintf(N_("Item Shortcut %d"), 1),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::shortcut,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_NOTARGET | COND_NOINPUT | COND_ENABLED},
    {"keyShortcut2", SDLK_2,
        strprintf(N_("Item Shortcut %d"), 2),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::shortcut,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_NOTARGET | COND_NOINPUT | COND_ENABLED},
    {"keyShortcut3", SDLK_3,
        strprintf(N_("Item Shortcut %d"), 3),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::shortcut,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_NOTARGET | COND_NOINPUT | COND_ENABLED},
    {"keyShortcut4", SDLK_4,
        strprintf(N_("Item Shortcut %d"), 4),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::shortcut,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_NOTARGET | COND_NOINPUT | COND_ENABLED},
    {"keyShortcut5", SDLK_5,
        strprintf(N_("Item Shortcut %d"), 5),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::shortcut,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_NOTARGET | COND_NOINPUT | COND_ENABLED},
    {"keyShortcut6", SDLK_6,
        strprintf(N_("Item Shortcut %d"), 6),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::shortcut,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_NOTARGET | COND_NOINPUT | COND_ENABLED},
    {"keyShortcut7", SDLK_7,
        strprintf(N_("Item Shortcut %d"), 7),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::shortcut,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_NOTARGET | COND_NOINPUT | COND_ENABLED},
    {"keyShortcut8", SDLK_8,
        strprintf(N_("Item Shortcut %d"), 8),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::shortcut,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_NOTARGET | COND_NOINPUT | COND_ENABLED},
    {"keyShortcut9", SDLK_9,
        strprintf(N_("Item Shortcut %d"), 9),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::shortcut,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_NOTARGET | COND_NOINPUT | COND_ENABLED},
    {"keyShortcut10", SDLK_0,
        strprintf(N_("Item Shortcut %d"), 10),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::shortcut,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_NOTARGET | COND_NOINPUT | COND_ENABLED},
    {"keyShortcut11", SDLK_MINUS,
        strprintf(N_("Item Shortcut %d"), 11),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::shortcut,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_NOTARGET | COND_NOINPUT | COND_ENABLED},
    {"keyShortcut12", SDLK_EQUALS,
        strprintf(N_("Item Shortcut %d"), 12),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::shortcut,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_NOTARGET | COND_NOINPUT | COND_ENABLED},
    {"keyShortcut13", SDLK_BACKSPACE,
        strprintf(N_("Item Shortcut %d"), 13),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::shortcut,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_NOTARGET | COND_NOINPUT | COND_ENABLED},
    {"keyShortcut14", SDLK_INSERT,
        strprintf(N_("Item Shortcut %d"), 14),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::shortcut,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_NOTARGET | COND_NOINPUT | COND_ENABLED},
    {"keyShortcut15", SDLK_HOME,
        strprintf(N_("Item Shortcut %d"), 15),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::shortcut,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_NOTARGET | COND_NOINPUT | COND_ENABLED},
    {"keyShortcut16", KeyboardConfig::KEY_NO_VALUE,
        strprintf(N_("Item Shortcut %d"), 16),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::shortcut,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_NOTARGET | COND_NOINPUT | COND_ENABLED},
    {"keyShortcut17", KeyboardConfig::KEY_NO_VALUE,
        strprintf(N_("Item Shortcut %d"), 17),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::shortcut,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_NOTARGET | COND_NOINPUT | COND_ENABLED},
    {"keyShortcut18", KeyboardConfig::KEY_NO_VALUE,
        strprintf(N_("Item Shortcut %d"), 18),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::shortcut,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_NOTARGET | COND_NOINPUT | COND_ENABLED},
    {"keyShortcut19", KeyboardConfig::KEY_NO_VALUE,
        strprintf(N_("Item Shortcut %d"), 19),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::shortcut,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_NOTARGET | COND_NOINPUT | COND_ENABLED},
    {"keyShortcut20", KeyboardConfig::KEY_NO_VALUE,
        strprintf(N_("Item Shortcut %d"), 20),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::shortcut,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_NOTARGET | COND_NOINPUT | COND_ENABLED},
    {"", 0,
        N_("Windows Keys"),
        0,
        nullptr,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_DEFAULT},
    {"keyWindowHelp", SDLK_F1,
        N_("Help Window"),
        KeyboardConfig::GRP_DEFAULT | KeyboardConfig::GRP_GUI,
        &ActionManager::helpWindowShow,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_NOTARGET},
    {"keyWindowStatus", SDLK_F2,
        N_("Status Window"),
        KeyboardConfig::GRP_DEFAULT | KeyboardConfig::GRP_GUI,
        &ActionManager::statusWindowShow,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_NOTARGET},
    {"keyWindowInventory", SDLK_F3,
        N_("Inventory Window"),
        KeyboardConfig::GRP_DEFAULT | KeyboardConfig::GRP_GUI,
        &ActionManager::inventoryWindowShow,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_NOTARGET},
    {"keyWindowEquipment", SDLK_F4,
        N_("Equipment Window"),
        KeyboardConfig::GRP_DEFAULT | KeyboardConfig::GRP_GUI,
        &ActionManager::equipmentWindowShow,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_NOTARGET},
    {"keyWindowSkill", SDLK_F5,
        N_("Skill Window"),
        KeyboardConfig::GRP_DEFAULT | KeyboardConfig::GRP_GUI,
        &ActionManager::skillDialogShow,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_NOTARGET},
    {"keyWindowMinimap", SDLK_F6,
        N_("Minimap Window"),
        KeyboardConfig::GRP_DEFAULT | KeyboardConfig::GRP_GUI,
        &ActionManager::minimapWindowShow,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_NOTARGET},
    {"keyWindowChat", SDLK_F7,
        N_("Chat Window"),
        KeyboardConfig::GRP_DEFAULT | KeyboardConfig::GRP_GUI,
        &ActionManager::chatWindowShow,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_NOTARGET},
    {"keyWindowShortcut", SDLK_F8,
        N_("Item Shortcut Window"),
        KeyboardConfig::GRP_DEFAULT | KeyboardConfig::GRP_GUI,
        &ActionManager::shortcutWindowShow,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_NOTARGET},
    {"keyWindowSetup", SDLK_F9,
        N_("Setup Window"),
        KeyboardConfig::GRP_DEFAULT | KeyboardConfig::GRP_GUI,
        &ActionManager::setupWindowShow,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_NOTARGET | COND_NOINPUT | COND_ENABLED},
    {"keyWindowDebug", SDLK_F10,
        N_("Debug Window"),
        KeyboardConfig::GRP_DEFAULT | KeyboardConfig::GRP_GUI,
        &ActionManager::debugWindowShow,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_NOTARGET},
    {"keyWindowSocial", SDLK_F11,
        N_("Social Window"),
        KeyboardConfig::GRP_DEFAULT | KeyboardConfig::GRP_GUI,
        &ActionManager::socialWindowShow,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_NOTARGET},
    {"keyWindowEmoteBar", SDLK_F12,
        N_("Emote Shortcut Window"),
        KeyboardConfig::GRP_DEFAULT | KeyboardConfig::GRP_GUI,
        &ActionManager::emoteShortcutWindowShow,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_NOTARGET},
    {"keyWindowOutfit", SDLK_BACKQUOTE,
        N_("Outfits Window"),
        KeyboardConfig::GRP_DEFAULT | KeyboardConfig::GRP_GUI,
        &ActionManager::outfitWindowShow,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_NOTARGET},
    {"keyWindowShop", -1,
        N_("Shop Window"),
        KeyboardConfig::GRP_DEFAULT | KeyboardConfig::GRP_GUI,
        &ActionManager::shopWindowShow,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_NOTARGET},
    {"keyWindowDrop", SDLK_w,
        N_("Quick drop Window"),
        KeyboardConfig::GRP_DEFAULT | KeyboardConfig::GRP_GUI,
        &ActionManager::dropShortcutWindowShow,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_NOTARGET},
    {"keyWindowKills", SDLK_e,
        N_("Kill Stats Window"),
        KeyboardConfig::GRP_DEFAULT | KeyboardConfig::GRP_GUI,
        &ActionManager::killStatsWindowShow,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_NOTARGET},
    {"keyWindowSpells", SDLK_j,
        N_("Commands Window"),
        KeyboardConfig::GRP_DEFAULT | KeyboardConfig::GRP_GUI,
        &ActionManager::spellShortcutWindowShow,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_NOTARGET},
    {"keyWindowBotChecker", SDLK_LEFTBRACKET,
        N_("Bot Checker Window"),
        KeyboardConfig::GRP_DEFAULT | KeyboardConfig::GRP_GUI,
        &ActionManager::botcheckerWindowShow,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_NOTARGET},
    {"keyWindowOnline", KeyboardConfig::KEY_NO_VALUE,
        N_("Who Is Online Window"),
        KeyboardConfig::GRP_DEFAULT | KeyboardConfig::GRP_GUI,
        &ActionManager::whoIsOnlineWindowShow,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_NOTARGET},
    {"keyWindowDidYouKnow", KeyboardConfig::KEY_NO_VALUE,
        N_("Did you know Window"),
        KeyboardConfig::GRP_DEFAULT | KeyboardConfig::GRP_GUI,
        &ActionManager::didYouKnowWindowShow,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_NOTARGET},
    {"keySocialPrevTab", KeyboardConfig::KEY_NO_VALUE,
        N_("Previous Social Tab"),
        KeyboardConfig::GRP_DEFAULT | KeyboardConfig::GRP_GUI,
        &ActionManager::prevSocialTab,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_NOINPUT},
    {"keySocialNextTab", KeyboardConfig::KEY_NO_VALUE,
        N_("Next Social Tab"),
        KeyboardConfig::GRP_DEFAULT | KeyboardConfig::GRP_GUI,
        &ActionManager::nextSocialTab,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_NOINPUT},
    {"", 0,
        N_("Emotes Keys"),
        0,
        nullptr,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_DEFAULT},
    {"keySmilie", SDLK_LALT,
        N_("Smilie"),
        KeyboardConfig::GRP_DEFAULT,
        nullptr,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_DEFAULT},
    {"keyEmoteShortcut1", SDLK_1,
        strprintf(N_("Emote Shortcut %d"), 1),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut2", SDLK_2,
        strprintf(N_("Emote Shortcut %d"), 2),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut3", SDLK_3,
        strprintf(N_("Emote Shortcut %d"), 3),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut4", SDLK_4,
        strprintf(N_("Emote Shortcut %d"), 4),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut5", SDLK_5,
        strprintf(N_("Emote Shortcut %d"), 5),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut6", SDLK_6,
        strprintf(N_("Emote Shortcut %d"), 6),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut7", SDLK_7,
        strprintf(N_("Emote Shortcut %d"), 7),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut8", SDLK_8,
        strprintf(N_("Emote Shortcut %d"), 8),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut9", SDLK_9,
        strprintf(N_("Emote Shortcut %d"), 9),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut10", SDLK_0,
        strprintf(N_("Emote Shortcut %d"), 10),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut11", SDLK_MINUS,
        strprintf(N_("Emote Shortcut %d"), 11),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut12", SDLK_EQUALS,
        strprintf(N_("Emote Shortcut %d"), 12),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut13", SDLK_BACKSPACE,
        strprintf(N_("Emote Shortcut %d"), 13),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut14", SDLK_INSERT,
        strprintf(N_("Emote Shortcut %d"), 14),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut15", SDLK_HOME,
        strprintf(N_("Emote Shortcut %d"), 15),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut16", SDLK_q,
        strprintf(N_("Emote Shortcut %d"), 16),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut17", SDLK_w,
        strprintf(N_("Emote Shortcut %d"), 17),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut18", SDLK_e,
        strprintf(N_("Emote Shortcut %d"), 18),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut19", SDLK_r,
        strprintf(N_("Emote Shortcut %d"), 19),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut20", SDLK_t,
        strprintf(N_("Emote Shortcut %d"), 20),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut21", SDLK_y,
        strprintf(N_("Emote Shortcut %d"), 21),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut22", SDLK_u,
        strprintf(N_("Emote Shortcut %d"), 22),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut23", SDLK_i,
        strprintf(N_("Emote Shortcut %d"), 23),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut24", SDLK_o,
        strprintf(N_("Emote Shortcut %d"), 24),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut25", SDLK_p,
        strprintf(N_("Emote Shortcut %d"), 25),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut26", SDLK_LEFTBRACKET,
        strprintf(N_("Emote Shortcut %d"), 26),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut27", SDLK_RIGHTBRACKET,
        strprintf(N_("Emote Shortcut %d"), 27),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut28", SDLK_BACKSLASH,
        strprintf(N_("Emote Shortcut %d"), 28),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut29", SDLK_a,
        strprintf(N_("Emote Shortcut %d"), 29),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut30", SDLK_s,
        strprintf(N_("Emote Shortcut %d"), 30),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut31", SDLK_d,
        strprintf(N_("Emote Shortcut %d"), 31),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut32", SDLK_f,
        strprintf(N_("Emote Shortcut %d"), 32),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut33", SDLK_g,
        strprintf(N_("Emote Shortcut %d"), 33),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut34", SDLK_h,
        strprintf(N_("Emote Shortcut %d"), 34),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut35", SDLK_j,
        strprintf(N_("Emote Shortcut %d"), 35),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut36", SDLK_k,
        strprintf(N_("Emote Shortcut %d"), 36),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut37", SDLK_l,
        strprintf(N_("Emote Shortcut %d"), 37),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut38", SDLK_SEMICOLON,
        strprintf(N_("Emote Shortcut %d"), 38),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut39", SDLK_QUOTE,
        strprintf(N_("Emote Shortcut %d"), 39),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut40", SDLK_z,
        strprintf(N_("Emote Shortcut %d"), 40),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut41", SDLK_x,
        strprintf(N_("Emote Shortcut %d"), 41),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut42", SDLK_c,
        strprintf(N_("Emote Shortcut %d"), 42),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut43", SDLK_v,
        strprintf(N_("Emote Shortcut %d"), 43),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut44", SDLK_b,
        strprintf(N_("Emote Shortcut %d"), 44),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut45", SDLK_n,
        strprintf(N_("Emote Shortcut %d"), 45),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut46", SDLK_m,
        strprintf(N_("Emote Shortcut %d"), 46),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut47", SDLK_COMMA,
        strprintf(N_("Emote Shortcut %d"), 47),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"keyEmoteShortcut48", SDLK_PERIOD,
        strprintf(N_("Emote Shortcut %d"), 48),
        KeyboardConfig::GRP_EMOTION,
        &ActionManager::emote,
        KeyboardConfig::KEY_NO_VALUE, 100,
        COND_GAME},
    {"", 0,
        N_("Outfits Keys"),
        0,
        nullptr,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_DEFAULT},
    {"keyWearOutfit", SDLK_RCTRL, N_("Wear Outfit"),
        KeyboardConfig::GRP_DEFAULT, nullptr,
        KeyboardConfig::KEY_NO_VALUE, 50, COND_DEFAULT},
    {"keyCopyOutfit", SDLK_RALT, N_("Copy Outfit"),
        KeyboardConfig::GRP_DEFAULT, nullptr,
        KeyboardConfig::KEY_NO_VALUE, 50, COND_DEFAULT},
    {"keyCopyEquipedOutfit", SDLK_RIGHTBRACKET,
        N_("Copy equipped to Outfit"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::copyEquippedToOutfit,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_VALIDSPEED},
    {"", 0, N_("Chat Keys"), 0, nullptr, KeyboardConfig::KEY_NO_VALUE, 50,
        COND_DEFAULT},
    {"keyChat", SDLK_RETURN,
        N_("Toggle Chat"),
        KeyboardConfig::GRP_DEFAULT | KeyboardConfig::GRP_CHAT,
        &ActionManager::toggleChat,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME},
    {"keyChatScrollUp", SDLK_PAGEUP,
        N_("Scroll Chat Up"),
        KeyboardConfig::GRP_DEFAULT | KeyboardConfig::GRP_GUI,
        &ActionManager::scrollChatUp,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_DEFAULT},
    {"keyChatScrollDown", SDLK_PAGEDOWN,
        N_("Scroll Chat Down"),
        KeyboardConfig::GRP_DEFAULT | KeyboardConfig::GRP_GUI,
        &ActionManager::scrollChatDown,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_DEFAULT},
    {"keyChatPrevTab", SDLK_KP7,
        N_("Previous Chat Tab"),
        KeyboardConfig::GRP_DEFAULT | KeyboardConfig::GRP_GUI,
        &ActionManager::prevChatTab,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_NOINPUT},
    {"keyChatNextTab", SDLK_KP9,
        N_("Next Chat Tab"),
        KeyboardConfig::GRP_DEFAULT | KeyboardConfig::GRP_GUI,
        &ActionManager::nextChatTab,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_NOINPUT},
    {"keyChatCloseTab", KeyboardConfig::KEY_NO_VALUE,
        N_("Close current Chat Tab"),
        KeyboardConfig::GRP_DEFAULT | KeyboardConfig::GRP_GUI,
        &ActionManager::closeChatTab,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_NOINPUT},
    {"keyChatPrevHistory", SDLK_KP7, N_("Previous chat line"),
        KeyboardConfig::GRP_CHAT, nullptr, KeyboardConfig::KEY_NO_VALUE, 50,
        COND_DEFAULT},
    {"keyChatNextHistory", SDLK_KP9, N_("Next chat line"),
        KeyboardConfig::GRP_CHAT, nullptr, KeyboardConfig::KEY_NO_VALUE, 50,
        COND_DEFAULT},
    {"keyAutoCompleteChat", SDLK_TAB, N_("Chat Auto Complete"),
        KeyboardConfig::GRP_CHAT, nullptr, KeyboardConfig::KEY_NO_VALUE, 50,
        COND_DEFAULT},
    {"keyDeActivateChat", SDLK_ESCAPE, N_("Deactivate Chat Input"),
        KeyboardConfig::GRP_CHAT, nullptr, KeyboardConfig::KEY_NO_VALUE, 50,
        COND_DEFAULT},
    {"", 0, N_("Other Keys"), 0, nullptr, KeyboardConfig::KEY_NO_VALUE, 50,
        COND_DEFAULT},
    {"keyIgnoreInput1", SDLK_LSUPER,
        N_("Ignore input 1"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::ignoreInput,
        KeyboardConfig::KEY_NO_VALUE, 500,
        COND_DEFAULT},
    {"keyIgnoreInput2", SDLK_RSUPER,
        N_("Ignore input 2"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::ignoreInput,
        KeyboardConfig::KEY_NO_VALUE, 500,
        COND_DEFAULT},
    {"keyDirectUp", SDLK_l,
        N_("Direct Up"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::directUp,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_DEFAULT},
    {"keyDirectDown", SDLK_SEMICOLON,
        N_("Direct Down"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::directDown,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_DEFAULT},
    {"keyDirectLeft", SDLK_k,
        N_("Direct Left"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::directLeft,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_DEFAULT},
    {"keyDirectRight", SDLK_QUOTE,
        N_("Direct Right"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::directRight,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_DEFAULT},
    {"keyCrazyMoves", SDLK_SLASH,
        N_("Crazy moves"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::crazyMoves,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_VALIDSPEED},
    {"keyChangeCrazyMoveType", SDLK_BACKSLASH,
        N_("Change Crazy Move mode"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::changeCrazyMove,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_VALIDSPEED | COND_EMODS},
    {"keyQuickDrop", SDLK_y,
        N_("Quick Drop N Items from 0 slot"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::dropItem0,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_VALIDSPEED},
    {"keyQuickDropN", SDLK_u,
        N_("Quick Drop N Items"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::dropItem,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_VALIDSPEED},
    {"keySwitchQuickDrop", SDLK_i,
        N_("Switch Quick Drop Counter"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::switchQuickDrop,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_VALIDSPEED},
    {"keyMagicInma1", SDLK_c,
        N_("Quick heal target or self"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::heal,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_VALIDSPEED},
    {"keyMagicItenplz", SDLK_m,
        N_("Use #itenplz spell"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::itenplz,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_VALIDSPEED},
    {"keyMagicAttack", SDLK_b,
        N_("Use magic attack"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::magicAttack,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_VALIDSPEED},
    {"keySwitchMagicAttack", SDLK_COMMA,
        N_("Switch magic attack"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::changeMagicAttack,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_VALIDSPEED | COND_EMODS},
    {"keySwitchPvpAttack", KeyboardConfig::KEY_NO_VALUE,
        N_("Switch pvp attack"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::changePvpMode,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_VALIDSPEED | COND_EMODS},
    {"keyInvertDirection", SDLK_KP0,
        N_("Change move type"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::changeMoveType,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_VALIDSPEED | COND_EMODS},
    {"keyChangeAttackWeaponType", SDLK_g,
        N_("Change Attack Weapon Type"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::changeAttackWeaponType,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_VALIDSPEED | COND_EMODS},
    {"keyChangeAttackType", SDLK_END,
        N_("Change Attack Type"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::changeAttackType,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_VALIDSPEED | COND_EMODS},
    {"keyChangeFollowMode", SDLK_KP1,
        N_("Change Follow mode"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::changeFollowMode,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_VALIDSPEED | COND_EMODS},
    {"keyChangeImitationMode", SDLK_KP4,
        N_("Change Imitation mode"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::changeImitationMode,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_VALIDSPEED | COND_EMODS},
    {"keyDisableGameModifiers", SDLK_KP8,
        N_("Disable / Enable Game modifier keys"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::changeGameModifier,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_VALIDSPEED},
    {"keyChangeAudio", SDLK_KP3,
        N_("On / Off audio"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::changeAudio,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_VALIDSPEED},
    {"keyAway", SDLK_KP2,
        N_("Enable / Disable away mode"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::away,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_VALIDSPEED},
    {"keyRightClick", SDLK_TAB,
        N_("Emulate right click from keyboard"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::mouseClick,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_NOINPUT | COND_NOAWAY | COND_NOMODAL},
    {"keyCameraMode", SDLK_KP_PLUS,
        N_("Toggle camera mode"),
        KeyboardConfig::GRP_DEFAULT,
        &ActionManager::camera,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_GAME | COND_VALIDSPEED | COND_EMODS},
    {"keyMod", SDLK_LSHIFT,
        N_("Modifier key"),
        KeyboardConfig::GRP_GUI,
        nullptr,
        KeyboardConfig::KEY_NO_VALUE, 50,
        COND_DEFAULT}
};

void KeyboardConfig::init()
{
    for (int i = 0; i < KEY_TOTAL; i++)
    {
        mKey[i].configField = keyData[i].configField;
        mKey[i].defaultValue = keyData[i].defaultValue;
        mKey[i].caption = gettext(keyData[i].caption.c_str());
        mKey[i].value = KEY_NO_VALUE;
        mKey[i].grp = keyData[i].grp;
        mKey[i].action = keyData[i].action;
        mKey[i].modKeyIndex = keyData[i].modKeyIndex;
        mKey[i].priority = keyData[i].priority;
        mKey[i].condition = keyData[i].condition;
    }
    for (int i = KEY_EMOTE_1; i <= KEY_EMOTE_48; i ++)
    {
        mKey[i].caption = strprintf(
            _("Emote Shortcut %d"), i - KEY_EMOTE_1 + 1);
    }
    for (int i = KEY_SHORTCUT_1; i <= KEY_SHORTCUT_20; i ++)
    {
        mKey[i].caption = strprintf(
            _("Item Shortcut %d"), i - KEY_SHORTCUT_1 + 1);
    }

    mNewKeyIndex = KEY_NO_VALUE;
    mEnabled = true;

    retrieve();
    updateKeyActionMap();
}

void KeyboardConfig::retrieve()
{
    for (int i = 0; i < KEY_TOTAL; i++)
    {
        if (*mKey[i].configField)
        {
            mKey[i].value = static_cast<int>(config.getValue(
                mKey[i].configField, mKey[i].defaultValue));
            if (mKey[i].value < -255 || mKey[i].value >= SDLK_LAST)
                mKey[i].value = KEY_NO_VALUE;
        }
    }
}

void KeyboardConfig::store()
{
    for (int i = 0; i < KEY_TOTAL; i++)
    {
        if (*mKey[i].configField)
            config.setValue(mKey[i].configField, mKey[i].value);
    }
}

void KeyboardConfig::makeDefault()
{
    for (int i = 0; i < KEY_TOTAL; i++)
        mKey[i].value = mKey[i].defaultValue;
}

bool KeyboardConfig::hasConflicts()
{
    int i, j;
    /**
     * No need to parse the square matrix: only check one triangle
     * that's enough to detect conflicts
     */
    for (i = 0; i < KEY_TOTAL; i++)
    {
        if (mKey[i].value == KEY_NO_VALUE || !*mKey[i].configField)
            continue;

        for (j = i, j++; j < KEY_TOTAL; j++)
        {
            // Allow for item shortcut and emote keys to overlap
            // as well as emote and ignore keys, but no other keys
            if (mKey[j].value != KEY_NO_VALUE &&
                mKey[i].value == mKey[j].value &&
                ((mKey[i].grp & mKey[j].grp) != 0 &&
                *mKey[i].configField)
               )
            {
                mBindError = strprintf(_("Conflict \"%s\" and \"%s\" keys. "
                                       "Resolve them, or gameplay may result"
                                       " in strange behaviour."),
                                       mKey[i].caption.c_str(),
                                       mKey[j].caption.c_str());
                return true;
            }
        }
    }
    mBindError = "";
    return false;
}

void KeyboardConfig::callbackNewKey()
{
    mSetupKey->newKeyCallback(mNewKeyIndex);
}

int KeyboardConfig::getKeyValueFromEvent(const SDL_Event &event) const
{
    if (event.key.keysym.sym)
        return event.key.keysym.sym;
    else if (event.key.keysym.scancode > 1)
        return -event.key.keysym.scancode;
    return 0;
}

int KeyboardConfig::getKeyIndex(const SDL_Event &event, int grp) const
{
    const int keyValue = getKeyValueFromEvent(event);
    for (int i = 0; i < KEY_TOTAL; i++)
    {
        if (keyValue == mKey[i].value &&
            (grp & mKey[i].grp) != 0)
        {
            return i;
        }
    }
    return KEY_NO_VALUE;
}

bool KeyboardConfig::isActionActive(int index) const
{
    if (!mActiveKeys)
        return false;
    const int value = mKey[index].value;
    if (value >= 0)
        return mActiveKeys[value];
    else
        return false;   // scan codes active state now not implimented
}

void KeyboardConfig::refreshActiveKeys()
{
    mActiveKeys = SDL_GetKeyState(nullptr);
}

std::string KeyboardConfig::getKeyStringLong(int index) const
{
    const int keyValue = getKeyValue(index);
    if (keyValue >= 0)
        return SDL_GetKeyName(static_cast<SDLKey>(keyValue));
    else if (keyValue < -1)
        return strprintf(_("key_%d"), -keyValue);
    else
        return _("unknown key");
}

std::string KeyboardConfig::getKeyValueString(int index) const
{
    const int keyValue = getKeyValue(index);
    if (keyValue >= 0)
    {
        std::string key = SDL_GetKeyName(static_cast<SDLKey>(keyValue));
        return getKeyShortString(key);
    }
    else if (keyValue < -1)
    {
        return strprintf("#%d", -keyValue);
    }
    else
    {
        // TRANSLATORS: Unknown key short string. This string must be maximum 5 chars
        return _("u key");
    }
}

std::string KeyboardConfig::getKeyShortString(const std::string &key) const
{
    if (key == "backspace")
    {
        return "bksp";
    }
    else if (key == "unknown key")
    {
        // TRANSLATORS: Unknown key short string. This string must be maximum 5 chars
        return _("u key");
    }
    return key;
}

SDLKey KeyboardConfig::getKeyFromEvent(const SDL_Event &event) const
{
    return event.key.keysym.sym;
}

void KeyboardConfig::setNewKey(const SDL_Event &event)
{
    mKey[mNewKeyIndex].value = getKeyValueFromEvent(event);
    updateKeyActionMap();
}

void KeyboardConfig::unassignKey()
{
    mKey[mNewKeyIndex].value = KEY_NO_VALUE;
    updateKeyActionMap();
}

void KeyboardConfig::updateKeyActionMap()
{
    mKeyToAction.clear();

    for (int i = 0; i < KEY_TOTAL; i++)
    {
        if (mKey[i].value != KEY_NO_VALUE && mKey[i].action)
            mKeyToAction[mKey[i].value].push_back(i);
    }

    keySorter.keys = &mKey[0];
    KeyToActionMapIter it = mKeyToAction.begin();
    KeyToActionMapIter it_end = mKeyToAction.end();
    for (; it != it_end; ++ it)
    {
        KeysVector *keys = &it->second;
        if (keys->size() > 1)
            sort(keys->begin(), keys->end(), keySorter);
    }
}

bool KeyboardConfig::triggerAction(const SDL_Event &event)
{
    const int i = getKeyValueFromEvent(event);
//    logger->log("key triggerAction: %d", i);
    if (i != 0 && i < SDLK_LAST && mKeyToAction.find(i) != mKeyToAction.end())
    {
        const KeysVector &ptrs = mKeyToAction[i];
//        logger->log("ptrs: %d", (int)ptrs.size());
        KeysVectorCIter it = ptrs.begin();
        KeysVectorCIter it_end = ptrs.end();

        int mask = inputManager.getInputConditionMask();

        for (; it != it_end; ++ it)
        {
            const int keyNum = *it;
            if (keyNum < 0 || keyNum >= KEY_TOTAL)
                continue;

            if (inputManager.checkKey(&mKey[keyNum], mask))
            {
                InputEvent evt(keyNum, mask);
                if ((*(mKey[keyNum].action))(evt))
                {
                    return true;
                }
            }
        }
    }
    return false;
}
