#pragma once

#include "core/object/class_db.h"
#include "servers/audio/audio_effect.h"
#include "core/math/audio_frame.h"

#include "frame_cache.h"

class AudioEffectTapInstance : public AudioEffectInstance {
  GDCLASS(AudioEffectTapInstance, AudioEffectInstance);

  friend class AudioEffectTap;

  Ref<FrameCache> frame_cache;
  
  public:
    virtual void process(const AudioFrame *p_src_frames, AudioFrame *p_dst_frames, int p_frame_count) override;
    virtual bool process_silence() const override;
};

class AudioEffectTap : public AudioEffect {
  GDCLASS(AudioEffectTap, AudioEffect);

  friend class AudioEffectTapInstance;

  Ref<FrameCache> frame_cache;

  protected:
    static void _bind_methods();

  public:
    virtual Ref<AudioEffectInstance> instantiate() override;

    Ref<FrameCache> get_frame_cache() const;
    void set_frame_cache(const Ref<FrameCache> &p_frame_cache);

    AudioEffectTap();
};