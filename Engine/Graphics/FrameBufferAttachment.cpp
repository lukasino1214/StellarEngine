#include "FrameBufferAttachment.h"
#include "Image.h"

namespace Engine {
    FrameBufferAttachment::FrameBufferAttachment(std::shared_ptr<Engine::Device> device, const FrameBufferAttachmentDecs& decs) : dimensions{decs.dimensions}, finalLayout{decs.finalLayout} {
        image = new Image(device, {
                .format = decs.format,
                .dimensions = { decs.dimensions.x, decs.dimensions.y, decs.dimensions.z },
                .usage = decs.usage,
        });

        view = new ImageView(device, {
                .format = decs.format,
                .aspectMask = (Aspect)GetAspectMask(decs.format),
                .image = image
        });
        isDepth = decs.isDepth;
    }

    FrameBufferAttachment::~FrameBufferAttachment() {
        delete image;
        delete view;
    }

    int FrameBufferAttachment::GetAspectMask(Format format) {
        switch (format) {
            case D16_UNORM:
            case D32_SFLOAT:
                return Aspect::DEPTH;
            case D16_UNORM_S8_UINT:
            case D24_UNORM_S8_UINT:
            case D32_SFLOAT_S8_UINT:
                return Aspect::DEPTH | Aspect::STENCIL;
            default:
                return Aspect::COLOR;
        }
    }
}