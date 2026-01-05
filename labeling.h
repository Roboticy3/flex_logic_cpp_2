#include <optional>

#include "core/object/class_db.h"
#include "core/templates/vector.h"

template<typename T>
class Labeling : public Vector<std::optional<T>> {

  public:
    /*
    Get the next available label for a new element. This is the lowest unused
    index in the vector.
    */
    typename Vector<std::optional<T>>::Size get_next_available_label() const {
      for (typename Vector<std::optional<T>>::Size i = 0; i < this->size(); i++) {
        if (this->operator[](i) == std::nullopt) {
          return i;
        }
      }
      return this->size();
    }

    typename Vector<std::optional<T>>::Size label_add(const T &element) {
      typename Vector<std::optional<T>>::Size label = get_next_available_label();
      if (label >= this->size()) {
        this->push_back(element);
      } else {
        this->set(label, element);
      }
      return label;
    }

    bool label_remove(typename Vector<std::optional<T>>::Size label) {
      if (label < this->size() && this->operator[](label) != std::nullopt) {
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
};