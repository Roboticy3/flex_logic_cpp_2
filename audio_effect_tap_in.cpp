
#include "core/math/audio_frame.h"
#include "core/object/class_db.h"

#include "audio_effect_tap_in.h"
#include "tap_circuit_types.h"
#include "tap_sim.h"

void AudioEffectTapIn::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_simulator"), &AudioEffectTapIn::get_simulator);
	ClassDB::bind_method(D_METHOD("set_simulator", "new_simulator"), &AudioEffectTapIn::set_simulator);

	ClassDB::bind_method(D_METHOD("get_activation_delta"), &AudioEffectTapIn::get_activation_delta);
	ClassDB::bind_method(D_METHOD("set_activation_delta", "new_activation_delta"), &AudioEffectTapIn::set_activation_delta);

	ClassDB::bind_method(D_METHOD("get_pid"), &AudioEffectTapIn::get_pid);
	ClassDB::bind_method(D_METHOD("set_pid", "new_pid"), &AudioEffectTapIn::set_pid);

	ClassDB::bind_method(D_METHOD("get_line_in"), &AudioEffectTapIn::get_line_in);
	ClassDB::bind_method(D_METHOD("set_line_in", "new_line_in"), &AudioEffectTapIn::set_line_in);

	ClassDB::bind_method(D_METHOD("get_line_out"), &AudioEffectTapIn::get_line_out);
	ClassDB::bind_method(D_METHOD("set_line_out", "new_line_out"), &AudioEffectTapIn::set_line_out);

	ClassDB::bind_method(D_METHOD("get_sample_skip"), &AudioEffectTapIn::get_sample_skip);
	ClassDB::bind_method(D_METHOD("set_sample_skip", "new_sample_skip"), &AudioEffectTapIn::set_sample_skip);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "simulator", PROPERTY_HINT_RESOURCE_TYPE, "TapSim"), "set_simulator", "get_simulator");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "activation_delta", PROPERTY_HINT_RANGE, "0,1.0,0.001"), "set_activation_delta", "get_activation_delta");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "pid"), "set_pid", "get_pid");

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "line_in"), "set_line_in", "get_line_in");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "line_out"), "set_line_out", "get_line_out");

	ADD_PROPERTY(PropertyInfo(Variant::INT, "sample_skip"), "set_sample_skip", "get_sample_skip");
}

Ref<AudioEffectInstance> AudioEffectTapIn::instantiate() {
	Ref<AudioEffectTapInInstance> instance;
	instance.instantiate();
	instance->effect = this;
	return instance;
}

tap_sample_t AudioEffectTapIn::get_activation_delta() const {
	return activation_delta;
}

void AudioEffectTapIn::set_activation_delta(tap_sample_t new_activation_delta) {
	activation_delta = new_activation_delta;
}

Ref<TapSim> AudioEffectTapIn::get_simulator() const {
	return simulator;
}

void AudioEffectTapIn::set_simulator(Ref<TapSim> new_simulator) {
	simulator = new_simulator;

	if (simulator.is_null()) {
		simulator.instantiate();
	}
}

tap_label_t AudioEffectTapIn::get_pid() const {
	return pid;
}

void AudioEffectTapIn::set_pid(tap_label_t new_pid) {
	pid = new_pid;
}

bool AudioEffectTapIn::get_line_in() const {
	return line_in;
}

void AudioEffectTapIn::set_line_in(bool new_line_in) {
	line_in = new_line_in;
}

bool AudioEffectTapIn::get_line_out() const {
	return line_out;
}

void AudioEffectTapIn::set_line_out(bool new_line_out) {
	line_out = new_line_out;
}

int AudioEffectTapIn::get_sample_skip() const {
	return sample_skip;
}

void AudioEffectTapIn::set_sample_skip(int new_sample_skip) {
	sample_skip = new_sample_skip;
}

AudioEffectTapIn::AudioEffectTapIn() {
	simulator.instantiate();
}

void AudioEffectTapInInstance::process(const AudioFrame *p_src_frames, AudioFrame *p_dst_frames, int p_frame_count) {
	if (effect->line_in) {
		process_line_in(p_src_frames, p_dst_frames, p_frame_count);
	}

	if (effect->line_out) {
		for (int i = 0; i < p_frame_count; i++) {
			p_dst_frames[i] = p_src_frames[i];
		}
	}
}

void AudioEffectTapInInstance::process_line_in(const AudioFrame *p_src_frames, AudioFrame *p_dst_frames, int p_frame_count) {
	for (int i = 0; i < p_frame_count; i += effect->sample_skip) {
		AudioFrame src_frame = p_src_frames[i];

		float delta = Math::abs(src_frame.left - last_activation.left);
		delta += Math::abs(src_frame.right - last_activation.right);

		if (delta >= effect->activation_delta) {
			tap_time_t time = total_time + i * effect->simulator->get_tick_rate();
			effect->simulator->push_event(time, src_frame, effect->pid);
			last_activation = src_frame;
		}
	}

	total_time += p_frame_count * effect->simulator->get_tick_rate();
}

bool AudioEffectTapInInstance::process_silence() const {
	return true;
}