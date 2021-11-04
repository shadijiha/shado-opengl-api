#include "LuaScript.h"
#include <fstream>
#include "Debug.h"
#include "Entity.h"

namespace Shado {
    // Entity Wrapper functions
	int _CreateEntity(lua_State* L) {
        
        EntityDefinition def;
		def.type = EntityType::DYNAMIC;

		// Check if lua has provided a b2World for entity
		if (lua_gettop(L) != 1) return -1;

		// Get the world
		Scene* scene = (Scene*)lua_touserdata(L, 1);
		
		Entity* entity = new Entity(def, scene->getWorld());
		scene->addEntityToWorld(entity);
		lua_pushlightuserdata(L, entity);

		return 1;	
	}

	int _SetEntityTexture(lua_State* L) {
		// Check if lua has provided an Entity Ptr and a path for texture
		if (lua_gettop(L) != 2) return -1;
		
		// Get entity
		Entity* e = (Entity*)lua_touserdata(L, 1);
		std::string path = lua_tostring(L, 2);
		e->setTexture(path);

		return 1;
	}

	int _SetColor(lua_State* L) {
		// Check if lua has provided an Entity Ptr and 4 arguments for colour
		if (lua_gettop(L) != 5) return -1;

		Entity* e = (Entity*)lua_touserdata(L, 1);
		float r = lua_tonumber(L, 2);
		float g = lua_tonumber(L, 3);
		float b = lua_tonumber(L, 4);
		float a = lua_tonumber(L, 5);
		e->setColor({r, g, b, a});

		return 1;
	}

	int _SetPosition(lua_State* L) {
		// Check if lua has provided an Entity Ptr and 2 arguments for position
		if (lua_gettop(L) != 5) return -1;

		Entity* e = (Entity*)lua_touserdata(L, 1);
		float x = lua_tonumber(L, 2);
		float y = lua_tonumber(L, 3);

		e->getNativeBody()->SetTransform({ x, y }, e->getNativeBody()->GetAngle());

		return 1;
	}
	
	// ===================== LUA SCRIPT STUFF ======================

    LuaScript::LuaScript(Scene& scene, const std::string& filename)
	: filename(filename)
	{
        L = luaL_newstate();
        luaL_openlibs(L);

		// Register the world variable
		lua_pushlightuserdata(L, &scene);
		lua_setglobal(L, "_scene");

		// Register all native functions
		lua_register(L, "_CreateEntity", _CreateEntity);
		lua_register(L, "_SetEntityTexture", _SetEntityTexture);
		lua_register(L, "_SetColor", _SetColor);
		lua_register(L, "_SetPosition", _SetPosition);
		
		int code = luaL_dofile(L, filename.c_str());
		checkLua(code);
    }

    LuaScript::~LuaScript() {
        if (L) lua_close(L);
    }

	void LuaScript::onCreate() {
		lua_getglobal(L, "OnCreate");
		if (lua_isfunction(L, -1)) {
			int code = lua_pcall(L, 0, 1, 0);
			checkLua(code);
		}
	}

	void LuaScript::onUpdate(TimeStep dt) {
		lua_getglobal(L, "OnUpdate");
		if (lua_isfunction(L, -1)) {
			lua_pushnumber(L, (float)dt);
			int code = lua_pcall(L, 1, 1, 0);
			checkLua(code);
		}
	}

	void LuaScript::onDestoy() {
	}

	void LuaScript::checkLua(int r) {
		if (r != LUA_OK) {
            auto errormsg = lua_tostring(L, -1);
            SHADO_CORE_ERROR("{0}", errormsg);
		}
	}

}
