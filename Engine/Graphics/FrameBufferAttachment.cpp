#include "FrameBufferAttachment.h"
#include "Image.h"

namespace Engine {
    FrameBufferAttachment::FrameBufferAttachment(std::shared_ptr<Engine::Device> device, const FrameBufferAttachmentDecs& decs) {
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
    }

    FrameBufferAttachment::~FrameBufferAttachment() {
        delete image;
        delete view;
    }

    int FrameBufferAttachment::GetAspectMask(Format format) {
        switch (format) {
            case D16:
            case D32:
                return Aspect::DEPTH;
            case D16S8:
            case D24S8:
            case D32S8:
                return Aspect::DEPTH | Aspect::STENCIL;
            default:
                return Aspect::COLOR;
        }
    }
}