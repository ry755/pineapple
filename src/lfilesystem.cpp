//
/// lfilesystem.cpp
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fatfs/ff.h>
static const char* const VolumeStr[FF_VOLUMES] = {FF_VOLUME_STRS};
#include <lua.hpp>

#include "kernel.h"

#include "lfilesystem.h"

char buf[500];

extern "C"
void lua_pushinfo(lua_State* state, FILINFO* info) {
    lua_newtable(state);
    lua_pushstring(state, "name");
    lua_pushstring(state, info->fname);
    lua_rawset(state, -3);
    lua_pushstring(state, "size");
    lua_pushnumber(state, info->fsize);
    lua_rawset(state, -3);
    lua_pushstring(state, "modified");
    CTime tTime;
    unsigned nDay = info->fdate & 0x1F;
    unsigned nMonth = (info->fdate >> 5) & 0x0F;
    unsigned nYear = 1980 + ((info->fdate >> (5+4)) & 0x7F);
    tTime.SetDate(nDay, nMonth, nYear);
    unsigned nHour = (info->ftime >> (5+6)) & 0x1F;
    unsigned nMinute = (info->ftime >> 5) & 0x3F;
    unsigned nSecond = (info->ftime & 0x1F) * 2;
    tTime.SetTime(nHour, nMinute, nSecond);
    lua_pushnumber(state, (unsigned) tTime.Get());
    lua_rawset(state, -3);
    lua_pushstring(state, "attributes");
    lua_pushnumber(state, info->fattrib);
    lua_rawset(state, -3);
}

extern "C"
int lua_getstat(lua_State* state) {
    if (!lua_isstring(state, 1)) {
        lua_pushnil(state);
        lua_pushstring(state, "stat: No filename");
        return 2;
    }
    const char* name = lua_tostring(state, 1);
    FILINFO info;
    unsigned Result = f_stat(name, &info);
    if (Result == FR_OK) {
        lua_pushinfo(state, &info);
        return 1;
    } else {
        lua_pushnil(state);
        sprintf(buf, "stat: error: %d", Result);
        lua_pushstring(state, buf);
        return 2;
    }
}

extern "C"
int lua_getdir(lua_State* state) {
    int err = 0;
    // The number of function arguments will be on top of the stack.
    int args = lua_gettop(state);
    const char* path = "/";
    if (args > 0) {
        path = lua_tostring(state, 1);
    }
    DIR dir;
    FILINFO info;
    FRESULT Result = f_opendir(&dir, path);
    if (Result == FR_OK) {
        lua_newtable(state);
        while (true) {
            Result = f_readdir(&dir, &info);
            if ((Result == FR_OK) && info.fname[0]) {
                // f_readdir returns empty name on end of dir
                lua_pushstring(state, info.fname);
                lua_pushinfo(state, &info);
                lua_rawset(state, -3);
            } else {
            // Error or end of dir
            if(info.fname[0]) {
                err++;
                sprintf(buf, "readdir failed with code %d", Result);
            }
            break;
            }
        }
        Result = f_closedir(&dir);
        if (Result != FR_OK) {
            err++;
            sprintf(buf, "closedir failed with code %d", Result);
        }
    } else {
        err++;
        sprintf(buf,"mopendir failed with code %d", Result);
    }
    if (err > 0) {
        lua_pushnil(state);
        lua_pushstring(state, buf);
        return 2;
    } else {
        return 1;
    }
}

extern "C"
int lua_checkdir(lua_State* state) {
    int err = 0;
    // The number of function arguments will be on top of the stack.
    int args = lua_gettop(state);
    const char* path = "/";
    if (args > 0) {
        path = lua_tostring(state, 1);
    }
    DIR dir;
    FRESULT Result = f_opendir(&dir, path);
    if (Result == FR_OK) {
        lua_pushstring(state, path);
        Result = f_closedir(&dir);
        if (Result != FR_OK) {
            err++;
            sprintf(buf, "closedir failed with code %d", Result);
        }
    } else {
        err++;
        sprintf(buf,"opendir failed with code %d", Result);
    }
    if (err > 0) {
        lua_pushnil(state);
        lua_pushstring(state, buf);
        return 2;
    } else {
        return 1;
    }
}

extern "C"
int lua_getcwd(lua_State* state) {
    FRESULT Result = f_getcwd(buf, sizeof(buf));
    if (Result == FR_OK) {
        lua_pushstring(state, buf);
        return 1;
    } else {
        lua_pushnil(state);
        lua_pushfstring(state, "getcwd: error: %d", Result);
        return 2;
    }
}

extern "C"
int lua_getfree(lua_State* state) {
    if (!lua_isstring(state, 1)) {
        lua_pushnil(state);
        lua_pushstring(state, "getfree: No filename.");
        return 2;
    }
    const char* name = lua_tostring(state, 1);
    DWORD free;
    FATFS *fs;
    DIR dir;
    FRESULT Result = f_opendir(&dir, name);
    if (Result == FR_OK) {
        Result = f_getfree(name, &free, &fs);
        if (Result == FR_OK) {
            lua_pushnumber(state, free);
            return 1;
        }
    }
    lua_pushnil(state);
    lua_pushfstring(state, "getfree: error: %d", Result);
    return 2;
}

