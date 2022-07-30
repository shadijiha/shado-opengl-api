#pragma once
#include "renderer/Texture2D.h"
#include "util/Util.h"

namespace Shado {
	class ConsolePanel {
	public:
		ConsolePanel();

		void onImGuiRender();

	private:
		Ref<Texture2D> detectMessageType(const std::string& message) const;
	private:
		Ref<Texture2D> m_ErrorIcon = CreateRef<Texture2D>("resources/icons/error.png");
		Ref<Texture2D> m_WarnIcon = CreateRef<Texture2D>("resources/icons/warn.png");
		Ref<Texture2D> m_InfoIcon = CreateRef<Texture2D>("resources/icons/info.png");
	};
}
