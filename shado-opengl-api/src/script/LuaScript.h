#pragma once
#include "Luna.h"
#include <string>

namespace Shado {
	class LuaScript {
	public:
		LuaScript(const std::string& code);
		~LuaScript();

		void run();

		static LuaScript fromFile(const std::string& filepath);

	private:
		lua_State* L;
		std::string code;
	};
}
