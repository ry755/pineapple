//
// kernel.cpp
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include "kernel.h"
#include <circle/startup.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <lua.hpp>
#include "lconsole.h"
#include "lfilesystem.h"
#include "lgfx.h"
#include "lpineapple.h"

extern "C"
int lua_interface();

#define BOOT_FILE "boot.lua"

CKernel *CKernel::s_pThis = 0;

CKernel::CKernel(void)
:   CSquareApp("pineapple") {
        assert(s_pThis == 0);
        s_pThis = this;
        mActLED.Blink(5); // show we are alive
}

CKernel *CKernel::Get(void) {
    assert(s_pThis != 0);
    return s_pThis;
}

CStdlibApp::TShutdownMode CKernel::Run(void) {
    mScreen.Write("\n", 1);
    lua_interface();
    return ShutdownHalt;
}

extern "C"
void print_error(lua_State *state) {
    // the error message is on top of the stack
    // tetch it, print it, and then pop it off the stack
    const char *message = lua_tostring(state, -1);
    puts(message);
    lua_pop(state, 1);
}

extern "C"
int lua_reboot(lua_State* state) {
    reboot();
    return 0;
}

extern "C"
int lua_interface() {
    lua_State *state = luaL_newstate();
    luaL_openlibs(state);
    lua_open_console(state);
    lua_setglobal(state, "console");
    lua_open_filesystem(state);
    lua_setglobal(state, "filesystem");
    lua_open_gfx(state);
    lua_setglobal(state, "gfx");
    lua_open_pineapple(state);
    lua_setglobal(state, "pineapple");
    lua_register(state, "reboot", lua_reboot);

    int result = luaL_loadfile(state, BOOT_FILE);
    if (result != LUA_OK && result != LUA_ERRFILE) {
        print_error(state);
        //return -1;
    }
    if (result == LUA_OK) {
        result = lua_pcall(state, 0, LUA_MULTRET, 0);
        if (result != LUA_OK) {
            print_error(state);
        }
    } else {
        printf("Failed to open %s!\n", BOOT_FILE);
    }

    char line[200];
    while (true) {
        printf("> ");
        fflush(stdout);
        if (fgets(line, sizeof(line), stdin) != nullptr) {
            // trim line feed
            line[strlen(line)-1] = '\0';
            // check if bare fn name given
            lua_getglobal(state, line);
            int objtype = lua_type(state, -1);
            if (objtype == LUA_TFUNCTION) {
                // try with no args
                strncat(line, "()", sizeof(line)-strlen(line)-1);
            }
            result = luaL_loadstring(state, line);
            if (result != LUA_OK) {
                print_error(state);
            } else {
                result = lua_pcall(state, 0, LUA_MULTRET, 0);
                if (result != LUA_OK) {
                    print_error(state);
                }
            }
        }
    }

    lua_close(state);
    return 0;
}
