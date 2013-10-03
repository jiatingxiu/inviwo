#ifndef IVW_FRAMEBUFFEROBJECT_H
#define IVW_FRAMEBUFFEROBJECT_H

#include <modules/opengl/openglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/opengl/inviwoopengl.h>
#include "texture2d.h"

namespace inviwo {

class IVW_MODULE_OPENGL_API FrameBufferObject {

public:
    FrameBufferObject();
    ~FrameBufferObject();

    void activate();
    static void deactivate();

    void attachTexture(Texture2D* texture, GLenum attachementID = GL_COLOR_ATTACHMENT0_EXT);
    GLenum attachTexture(Texture2D* texture, int attachmentNumber, bool attachFromRear);
    void detachTexture(GLenum attachementID);
    void detachAllTextures();
    std::vector<GLenum> getColorAttachementIDs() const { return attachedColorTextures_; }

    bool hasDepthAttachment() const { return hasDepthAttachment_; }
    bool hasStencilAttachment() const { return hasStencilAttachment_; }

    void checkStatus();

    void setRead_Blit(bool set=true) const;
    void setDraw_Blit(bool set=true);

private:
    unsigned int id_;
    bool hasDepthAttachment_;
    bool hasStencilAttachment_;
    std::vector<GLenum> attachedColorTextures_;
};

} // namespace

#endif // IVW_FRAMEBUFFEROBJECT_H
