#pragma once
#include "input.h"
#include <utility>

#include "../Application.h"

namespace Shado {
	
	Input* Input::s_instance = new WindowsInput();

	bool WindowsInput::isKeyPressedImplementation(KeyCode keycode) {

		auto window = static_cast<GLFWwindow*>(Application::get().getWindow().getNativeWindow());
		auto state = glfwGetKey(window, (uint16_t)keycode);

		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool WindowsInput::isMouseButtonPressedImplementation(int button) {
		auto window = static_cast<GLFWwindow*>(Application::get().getWindow().getNativeWindow());
		auto state = glfwGetMouseButton(window, button);

		return state == GLFW_PRESS;
	}

	float WindowsInput::getMouseXImplementation() {
		auto pos = getMousePositionImplementation();
		return pos.first;
	}

	float WindowsInput::getMouseYImplementation() {
		auto pos = getMousePositionImplementation();
		return pos.second;
	}

	std::pair<float, float> WindowsInput::getMousePositionImplementation() {
		auto window = static_cast<GLFWwindow*>(Application::get().getWindow().getNativeWindow());
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return { (float)xpos, (float)ypos };
	}
}
