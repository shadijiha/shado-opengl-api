﻿#pragma once

#ifndef SHADO_H	
#define SHADO_H

// Shado Engine
#include "Application.h"
#include "renderer/Renderer2D.h"
#include "renderer/Shader.h"
#include "debug/Profile.h"
#include "renderer/Buffer.h"
#include "renderer/Texture2D.h"
#include "renderer/VertexArray.h"

#include "renderer/FrameBuffer.h"

#include "scene/Scene.h"
#include "scene/Components.h"
#include "scene/Entity.h"

#include "cameras/Camera.h"
#include "cameras/OrthoCamera.h"
#include "cameras/OrbitCamera.h"
#include "cameras/EditorCamera.h"


// ImGui
#include "ui/ImguiScene.h"

// Events
#include "Events/Event.h"
#include "Events/input.h"
#include "Events/ApplicationEvent.h"
#include "Events/KeyEvent.h"
#include "Events/KeyCodes.h"
#include "Events/MouseButtonCodes.h"
#include "Events/MouseEvent.h"

// Utility
#include "util/Util.h"
#include "util/random.h"
#include "util/ParticuleSystem.h"

#endif
