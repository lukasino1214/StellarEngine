#include "framebuffer_attachment.h"
#include "image.h"
#include "vk_types.h"

namespace Engine {
    FrameBufferAttachment::FrameBufferAttachment(std::shared_ptr<Device> device, const FrameBufferAttachmentDescription& _description) : description{_description}, dimensions{_description.dimensions}, final_layout{_description.final_layout} {
        image = new Image(device, {
                .format = description.format,
                .dimensions = { description.dimensions.x, description.dimensions.y, description.dimensions.z },
                .usage = description.usage,
        });

        image_view = new ImageView(device, {
                .format = description.format,
                .aspect_mask = (ImageAspectFlags)get_aspect_mask(description.format),
                .image = image
        });

        switch (description.format) {
            case ImageFormat::D16_UNORM:
            case ImageFormat::D32_SFLOAT:
            case ImageFormat::D16_UNORM_S8_UINT:
            case ImageFormat::D24_UNORM_S8_UINT:
            case ImageFormat::D32_SFLOAT_S8_UINT:
                is_depth = true;
                break;
            default:
                is_depth = false;
                break;
        }
    }

    FrameBufferAttachment::~FrameBufferAttachment() {
        delete image;
        delete image_view;
    }

    int FrameBufferAttachment::get_aspect_mask(ImageFormat format) {
        switch (format) {
            case ImageFormat::D16_UNORM:
            case ImageFormat::D32_SFLOAT:
                return ImageAspectFlagBits::DEPTH;
            case ImageFormat::D16_UNORM_S8_UINT:
            case ImageFormat::D24_UNORM_S8_UINT:
            case ImageFormat::D32_SFLOAT_S8_UINT:
                return ImageAspectFlagBits::DEPTH | ImageAspectFlagBits::STENCIL;
            default:
                return ImageAspectFlagBits::COLOR;
        }
    }
}