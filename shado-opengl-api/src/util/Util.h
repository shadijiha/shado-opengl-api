#pragma once

#ifndef UTIL_H
#define UTIL_H
#include <functional>
#include <string>
#include <thread>
#include <windows.h>
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include <clocale>
#include <locale>
#include <filesystem>

#define FILE_PATH std::filesystem::current_path().u8string()

namespace Shado {
	struct Color
	{
		Color(float r, float g, float b, float a);
		Color(float r, float g, float b);
		Color(const Color& c);
		Color();
		~Color();

		Color& operator=(const Color& other);
		operator glm::vec4() const;
		explicit operator glm::vec3() const;

		static Color fromRGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
		static Color fromRGB(unsigned char r, unsigned char g, unsigned char b);
		static Color fromHex(const std::string& hexColor);
		static Color fromHex(long hexColor);
		static Color random();

		const float& red()		const { return _red; }
		const float& green()	const { return _green; }
		const float& blue()		const { return _blue; }
		const float& alpha()	const { return _alpha; }

		// Static variables
		static const Color WHITE;
		static const Color BLACK;
		static const Color RED;
		static const Color GREEN;
		static const Color BLUE;
		static const Color MAGENTA;
		static const Color CYAN;
		static const Color YELLOW;

	private:
		static float validateInput(float input, float min, float max);

	private:
		float _red;
		float _green;
		float _blue;
		float _alpha;
	};

	struct TimeStep {
		TimeStep(double ms) : m_ms(ms) {}

		double toMillis() const { return m_ms; }
		double toSeconds() const { return m_ms / 1000; }
		double toFPS() const { return 1.0 / m_ms; }

		operator float() const { return (float)m_ms; }
		explicit operator double() const { return m_ms; }

	private:
		double m_ms;
	};

	struct IntervalObject {

		IntervalObject(std::thread* t, bool* flag)
			: id(rand() % UINT64_MAX), thread(t), executing(flag)
		{}

		void terminate() {
			*executing = false;
		}

	private:
		const uint64_t id;
		std::thread* thread;
		bool* executing;
	};

	IntervalObject setInterval(std::function<void()> task, unsigned long intervalMS);

	struct TimeoutObject {

		TimeoutObject(std::thread* t, bool* flag)
			: id(rand() % UINT64_MAX), thread(t), executing(flag)
		{}

		void cancel() {
			*executing = false;
		}

	private:
		const uint64_t id;
		std::thread* thread;
		bool* executing;
	};
	TimeoutObject setTimeout(std::function<void()> task, unsigned long startAfterMS);
}

/****************** Costum smart ptr ********************/
template<typename T>
using ScopedPtr = std::unique_ptr<T>;

template<typename T>
using Ref = std::shared_ptr<T>;

#endif
