//
/// lconsole.cpp
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include <lua.hpp>

#include "kernel.h"

#include "lconsole.h"

static const char CursorOff[] = "\x1b[?25l";
static const char CursorOn[] = "\x1b[?25h";
static const char CursorMove[] = "\x1b[%lu;%luH";

static TScreenStatus scrstat;
extern "C"
int lua_getcpos(lua_State* state) {
    scrstat = CKernel::Get()->mScreen.GetStatus();
    return 0;
}

extern "C"
int lua_setcpos(lua_State* state) {
    CKernel::Get()->mScreen.SetStatus(scrstat);
    return 0;
}

extern "C"
int lua_getcursor(lua_State* state) {
    TScreenStatus status = CKernel::Get()->mScreen.GetStatus();
    int rows = CKernel::Get()->mScreen.GetRows();
    int cols = CKernel::Get()->mScreen.GetColumns();

    lua_pushnumber(state, status.nCursorX / cols);
    lua_pushnumber(state, status.nCursorY / rows);
    return 2;
}

extern "C"
int lua_setcursor(lua_State* state) {
    lua_Integer row = luaL_checkinteger(state, 1);
    lua_Integer col = luaL_checkinteger(state, 2);

    char move_string[20];
    int length = sprintf(move_string, CursorMove, row, col);
    CKernel::Get()->mScreen.Write(move_string, length);
    return 0;
}

extern "C"
int lua_cursor(lua_State* state) {
    lua_Integer arg = luaL_checkinteger(state, 1);
    CKernel::Get()->mScreen.Write(arg ? CursorOn : CursorOff, sizeof CursorOn);
    return 0;
}

extern "C"
int lua_consolesize(lua_State* state) {
    CScreenDevice* scrp = &CKernel::Get()->mScreen;
    unsigned rows = scrp->GetRows();
    unsigned cols = scrp->GetColumns();
    lua_pushnumber(state, rows);
    lua_pushnumber(state, cols);
    return 2;
}

extern "C"
int lua_conmode(lua_State* state) {
    // return previous mode
    lua_pushnumber(state, CKernel::Get()->mConsole.GetOptions());
    // The number of function arguments will be on top of the stack.
    int args = lua_gettop(state);
    if (args > 0) {
        if (lua_isinteger(state, 1)) {
            lua_Integer arg = lua_tointeger(state, 1);
            CKernel::Get()->mConsole.SetOptions(arg);
        } else {
            const char* val = lua_tostring(state, 1);
            if (!strcmp(val, "sane")) {
                CKernel::Get()->mConsole.SetOptions(CONSOLE_OPTION_ICANON|CONSOLE_OPTION_ECHO);
            }
        }
    }
    return 1;
}

static char buf[20];
extern "C"
int lua_readkey(lua_State* state) {
    int n = CKernel::Get()->mConsole.Read(buf, 1);
    buf[n] = '\0';
    lua_pushstring(state, buf);
    return 1;
}

extern "C"
int lua_plotxy(lua_State* state) {
    lua_Integer px = luaL_checkinteger(state, 1);
    lua_Integer py = luaL_checkinteger(state, 2);
    lua_Integer col = luaL_checkinteger(state, 3);
    CKernel::Get()->mScreen.SetPixel(px, py, col);
    return 0;
}

luaL_Reg const cons_funcs [] = {
    { "cursor",           lua_cursor },
    { "getcpos",          lua_getcpos },
    { "setcpos",          lua_setcpos },
    { "getcursor",        lua_getcursor },
    { "setcursor",        lua_setcursor },
    { "size",             lua_consolesize },
    { "mode",             lua_conmode },
    { "readkey",          lua_readkey },
    { "plotxy",           lua_plotxy },
    { NULL,               NULL }
};

extern "C"
int lua_open_console(lua_State* state) {
    luaL_newlib(state, cons_funcs);
    return 1;
}
