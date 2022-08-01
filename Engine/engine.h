#pragma once

#include "core/logger.h"
#include "core/timestamp.h"
#include "core/input_manager.h"
#include "core/window.h"

#include "data/scene.h"
#include "data/entity.h"
#include "data/scene_serializer.h"

#include "graphics/device.h"
#include "graphics/model.h"
#include "graphics/pipeline.h"
#include "graphics/swapchain.h"
#include "graphics/renderer.h"
#include "graphics/camera.h"
#include "graphics/imgui_layer.h"
#include "graphics/buffer.h"
#include "graphics/descriptor_set.h"
#include "graphics/texture.h"
#include "graphics/core.h"
#include "graphics/image.h"

#include "system/rendering_system.h"
#include "system/grid_system.h"
#include "system/point_light_system.h"
#include "system/offscreen_system.h"
#include "system/postprocessing_system.h"
#include "system/shadow_system.h"
#include "system/deferred_rendering_system.h"
#include "system/pbr_system.h"
#include "system/bloom_system.h"

#include "scripting/native_script.h"

#include "math/math.h"