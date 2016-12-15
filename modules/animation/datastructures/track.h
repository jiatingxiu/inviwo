/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#ifndef IVW_TRACK_H
#define IVW_TRACK_H

#include <modules/animation/animationmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/io/serialization/serializable.h>

#include <modules/animation/datastructures/keyframe.h>
#include <modules/animation/datastructures/keyframesequence.h>
#include <modules/animation/datastructures/trackobserver.h>

namespace inviwo {

namespace animation {

/**
 * \class Track
 * \brief VERY_BRIEFLY_DESCRIBE_THE_CLASS
 * DESCRIBE_THE_CLASS
 */

class IVW_MODULE_ANIMATION_API Track : public Serializable,
                                       public TrackObservable,
                                       public KeyframeSequenceObserver {
public:
    Track() = default;
    virtual ~Track() = default;

    virtual void setEnabled(bool enabled) = 0;
    virtual bool isEnabled() const = 0;

    virtual void evaluate(Time from, Time to) const = 0;

    virtual void serialize(Serializer& s) const override = 0;
    virtual void deserialize(Deserializer& d) override = 0;
};

template <typename Key>
class TrackTyped : public Track {
public:
    TrackTyped() = default;
    virtual ~TrackTyped() = default;

    virtual void evaluate(Time from, Time to) const override = 0;

    virtual size_t size() const = 0;
    virtual KeyframeSequenceTyped<Key>& operator[](size_t i) = 0;
    virtual const KeyframeSequenceTyped<Key>& operator[](size_t i) const = 0;

    virtual void add(const KeyframeSequenceTyped<Key>& sequence) = 0;
    virtual void remove(size_t i) = 0;

    virtual void serialize(Serializer& s) const override = 0;
    virtual void deserialize(Deserializer& d) override = 0;
};


template <typename Prop, typename Key>
class TrackProperty : public TrackTyped<Key> {
public:
    TrackProperty(Prop* property) : TrackTyped<Key>(), property_(property) {}
    virtual ~TrackProperty() = default;

    virtual void setEnabled(bool enabled) override { enabled_ = enabled; }
    virtual bool isEnabled() const override { return enabled_; }

    /**
     * Track of sequences
     * ----------X======X====X-----------X=========X-------X=====X--------
     * 
     */
    virtual void evaluate(Time from, Time to) const override {
        if (!enabled_ || sequences_.empty()) return;

        auto it = std::upper_bound(
            sequences_.begin(), sequences_.end(), to,
            [](const auto& time, const auto& seq) { return time < seq->getFirst().getTime(); });

        if (it == sequences_.begin() && from > (*it)->getFirst().getTime()) {
             property_->set((*it)->getFirst().getValue());      
        } else {
            auto& seq1 = *std::prev(it);

            if (to < seq1->getLast().getTime() ) {
                property_->set(seq1->evaluate(from, to));
            } else {
                if( from < seq1->getLast().getTime() ) {
                    property_->set(seq1->getLast().getValue());
                } else if(it != sequences_.end() &&  from > (*it)->getFirst().getTime()) {
                    property_->set((*it)->getFirst().getValue());
                }
            }
        }
    };

    virtual void add(const KeyframeSequenceTyped<Key>& sequence) {
        auto it = std::upper_bound(
            sequences_.begin(), sequences_.end(), sequence.getFirst().getTime(),
            [](const auto& time, const auto& seq) { return seq->getFirst().getTime() < time; });

        if (it != sequences_.begin()) {
            if ((*std::prev(it))->getLast().getTime() > sequence.getFirst().getTime()) {
                throw Exception("Overlapping Sequence", IvwContext);
            }
        }
        if (it != sequences_.end() && (*it)->getFirst().getTime() < sequence.getLast().getTime()) {
            throw Exception("Overlapping Sequence", IvwContext);
        }

        auto inserted =
            sequences_.insert(it, std::make_unique<KeyframeSequenceTyped<Key>>(sequence));
        notifyKeyframeSequenceAdded(inserted->get());
    };

    virtual size_t size() const override {
       return sequences_.size();
    }

    virtual KeyframeSequenceTyped<Key>& operator[](size_t i) override {
        return *sequences_[i];
    }

    virtual const KeyframeSequenceTyped<Key>& operator[](size_t i) const override {
        return *sequences_[i];
    }

    virtual void remove(size_t i) override {
        auto seq = std::move(sequences_[i]);
        sequences_.erase(sequences_.begin()+i);
        notifyKeyframeSequenceRemoved(seq.get());
    }

    virtual void serialize(Serializer& s) const override {
    
    };
    virtual void deserialize(Deserializer& d) override {
    
    };

private:
    virtual void onKeyframeSequenceMoved(KeyframeSequence* key) override {
        std::stable_sort(sequences_.begin(), sequences_.end(), [](const auto& a, const auto& b) {
            return a->getFirst().getTime() < b->getFirst().getTime();
        });
        /// Do validation? 
    }

    bool enabled_ = true;
    Prop* property_;

    // Sorted list of non-overlapping sequences of key frames
    std::vector<std::unique_ptr<KeyframeSequenceTyped<Key>>> sequences_;
};



} // namespace

} // namespace

#endif // IVW_TRACK_H

