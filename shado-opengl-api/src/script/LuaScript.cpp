#include "LuaScript.h"
#include <fstream>
#include "Debug.h"

namespace Shado {

	class TestWrapper {
	public:
		static const char className[];
		static Luna<TestWrapper>::RegType methods[];

		int moveBy(lua_State* L) { x += luaL_checknumber(L, 1); return 0; }
		int moveTo(lua_State* L) { x = luaL_checknumber(L, 1); return 0; }
		int position(lua_State* L) { lua_pushnumber(L, x); return 1; }
	private:
		lua_Number x;
	};

	const char TestWrapper::className[] = "Test";
	Luna<TestWrapper>::RegType TestWrapper::methods[] = {
		 method(TestWrapper, moveBy),
		 method(TestWrapper, moveTo),
		 method(TestWrapper, position),
		{0,0}
	};

	LuaScript::LuaScript(const std::string& code)
		: code(code) {
		L = luaL_newstate();
		luaL_openlibs(L);

		Luna<TestWrapper>::Register(L);
	}

	LuaScript::~LuaScript() {
		//lua_setgcthreshold(L, 0);  // collected garbage
		lua_close(L);
	}

	void LuaScript::run() {
		int code = luaL_dostring(L, this->code.c_str());

		if (code != LUA_OK) {
			SHADO_CORE_ERROR("LuaScript Error: {0}", lua_tostring(L, -1));
		}
	}

	LuaScript LuaScript::fromFile(const std::string& filepath) {
		std::ifstream ifs(filepath);
		std::string content((std::istreambuf_iterator<char>(ifs)),
			(std::istreambuf_iterator<char>()));

		return LuaScript(content);
	}
}
