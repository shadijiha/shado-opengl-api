#include "LuaScript.h"
#include <fstream>

#include "Application.h"
#include "Debug.h"
#include "Entity.h"
#include "box2d/b2_fixture.h"
#include "Events/input.h"
#include "Events/MouseEvent.h"
#include "Events/KeyEvent.h"
#include "Events/KeyCodes.h"

namespace Shado {

	static std::string toLower(std::string s) {
		std::transform(s.begin(), s.end(), s.begin(),
			[](unsigned char c) { return std::tolower(c); });
		return s;
	}
	static std::unordered_map<std::string, int> eventToData(Event&);
	
    // Entity Wrapper functions
	int _CreateEntity(lua_State* L) {
        
		// Check if lua has provided a b2World for entity and a width and height
		if (lua_gettop(L) != 3) return -1;

		// Get the world
		Scene* scene = (Scene*)lua_touserdata(L, 1);
		float width = lua_tonumber(L, 2);
		float height = lua_tonumber(L, 3);

		EntityDefinition def;
		def.type = EntityType::DYNAMIC;
		def.scale = { width, height };
		def.name = "Lua entity";
		
		Entity* e = scene->addEntityToWorld(def);
		lua_pushlightuserdata(L, e);

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

	int _SetTillingFactor(lua_State* L) {
		// Check if lua has provided an Entity Ptr and a factor
		if (lua_gettop(L) != 2) return -1;

		// Get entity
		Entity* e = (Entity*)lua_touserdata(L, 1);
		int factor = (int)lua_tonumber(L, 2);
		e->setTillingFactor(factor);
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
		if (lua_gettop(L) != 3) return -1;

		Entity* e = (Entity*)lua_touserdata(L, 1);
		float x = lua_tonumber(L, 2);
		float y = lua_tonumber(L, 3);

		e->getNativeBody()->SetTransform({ x, y }, e->getNativeBody()->GetAngle());

		return 1;
	}
	
	int _SetType(lua_State* L) {
		// Check if lua has provided an Entity Ptr and 1 arguments for type
		if (lua_gettop(L) != 2) return -1;

		Entity* e = (Entity*)lua_touserdata(L, 1);
		std::string type = lua_tostring(L, 2);

		using namespace std::string_literals;
		if (toLower(type) == "dynamic"s)
			e->getNativeBody()->SetType(b2_dynamicBody);
		else if (toLower(type) == "static"s)
			e->getNativeBody()->SetType(b2_staticBody);
		else if (toLower(type) == "kinematic"s)
			e->getNativeBody()->SetType(b2_kinematicBody);

		return 1;	
	}

	int _IsKeyDown(lua_State* L) {
		// Check if lua has provided a key code
		if (lua_gettop(L) != 1) return -1;

		int keycode = lua_tointeger(L, 1);
		lua_pushboolean(L, Input::isKeyPressed(keycode));
		
		return 1;
	}

	int _GetMouseX(lua_State* L) {
		lua_pushnumber(L, Input::getMouseX());
		return 1;
	}

	int _GetMouseY(lua_State* L) {
		lua_pushnumber(L, Input::getMouseY());
		return 1;
	}

	int _GetWindowWidth(lua_State* L) {
		lua_pushinteger(L, Application::get().getWindow().getWidth());
		return 1;
	}

	int _GetWindowHeight(lua_State* L) {
		lua_pushinteger(L, Application::get().getWindow().getHeight());
		return 1;
	}

	int _SetWindowTitle(lua_State* L) {
		// Check if lua has provided a new title
		if (lua_gettop(L) != 1) return -1;
		
		std::string title = lua_tostring(L, 1);
		Application::get().getWindow().setTitle(title);
		return 1;
	}

	int _SetWindowMode(lua_State* L) {
		// Check if lua has provided a new title
		if (lua_gettop(L) != 1) return -1;

		WindowMode mode = (WindowMode)lua_tointeger(L, 1);
		Application::get().getWindow().setMode(mode);
		return 1;
	}

	int _SetWindowOpacity(lua_State* L) {
		// Check if lua has provided a new title
		if (lua_gettop(L) != 1) return -1;

		float mode = lua_tonumber(L, 1);
		Application::get().getWindow().setOpacity(mode);
		return 1;
	}

	int _SetResizable(lua_State* L) {
		// Check if lua has provided a new title
		if (lua_gettop(L) != 1) return -1;

		bool mode = lua_toboolean(L, 1);
		Application::get().getWindow().setResizable(mode);
		return 1;
	}

	int _SetClearColor(lua_State* L) {
		// Check if lua has provided a r, g, b and a
		if (lua_gettop(L) != 4) return -1;

		float r = lua_tonumber(L, 1);
		float g = lua_tonumber(L, 2);
		float b = lua_tonumber(L, 3);
		float a = lua_tonumber(L, 4);

		Renderer2D::SetClearColor({ r, g, b, a });
		
	}

	int _GetSceneByName(lua_State* L) {
		// Check if lua has provided a scene name
		if (lua_gettop(L) != 1) return -1;

		std::string name = lua_tostring(L, 1);
		auto& app = Application::get();

		Scene* found = nullptr;
		for (Scene* element : app.getScenes()) {
			if (element->getName() == name) {
				found = element;
				break;
			}
		}


		if (found != nullptr)
			lua_pushlightuserdata(L, found);
		else
		{
			lua_pushnil(L);
		}
			
		return 1;
	}

	int _GetActiveScene(lua_State* L) {
		lua_pushlightuserdata(L, (void*)&Application::get().getActiveScene());
		return 1;
	}

	int _SetActiveScene(lua_State* L) {
		// Check if lua has provided a scene ptr
		if (lua_gettop(L) != 1) return -1;

		Scene* scene = (Scene*)lua_touserdata(L, 1);
		Application::get().setActiveScene(scene);
		
		return 1;
	}

	int _DestroyEntity(lua_State* L) {
		// Check if lua has provided a entity ptr and scene ptr
		if (lua_gettop(L) != 2) return -1;

		Entity* entity = (Entity*)lua_touserdata(L, 1);
		Scene* scene = (Scene*)lua_touserdata(L, 2);

		scene->destroyEntity(entity);
		lua_pushboolean(L, true);
		
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
		lua_register(L, "_SetEntityTillingFactor", _SetTillingFactor);
		lua_register(L, "_SetEntityColor", _SetColor);
		lua_register(L, "_SetEntityPosition", _SetPosition);
		lua_register(L, "_SetEntityType", _SetType);
		lua_register(L, "_DestroyEntity", _DestroyEntity);
		
		lua_register(L, "_IsKeyDown", _IsKeyDown);
		lua_register(L, "_GetMouseX", _GetMouseX);
		lua_register(L, "_GetMouseY", _GetMouseY);
		
		lua_register(L, "_GetWindowWidth", _GetWindowWidth);
		lua_register(L, "_GetWindowHeight", _GetWindowHeight);
		lua_register(L, "_SetWindowTitle", _SetWindowTitle);
		lua_register(L, "_SetWindowMode", _SetWindowMode);
		lua_register(L, "_SetWindowOpacity", _SetWindowOpacity);
		lua_register(L, "_SetResizable", _SetResizable);

		lua_register(L, "_SetClearColor", _SetClearColor);
		lua_register(L, "_GetSceneByName", _GetSceneByName);
		lua_register(L, "_GetActiveScene", _GetActiveScene);
		lua_register(L, "_SetActiveScene", _SetActiveScene);
		
		
		int code = luaL_dofile(L, filename.c_str());
		checkLua(code);
		if (code != LUA_OK)
			hasError = true;
    }

    LuaScript::~LuaScript() {
        if (L) lua_close(L);
    }

	void LuaScript::onCreate() {
		if (hasError)
			return;
		
		lua_getglobal(L, "OnCreate");
		if (lua_isfunction(L, -1)) {
			int code = lua_pcall(L, 0, 1, 0);
			checkLua(code);
		}
	}

	void LuaScript::onUpdate(TimeStep dt) {
		if (hasError)
			return;
		
		lua_getglobal(L, "OnUpdate");
		if (lua_isfunction(L, -1)) {
			lua_pushnumber(L, (float)dt);
			int code = lua_pcall(L, 1, 1, 0);
			checkLua(code);
		}
	}

	void LuaScript::onDestoy() {
		if (hasError)
			return;
		
		lua_getglobal(L, "OnDestroy");
		if (lua_isfunction(L, -1)) {
			int code = lua_pcall(L, 0, 1, 0);
			checkLua(code);
		}
	}

	void LuaScript::onEvent(Event& event) {
		if (hasError)
			return;
		
		lua_getglobal(L, "OnEvent");
		if (lua_isfunction(L, -1)) {

			lua_newtable(L);
			// xpos = 50
			lua_pushstring(L, "name");
			lua_pushstring(L, event.getName());
			lua_settable(L, -3);

			auto map = eventToData(event);
			for (const auto& kv : map) {
				lua_pushstring(L, kv.first.c_str());
				lua_pushnumber(L, kv.second);
				lua_settable(L, -3);
			}
						
			int code = lua_pcall(L, 1, 1, 0);
			checkLua(code);
		}
	}

	void LuaScript::checkLua(int r) {
		if (r != LUA_OK) {
            auto errormsg = lua_tostring(L, -1);
            SHADO_CORE_ERROR("{0}", errormsg);
		}
	}


	// ------------------ Utility --------------------
	static std::unordered_map<std::string, int> eventToData(Event& event) {

		EventDispatcher dispatcher(event);
		std::unordered_map<std::string, int> result;
		
		dispatcher.dispatch<MouseMovedEvent>([&result](MouseMovedEvent& e) {
			result["x"] = (int)e.getX();
			result["y"] = (int)e.getY();
			return false;
		});

		dispatcher.dispatch<MouseButtonPressedEvent>([&result](MouseButtonPressedEvent& e) {
			result["button"] = e.getMouseButton();
			return false;
		});

		dispatcher.dispatch<MouseButtonReleasedEvent>([&result](MouseButtonReleasedEvent& e) {
			result["button"] = e.getMouseButton();
			return false;
		});

		dispatcher.dispatch<KeyPressedEvent>([&result](KeyPressedEvent& e) {
			result["keyCode"] = e.getKeyCode();
			return false;
		});

		return result;
	}
	
}
