#pragma once
#include <string>
#include <vector>
#include <iostream>
#include "Layer.h"

// Lua is written in C, so compiler needs to know how to link its libraries
extern "C" {
	# include "lua.h"
	# include "lauxlib.h"
	# include "lualib.h"
}

namespace Shado {
    class LuaScript {
    public:
        LuaScript(Scene& scene, const std::string& filename);
        ~LuaScript();

        void onCreate();
        void onUpdate(TimeStep dt);
        void onDestoy();
    	
    private:
        void checkLua(int r);
    	
        lua_State* L;
        std::string filename;
    };    
}


