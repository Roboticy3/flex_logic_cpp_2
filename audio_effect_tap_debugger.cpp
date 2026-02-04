#include "audio_effect_tap_debugger.h"
#include "core/variant/variant.h"

void AudioEffectTapDebuggerQuery::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_means"), &AudioEffectTapDebuggerQuery::get_means);
	ClassDB::bind_method(D_METHOD("get_stddevs"), &AudioEffectTapDebuggerQuery::get_stddevs);
	ClassDB::bind_method(D_METHOD("get_pids"), &AudioEffectTapDebuggerQuery::get_pids);
	ClassDB::bind_method(D_METHOD("get_start_time"), &AudioEffectTapDebuggerQuery::get_start_time);
	ClassDB::bind_method(D_METHOD("get_end_time"), &AudioEffectTapDebuggerQuery::get_end_time);

	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR2_ARRAY, "means"), "", "get_means");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR2_ARRAY, "stddevs"), "", "get_stddevs");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT64_ARRAY, "pids"), "", "get_pids");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "start_time"), "", "get_start_time");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "end_time"), "", "get_end_time");
}

PackedVector2Array AudioEffectTapDebuggerQuery::get_means() const {
	return means;
}

PackedVector2Array AudioEffectTapDebuggerQuery::get_stddevs() const {
	return stddevs;
}

PackedInt64Array AudioEffectTapDebuggerQuery::get_pids() const {
	return pids;
}

tap_time_t AudioEffectTapDebuggerQuery::get_start_time() const {
	return start_time;
}

tap_time_t AudioEffectTapDebuggerQuery::get_end_time() const {
	return end_time;
}

void AudioEffectTapDebugger::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_simulator"), &AudioEffectTapDebugger::get_simulator);
	ClassDB::bind_method(D_METHOD("set_simulator", "new_simulator"), &AudioEffectTapDebugger::set_simulator);

	ClassDB::bind_method(D_METHOD("get_monitor_pids"), &AudioEffectTapDebugger::get_monitor_pids);
	ClassDB::bind_method(D_METHOD("set_monitor_pids", "new_monitor_pids"), &AudioEffectTapDebugger::set_monitor_pids);

	ClassDB::bind_method(D_METHOD("get_live"), &AudioEffectTapDebugger::get_live);
	ClassDB::bind_method(D_METHOD("set_live", "new_live"), &AudioEffectTapDebugger::set_live);

	ClassDB::bind_method(D_METHOD("get_sample_count"), &AudioEffectTapDebugger::get_sample_count);
	ClassDB::bind_method(D_METHOD("set_sample_count", "new_sample_count"), &AudioEffectTapDebugger::set_sample_count);

	ClassDB::bind_method(D_METHOD("get_samples"), &AudioEffectTapDebugger::get_samples);
	ClassDB::bind_method(D_METHOD("get_stats"), &AudioEffectTapDebugger::get_stats);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "simulator", PROPERTY_HINT_RESOURCE_TYPE, "TapSim"), "set_simulator", "get_simulator");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT64_ARRAY, "monitor_pids"), "set_monitor_pids", "get_monitor_pids");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "live"), "set_live", "get_live");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "sample_count"), "set_sample_count", "get_sample_count");
}

Ref<TapSim> AudioEffectTapDebugger::get_simulator() const {
	return ls.get_simulator();
}

void AudioEffectTapDebugger::set_simulator(Ref<TapSim> new_simulator) {
	ls.set_simulator(new_simulator);
}

PackedInt64Array AudioEffectTapDebugger::get_monitor_pids() const {
	return ls.get_live_pids();
}

void AudioEffectTapDebugger::set_monitor_pids(PackedInt64Array new_monitor_pids) {
	ls.set_live_pids(new_monitor_pids);
}

bool AudioEffectTapDebugger::get_live() const {
	return ls.get_live();
}

void AudioEffectTapDebugger::set_live(bool new_live) {
	ls.set_live(new_live);
}

int AudioEffectTapDebugger::get_sample_count() const {
	return sample_count;
}

void AudioEffectTapDebugger::set_sample_count(int new_sample_count) {
	sample_count = new_sample_count;
}

