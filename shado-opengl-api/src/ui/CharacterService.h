#pragma once
#include <unordered_map>

#include "glm/vec2.hpp"
#include "../Texture2D.h"

namespace Shado {

	struct Character {
		Texture2D TextureID;  // ID handle of the glyph texture
		glm::ivec2   Size;       // Size of glyph
		glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
		unsigned int Advance;    // Offset to advance to next glyph
	};

	class CharacterService
	{
		CharacterService();


	private:
		static std::unordered_map<char, Character> characters;
	};

}
