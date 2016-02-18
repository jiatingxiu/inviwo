/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 *********************************************************************************/

#include <modules/glfw/canvasprocessorwidgetglfw.h>
#include <modules/glfw/canvasglfw.h>
#include <inviwo/core/processors/canvasprocessor.h>
#include <inviwo/core/common/inviwoapplication.h>

namespace inviwo {

CanvasProcessorWidgetGLFW::CanvasProcessorWidgetGLFW(Processor* p)
    : CanvasProcessorWidget(p)
    , canvas_(util::make_unique<CanvasGLFW>(processor_->getIdentifier(), getDimensions())) {
    
    canvas_->setEventPropagator(processor_);
    canvas_->setProcessorWidgetOwner(this);
    canvas_->setWindowSize(getDimensions());
    canvas_->setWindowPosition(getPosition());
    if (ProcessorWidget::isVisible()) {
        canvas_->show();
    } else {
        canvas_->hide();
    }
}

CanvasProcessorWidgetGLFW::~CanvasProcessorWidgetGLFW() {
    this->hide();
}

void CanvasProcessorWidgetGLFW::setVisible(bool visible) {
    if (visible) {
        canvas_->show();
        static_cast<CanvasProcessor*>(processor_)->triggerQueuedEvaluation();
    } else {
        canvas_->hide();
    }
    CanvasProcessorWidget::setVisible(visible);
}

void CanvasProcessorWidgetGLFW::show() {
    CanvasProcessorWidgetGLFW::setVisible(true);
}

void CanvasProcessorWidgetGLFW::hide() {
    CanvasProcessorWidgetGLFW::setVisible(false);
}

void CanvasProcessorWidgetGLFW::setDimensions(ivec2 dim) {
    CanvasProcessorWidget::setDimensions(dim);
    canvas_->setWindowSize(uvec2(dim.x, dim.y));
}

void CanvasProcessorWidgetGLFW::setPosition(ivec2 dim) {
    CanvasProcessorWidget::setPosition(dim);
    canvas_->setWindowPosition(uvec2(dim.x, dim.y));
}


Canvas* CanvasProcessorWidgetGLFW::getCanvas() const {
    return canvas_.get();
}



} // namespace
