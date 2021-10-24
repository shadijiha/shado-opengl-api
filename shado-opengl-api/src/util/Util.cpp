#include "Util.h"

#include "../Debug.h"


namespace Shado {

	const Color Color::WHITE = {1, 1, 1};
	const Color Color::BLACK = {0, 0, 0};
	const Color Color::RED = {1, 0, 0};
	const Color Color::GREEN = {0, 1, 0};
	const Color Color::BLUE = {0, 0, 1};
	const Color Color::MAGENTA = {1, 0, 1};
	const Color Color::CYAN = {0, 1, 1};
	const Color Color::YELLOW = {1, 1, 0};

	
	Color::Color(float r, float g, float b, float a) :
		_red(validateInput(r, 0.0, 1.0)),
		_green(validateInput(g, 0.0, 1.0)),
		_blue(validateInput(b, 0.0, 1.0)),
		_alpha(validateInput(a, 0.0, 1.0))
	{
	}

	Color::Color(float r, float g, float b)
		: Color(r, g, b, 1.0)
	{
	}

	Color::Color(const Color& c)
		: Color(c._red, c._green, c._blue, c._alpha)
	{
	}

	Color::Color()
		: Color(0, 0, 0)
	{
	}

	Color::~Color() {
	}

	Color& Color::operator=(const Color& other) {
		_red = other._red;
		_green = other._green;
		_blue = other._blue;
		_alpha = other._alpha;
		return *this;
	}

	Color::operator glm::vec4() const {
		return glm::vec4(_red, _green, _blue, _alpha);
	}

	Color::operator glm::vec3() const {
		return glm::vec3(_red, _green, _blue);
	}

	Color Color::fromRGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {

		validateInput(r, 0, 255);
		validateInput(g, 0, 255);
		validateInput(b, 0, 255);
		validateInput(a, 0, 255);

		return Color(r / 255.0, g / 255.0, b / 255.0, a / 255.0);
	}

	Color Color::fromRGB(unsigned char r, unsigned char g, unsigned char b) {
		return fromRGBA(r, g, b, 255);
	}

	Color Color::fromHex(const std::string& hexColor) {
		// TODO : must convert from a base 16 not base 10
		return Color::fromRGB(
			std::stoi(hexColor.substr(1, 3)),
			std::stoi(hexColor.substr(3, 5)),
			std::stoi(hexColor.substr(5, 7)));
	}

	Color Color::fromHex(long hexColor) {
		double r = ((hexColor >> 16) & 0xFF) / 255.0;  // Extract the RR byte
		double g = ((hexColor >> 8) & 0xFF) / 255.0;   // Extract the GG byte
		double b = ((hexColor) & 0xFF) / 255.0;        // Extract the BB byte

		return Color::fromRGB(r, g, b);
	}

	Color Color::random() {
		return Color::fromRGB(rand() % 255, rand() % 255, rand() % 255);
	}

	float Color::validateInput(float input, float min, float max) {
		if (input < min || input > max) {

			const std::string message = "Invalid input! " + std::to_string(input) +
				" must be less that " + std::to_string(max) + " and greater than "
				+ std::to_string(min) + " (got " + std::to_string(input) + ")";

			SHADO_ERROR(message);
			SHADO_ASSERT(false, message);
		}

		return input;
	}

	// =========================== OTHERS ==============================

	IntervalObject setInterval(std::function<void()> task, unsigned long interval) {

		bool* flag = new bool(true);

		// TODO: Make it a shared Pointer
		std::thread* thread = new std::thread([=]() {
			while (*flag) {
				std::this_thread::sleep_for(std::chrono::milliseconds(interval));
				task();
			}
			});
		thread->detach();

		IntervalObject obj(thread, flag);

		return obj;
	}

	TimeoutObject setTimeout(std::function<void()> task, unsigned long startAfterMS) {

		bool* flag = new bool(true);

		std::thread thread([=]() {
			std::this_thread::sleep_for(std::chrono::milliseconds(startAfterMS));
			if (*flag)
				task();
			});
		thread.detach();

		TimeoutObject obj(&thread, flag);

		return obj;
	}	
}
