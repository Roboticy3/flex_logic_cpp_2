#include <mutex>

#include "core/object/object.h"
#include "core/variant/variant.h"
#include "servers/audio/audio_server.h"

#include "audio_effect_tap_out.h"

void AudioEffectTapOut::_bind_methods() {
	ClassDB::bind_method(D_METHOD("any_input_connected"), &AudioEffectTapOut::any_input_connected);

	ClassDB::bind_method(D_METHOD("get_simulator"), &AudioEffectTapOut::get_simulator);
	ClassDB::bind_method(D_METHOD("set_simulator", "new_simulator"), &AudioEffectTapOut::set_simulator);

	ClassDB::bind_method(D_METHOD("get_output_pids"), &AudioEffectTapOut::get_output_pids);
	ClassDB::bind_method(D_METHOD("set_output_pids", "new_output_pids"), &AudioEffectTapOut::set_output_pids);

	ClassDB::bind_method(D_METHOD("get_input_pids"), &AudioEffectTapOut::get_input_pids);
	ClassDB::bind_method(D_METHOD("set_input_pids", "new_input_pids"), &AudioEffectTapOut::set_input_pids);

	ClassDB::bind_method(D_METHOD("get_reference_sim"), &AudioEffectTapOut::get_reference_sim);
	ClassDB::bind_method(D_METHOD("set_reference_sim", "new_reference_sim"), &AudioEffectTapOut::set_reference_sim);

	ClassDB::bind_method(D_METHOD("get_live"), &AudioEffectTapOut::get_live);
	ClassDB::bind_method(D_METHOD("set_live", "new_live"), &AudioEffectTapOut::set_live);

	ClassDB::bind_method(D_METHOD("get_sample_skip"), &AudioEffectTapOut::get_sample_skip);
	ClassDB::bind_method(D_METHOD("set_sample_skip", "new_sample_skip"), &AudioEffectTapOut::set_sample_skip);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "simulator", PROPERTY_HINT_RESOURCE_TYPE, "TapSim"), "set_simulator", "get_simulator");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT64_ARRAY, "output_pids"), "set_output_pids", "get_output_pids");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT64_ARRAY, "input_pids"), "set_input_pids", "get_input_pids");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "reference_sim", PROPERTY_HINT_RESOURCE_TYPE, "ReferenceSim"), "set_reference_sim", "get_reference_sim");
	
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "live", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR), "set_live", "get_live");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "sample_skip", PROPERTY_HINT_ENUM_SUGGESTION, "1,2,4"), "set_sample_skip", "get_sample_skip");
}

bool AudioEffectTapOut::any_input_connected() {
	return false;
}

Ref<TapSim> AudioEffectTapOut::get_simulator() const {
	return ls_out.get_simulator();
}

void AudioEffectTapOut::set_simulator(Ref<TapSim> new_simulator) {
	ls_out.set_simulator(new_simulator);
	ls_in.set_simulator(new_simulator);
}

PackedInt64Array AudioEffectTapOut::get_output_pids() const {
	return ls_out.get_live_pids();
}

void AudioEffectTapOut::set_output_pids(PackedInt64Array new_output_pids) {
	ls_out.set_live_pids(new_output_pids);
}

PackedInt64Array AudioEffectTapOut::get_input_pids() const {
	return ls_in.get_live_pids();
}

void AudioEffectTapOut::set_input_pids(PackedInt64Array new_input_pids) {
	ls_in.set_live_pids(new_input_pids);
}

Ref<ReferenceSim> AudioEffectTapOut::get_reference_sim() const {
	return reference_sim;
}

void AudioEffectTapOut::set_reference_sim(Ref<ReferenceSim> new_reference_sim) {
	reference_sim = new_reference_sim;
}

bool AudioEffectTapOut::get_live() const {
	return ls_out.get_live() && ls_in.get_live();
}

void AudioEffectTapOut::set_live(bool new_live) {
	ls_out.set_live(new_live);
	ls_in.set_live(new_live);
}