TypedArray<PackedVector2Array> AudioEffectTapDebugger::get_samples() const {
	TypedArray<PackedVector2Array> result;

	for (tap_label_t pid : get_monitor_pids()) {
		PackedVector2Array pid_result;
		if (samples.has(pid)) {
			Vector<AudioFrame> frames = samples.get(pid);

			pid_result.resize(frames.size());
			for (int i = 0; i < frames.size(); i++) {
				pid_result.set(i, frames[i]);
			}
		}
		result.push_back(pid_result);
	}

	return result;
}

Ref<AudioEffectTapDebuggerQuery> AudioEffectTapDebugger::get_stats() const {
	Ref<AudioEffectTapDebuggerQuery> query;
	query.instantiate();

	// Get the list of PIDs we're monitoring
	PackedInt64Array pids = get_monitor_pids();
	int pid_count = pids.size();
	if (pid_count == 0) {
		return query; // No PIDs to process
	}

	// Prepare arrays to store results
	PackedVector2Array means;
	PackedVector2Array stddevs;
	means.resize(pid_count);
	stddevs.resize(pid_count);

	// For each PID, calculate mean and standard deviation
	for (int i = 0; i < pid_count; i++) {
		tap_label_t pid = pids[i];
		Vector2 mean(0, 0);
		Vector2 variance(0, 0);

		if (!samples.has(pid)) {
			means.set(i, Vector2(NAN, NAN));
			stddevs.set(i, Vector2(NAN, NAN));
			continue;
		}

		const Vector<AudioFrame> &frames = samples.get(pid);
		int frame_count = frames.size();
		if (frame_count == 0) {
			means.set(i, mean);
			stddevs.set(i, Vector2(0, 0));
			continue;
		}

		// Calculate mean
		for (int j = 0; j < frame_count; j++) {
			mean.x += frames[j].left;
			mean.y += frames[j].right;
		}
		mean /= frame_count;

		// Calculate variance
		for (int j = 0; j < frame_count; j++) {
			float diff_left = frames[j].left - mean.x;
			float diff_right = frames[j].right - mean.y;
			variance.x += diff_left * diff_left;
			variance.y += diff_right * diff_right;
		}
		variance /= frame_count;

		// Store results
		means.set(i, mean);
		stddevs.set(i, Vector2(Math::sqrt(variance.x), Math::sqrt(variance.y)));
	}

	// Set the query results
	query->means = means;
	query->stddevs = stddevs;
	query->pids = pids;
	query->start_time = samples_start_time;
	query->end_time = samples_end_time;

	return query;
}

Ref<AudioEffectInstance> AudioEffectTapDebugger::instantiate() {
	Ref<AudioEffectTapDebuggerInstance> instance;
	instance.instantiate();
	instance->effect = this;
	return instance;
}

void AudioEffectTapDebuggerInstance::process(const AudioFrame *p_src_frames, AudioFrame *p_dst_frames, int p_frame_count) {
	if (effect->get_live()) {
		process_live(p_src_frames, p_dst_frames, p_frame_count);
	} else {
	}

	//always leave audio unbothered. We're just probing here.
	for (int i = 0; i < p_frame_count; i++) {
		p_dst_frames[i] = p_src_frames[i];
	}
}

bool AudioEffectTapDebuggerInstance::process_silence() const {
	return true;
}

void AudioEffectTapDebuggerInstance::process_live(const AudioFrame *p_src_frames, AudioFrame *p_dst_frames, int p_frame_count) {
	for (tap_label_t pid : effect->get_monitor_pids()) {
		if (!effect->samples.has(pid)) {
			effect->samples.insert(pid, Vector<AudioFrame>());
		}

		//originally i wanted to do a ring buffer, but the builtin class' interface is too hard lol
		while (effect->samples.get(pid).size() >= effect->sample_count) {
			effect->samples.get(pid).remove_at(0);
		}

		effect->samples.get(pid).push_back(p_src_frames[p_frame_count - 1]);

		//print_line("pid ", pid, " has ", p_src_frames[p_frame_count - 1].left, ", ", p_src_frames[p_frame_count - 1].right, " values");
	}

	// Update total time
	tap_time_t simulator_ticks_passed = p_frame_count * effect->ls.get_simulator()->get_tick_rate();
	effect->samples_end_time += simulator_ticks_passed;
	effect->samples_start_time = effect->samples_end_time - effect->sample_count * simulator_ticks_passed;
}
