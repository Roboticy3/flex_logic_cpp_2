#pragma once

#include "core/math/audio_frame.h"
#include "circuit.h"

//base tap types
typedef unsigned int tap_label_t; //used to identify components and pins in separate collections
typedef unsigned int tap_time_t; //sample count time
typedef AudioFrame tap_frame_t;

//event tap types
typedef circuit_event_t<tap_frame_t, tap_time_t, tap_label_t, tap_label_t> tap_event_t;
typedef circuit_queue_t<tap_frame_t, tap_time_t, tap_label_t, tap_label_t> tap_queue_t;

//component tap types
typedef circuit_pin_t<tap_frame_t, tap_time_t, tap_label_t> tap_pin_t;
typedef circuit_component_type_t<tap_time_t, tap_label_t, tap_event_t, tap_queue_t, void*> tap_component_type_t;
typedef circuit_component_t<tap_frame_t, tap_label_t, tap_component_type_t> tap_component_t;
typedef solver_t<tap_label_t, tap_component_t, tap_event_t, tap_queue_t, tap_time_t> tap_solver_t;
