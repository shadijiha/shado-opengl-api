#pragma once
#ifndef INPUT_H
#define INPUT_H
#include <utility>

#include "KeyCodes.h"

namespace Shado {
	
	class Input {
	public:
		inline static bool isKeyPressed(KeyCode keycode) { return s_instance->isKeyPressedImplementation(keycode); }
		inline static bool isMouseButtonPressed(int button) { return s_instance->isMouseButtonPressedImplementation(button); }
		inline static float getMouseX() { return s_instance->getMouseXImplementation(); }
		inline static float getMouseY() { return s_instance->getMouseYImplementation(); }

		inline static std::pair<float, float> getMousePosition() { return s_instance->getMousePositionImplementation(); }

	protected:
		virtual bool isKeyPressedImplementation(KeyCode keycode) = 0;

		virtual bool isMouseButtonPressedImplementation(int button) = 0;

		virtual float getMouseXImplementation() = 0;

		virtual float getMouseYImplementation() = 0;

		virtual std::pair<float, float> getMousePositionImplementation() = 0;

	private:
		static Input* s_instance;
	};

	class WindowsInput : public Input
	{
	protected:
		virtual bool isKeyPressedImplementation(KeyCode keycode) override;

		virtual bool isMouseButtonPressedImplementation(int button) override;

		virtual float getMouseXImplementation() override;

		virtual float getMouseYImplementation() override;

		virtual std::pair<float, float> getMousePositionImplementation() override;
	};
	
}

#endif // !INPUT_H