int AudioEffectTapOut::get_sample_skip() const {
	return sample_skip;
}
void AudioEffectTapOut::set_sample_skip(int new_sample_skip) {
	if (new_sample_skip < 1 || 512 % new_sample_skip != 0) {
		WARN_PRINT("Suggested sample skip does not divide 512, the typical batch count for audio processing. Expect phase issues.");
	}

	if (new_sample_skip > 4) {
		WARN_PRINT("Suggested sample skip would produce audio at a resolution under 10kHz at typical 44kHz audio processing. Expect choppiness");
	}

	sample_skip = new_sample_skip;
}

Ref<AudioEffectInstance> AudioEffectTapOut::instantiate() {
	Ref<AudioEffectTapOutInstance> instance;
	instance.instantiate();
	instance->effect = this;

	instance->inputs.resize(get_input_pids().size());
	instance->outputs.resize(get_output_pids().size());

	instance->mix_rate = AudioServer::get_singleton()->get_mix_rate();

	return instance;
}

void AudioEffectTapOutInstance::process(const AudioFrame *p_src_frames, AudioFrame *p_dst_frames, int p_frame_count) {
	if (effect->get_live()) {
		process_live(p_src_frames, p_dst_frames, p_frame_count);
	}

	/* Insanity check
	const double frequency = 440.0;
	  const double sample_rate = AudioServer::get_singleton()->get_mix_rate();
	  const double phase_inc = 2.0 * Math::PI * frequency / sample_rate;

	  for (int i = 0; i < p_frame_count; i++) {
		  float sample = static_cast<float>(std::sin(phase));
		  phase += phase_inc;
		  if (phase > 2.0 * Math::PI)
			  phase -= 2.0 * Math::PI;

		  p_dst_frames[i].left = sample;
		  p_dst_frames[i].right = sample;
	  }
	*/
}

bool AudioEffectTapOutInstance::process_silence() const {
	return true;
}

void AudioEffectTapOutInstance::process_live(const AudioFrame *p_src_frames, AudioFrame *p_dst_frames, int p_frame_count) {
	
	//get mutex permission from the simulator so it can keep some operations exclusive
	std::mutex &mutex = effect->get_simulator()->get_mutex();
	
	if (!mutex.try_lock()) {
		WARN_PRINT("Failed to lock simulator mutex, skipping processing");

		//Zero out the audio in this case
		for (int i = 0; i < p_frame_count; i++) {
			p_dst_frames[i] = AudioFrame(0.0, 0.0);
		}

		return;
	}
	
	tap_time_t target_time = total_time + p_frame_count * effect->get_simulator()->get_tick_rate();

	if (effect->get_simulator()->get_latest_event_time() <= target_time) {
		WARN_PRINT("Not enough events in simulator, skipping processing");

		mutex.unlock();
		return;
	}

	//simulate the circuit
	//technically simulating behind by one `process` call because `total_time`
	//  isn't updated yet. But if this instance is synced properly with the input
	//  instances, that's the only way to stop it from trying to read events that
	//  don't exist yet. Should cause a negligible delay.
	Ref<TapPatchBay> patch_bay = effect->get_simulator()->get_patch_bay();
	for (int i = 0; i < p_frame_count; i += effect->sample_skip) {
		tap_time_t time = total_time + i * effect->get_simulator()->get_tick_rate();

		const PackedInt64Array &input_pids = effect->get_input_pids();
		for (int i = 0; i < input_pids.size(); i++) {
			inputs[i] = patch_bay->get_pin_state_internal(input_pids[i]);
		}

		effect->get_simulator()->process_to(time);

		const PackedInt64Array &output_pids = effect->get_output_pids();
		for (int i = 0; i < output_pids.size(); i++) {
			outputs[i] = patch_bay->get_pin_state_internal(output_pids[i]);
		}

		Ref<ReferenceSim> reference_sim = effect->get_reference_sim();
		if (reference_sim.is_valid()) {
			reference_sim->measure_error_internal(outputs, inputs, 1.0f / mix_rate);
		}

		AudioFrame total;
		for (tap_label_t pid : effect->get_output_pids()) {
			AudioFrame frame = patch_bay->get_pin_state_internal(pid);
			if (Vector2i(frame.l, frame.r) == patch_bay->get_state_missing()) {
				continue;
			}
			total += frame;
		}
		p_dst_frames[i] = total;
	}

	total_time = target_time;
	
	mutex.unlock();
}