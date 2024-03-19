//
/// lpineapple.cpp
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

#include "lpineapple.h"

extern "C"
int lua_pineapple_version(lua_State* state) {
    lua_pushinteger(state, PINEAPPLE_VERSION_MAJOR);
    lua_pushinteger(state, PINEAPPLE_VERSION_MINOR);
    return 2;
}

luaL_Reg const pineapple_funcs [] = {
    { "version",          lua_pineapple_version },
    { NULL,               NULL }
};

extern "C"
int lua_open_pineapple(lua_State* state) {
    luaL_newlib(state, pineapple_funcs);
    return 1;
}
