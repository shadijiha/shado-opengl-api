#pragma once

#ifndef SHADO_H	
#define SHADO_H

// Shado Engine
#include "Application.h"
#include "renderer/Renderer2D.h"
#include "renderer/Renderer3D.h"
#include "renderer/Shader.h"
#include "Debug.h"
#include "renderer/Buffer.h"
#include "renderer/Texture2D.h"
#include "renderer/VertexArray.h"
#include "Entity.h"
#include "renderer/FrameBuffer.h"

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
#include "Objects3D/Object3D.h"
#include "Objects3D/Sphere.h"
#include "Objects3D/Cube.h"


#endif
