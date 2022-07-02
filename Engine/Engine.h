//
// Created by lukas on 15.09.21.
//

#ifndef ENGINE_ENGINE_H
#define ENGINE_ENGINE_H

#include "Core/Base.h"
#include "Core/Log.h"
#include "Core/Timestamp.h"
#include "Core/Input.h"
#include "Core/Window.h"

#include "Data/Scene.h"
#include "Data/Entity.h"
#include "Data/SceneSerializer.h"

#include "Graphics/Device.h"
#include "Graphics/Model.h"
#include "Graphics/Pipeline.h"
#include "Graphics/SwapChain.h"
#include "Graphics/Renderer.h"
#include "Graphics/Camera.h"
#include "Graphics/ImGuiLayer.h"
#include "Graphics/Buffer.h"
#include "Graphics/Descriptors.h"
#include "Graphics/Texture.h"
#include "Graphics/Core.h"
#include "Graphics/Image.h"

#include "System/RenderSystem.h"
#include "System/GridSystem.h"
#include "System/PhysicsSystem.h"
#include "System/PointLightSystem.h"
#include "System/OffScreen.h"
#include "System/PostProcessingSystem.h"
#include "System/ShadowSystem.h"
#include "System/DefferedRenderingSystem.h"

#include "Scripting/NativeScript.h"

#include "Math/Math.h"

#endif //ENGINE_ENGINE_H