extern "C"
int lua_rename(lua_State* state) {
    if (!(lua_isstring(state, 1) && lua_isstring(state, 2))) {
        lua_pushnil(state);
        lua_pushstring(state, "rename: No filename(s).");
        return 2;
    }
    const char* nold = lua_tostring(state, 1);
    const char* nnew = lua_tostring(state, 2);
    FRESULT Result = f_rename(nold, nnew);
    if (Result == FR_OK) {
        lua_pushstring(state, nnew);
        return 1;
    } else {
        lua_pushnil(state);
        lua_pushfstring(state, "rename: Error, result code: %d", Result);
        return 2;
    }
}

extern "C"
int lua_fcopy(lua_State* state) {
    if (!(lua_isstring(state, 1) && lua_isstring(state, 2))) {
        lua_pushnil(state);
        lua_pushstring(state, "copy: No filename(s).");
        return 2;
    }
    const char* nfrom = lua_tostring(state, 1);
    const char* nto = lua_tostring(state, 2);
    int Result = 0;
    char buf[BUFSIZ]; // from stdio.h
    size_t size;

    FILE* source = fopen(nfrom, "rb");
    if (source) {
        FILE* dest = fopen(nto, "wb");
        if (dest) {
            while (size = fread(buf, 1, BUFSIZ, source)) {
                fwrite(buf, 1, size, dest);
            }

            fclose(source);
            fclose(dest);
        }
    }
    if (!Result) {
        lua_pushstring(state, nto);
        return 1;
    } else {
        lua_pushnil(state);
        lua_pushfstring(state, "copy: Error, result code: %d", Result);
        return 2;
    }
}

extern "C"
int lua_chmod(lua_State* state) {
    if (!(lua_isstring(state, 1) && lua_isinteger(state, 2) && lua_isinteger(state, 3))) {
        lua_pushnil(state);
        lua_pushstring(state, "attr: Bad arguments.");
        return 2;
    }
    const char* path = lua_tostring(state, 1);
    unsigned attr = lua_tonumber(state, 2);
    unsigned mask = lua_tonumber(state, 3);
    FRESULT Result = f_chmod(path, attr, mask);
    if (Result == FR_OK) {
        lua_pushstring(state, path);
        return 1;
    } else {
        lua_pushnil(state);
        lua_pushfstring(state, "attr: Error, result code: %d", Result);
        return 2;
    }
}

extern "C"
int lua_umount(lua_State* state) {
    if (!lua_isstring(state, 1)) {
        lua_pushnil(state);
        lua_pushfstring(state, "unmount: No path.");
        return 2;
    }
    const char* name = lua_tostring(state, 1);
    FRESULT Result = f_mount(NULL, name, 1);
    if (Result == FR_OK) {
        lua_pushstring(state, name);
        return 1;
    } else {
        lua_pushnil(state);
        lua_pushfstring(state, "unmount: error: %d", Result);
        return 2;
    }
}

extern "C"
int lua_vols(lua_State* state) {
    lua_newtable(state);
    for (int i = 0; i < FF_VOLUMES; ) {
        lua_pushstring(state, VolumeStr[i]);
        lua_rawseti(state, -2, ++i);
    }
    return 1;
}

extern "C"
int fsop_one(lua_State* state, FRESULT (*func)(const char *), const char *fname) {
    if (!lua_isstring(state, 1)) {
        lua_pushnil(state);
        lua_pushfstring(state, "%s: No filename.", fname);
        return 2;
    }
    const char* name = lua_tostring(state, 1);
    FRESULT Result = (*func)(name);
    if (Result == FR_OK) {
        lua_pushstring(state, name);
        return 1;
    } else {
        lua_pushnil(state);
        lua_pushfstring(state, "%s: error: %d", fname, Result);
        return 2;
    }
}

extern "C"
int lua_unlink(lua_State* state) {
    return fsop_one(state, &f_unlink, "unlink");
}

extern "C"
int lua_mkdir(lua_State* state) {
    return fsop_one(state, &f_mkdir, "mkdir");
}

extern "C"
int lua_chdir(lua_State* state) {
    return fsop_one(state, &f_chdir, "chdir");
}

extern "C"
int lua_chdrive(lua_State* state) {
    return fsop_one(state, &f_chdrive, "chdrive");
}

luaL_Reg const fs_funcs [] = {
    { "checkdir",         lua_checkdir },
    { "getdir",           lua_getdir },
    { "getstat",          lua_getstat },
    { "unlink",           lua_unlink },
    { "mkdir",            lua_mkdir },
    { "chdir",            lua_chdir },
    { "drive",            lua_chdrive },
    { "getpath",          lua_getcwd },
    { "getfree",          lua_getfree },
    { "copy",          	  lua_fcopy },
    { "rename",           lua_rename },
    { "attr",             lua_chmod },
    { "unmount",          lua_umount },
    { "getvols",          lua_vols },
    { NULL,               NULL }
};

extern "C"
int lua_open_filesystem(lua_State* state) {
    luaL_newlib(state, fs_funcs);
    return 1;
}
