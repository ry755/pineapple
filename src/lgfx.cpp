//
/// lgfx.cpp
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

#include "lgfx.h"

extern "C"
int lua_text(lua_State* state) {
    lua_Integer px = luaL_checkinteger(state, 1);
    lua_Integer py = luaL_checkinteger(state, 2);
    const char *str = lua_tostring(state, 3);
    lua_Integer col = luaL_checkinteger(state, 4);
    CKernel::Get()->mGfx.DrawText(px, py, col, str);
    return 0;
}

extern "C"
int lua_rect(lua_State* state) {
    lua_Integer px = luaL_checkinteger(state, 1);
    lua_Integer py = luaL_checkinteger(state, 2);
    lua_Integer w = luaL_checkinteger(state, 3);
    lua_Integer h = luaL_checkinteger(state, 4);
    lua_Integer col = luaL_checkinteger(state, 5);
    CKernel::Get()->mGfx.DrawRect(px, py, w, h, col);
    return 0;
}

extern "C"
int lua_circle(lua_State* state) {
    lua_Integer px = luaL_checkinteger(state, 1);
    lua_Integer py = luaL_checkinteger(state, 2);
    lua_Integer r = luaL_checkinteger(state, 3);
    lua_Integer col = luaL_checkinteger(state, 4);
    CKernel::Get()->mGfx.DrawCircle(px, py, r, col);
    return 0;
}

extern "C"
int lua_clear(lua_State* state) {
    lua_Integer col = luaL_checkinteger(state, 1);
    CKernel::Get()->mGfx.ClearScreen(col);
    return 0;
}

luaL_Reg const gfx_funcs [] = {
    { "text",             lua_text },
    { "rect",             lua_rect },
    { "circle",           lua_circle },
    { "clear",            lua_clear },
    { NULL,               NULL }
};

extern "C"
int lua_open_gfx(lua_State* state) {
    luaL_newlib(state, gfx_funcs);
    return 1;
}
