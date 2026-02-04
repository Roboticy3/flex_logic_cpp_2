#pragma once

#include <optional>

#include "core/object/class_db.h"
#include "core/templates/vector.h"

template <typename T>
class Labeling : public Vector<std::optional<T>> {
public:
	/*
	kind of a holdover from earlier implementations. Can still be used to
	force an element onto the end of the labeling.
	*/
	static constexpr typename Vector<std::optional<T>>::Size INVALID_LABEL = -1;

	/*
	Get the next available label for a new element. This is the lowest unused
	index in the vector.
	*/
	typename Vector<std::optional<T>>::Size get_next_available_label(typename Vector<std::optional<T>>::Size start_index = 0) const {
		for (typename Vector<std::optional<T>>::Size i = start_index; i < this->size(); i++) {
			if (this->operator[](i) == std::nullopt) {
				return i;
			}
		}
		return this->size();
	}

	typename Vector<std::optional<T>>::Size label_add(const T &element, typename Vector<std::optional<T>>::Size start_index = 0) {
		typename Vector<std::optional<T>>::Size label = get_next_available_label(start_index);
		if (label >= this->size()) {
			this->push_back(element);
		} else {
			this->set(label, element);
		}
		return label;
	}

	bool label_remove(typename Vector<std::optional<T>>::Size label) {
		if (label < this->size() && this->operator[](label).has_value()) {
			this->set(label, std::nullopt);
			return true;
		}
		return false;
	}

	std::optional<T> label_get(typename Vector<std::optional<T>>::Size label) const {
		if (label < this->size()) {
			return this->operator[](label);
		}

		return std::nullopt;
	}

	T *label_get_mut(typename Vector<std::optional<T>>::Size label) {
		if (label < this->size()) {
			std::optional<T> &o_t = this->write[label];
			if (o_t.has_value()) {
				return &o_t.value();
			} else {
				return nullptr;
			}
		}
		return nullptr;
	}
};