#include "reference_frame.h"
#include "core/object/class_db.h"

void ReferenceFrame::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_success"), &ReferenceFrame::get_success);
	
	ClassDB::bind_method(D_METHOD("set_tolerance", "tolerance"), &ReferenceFrame::set_tolerance);
	ClassDB::bind_method(D_METHOD("get_tolerance"), &ReferenceFrame::get_tolerance);
	
	ClassDB::bind_method(D_METHOD("set_total_error", "total_error"), &ReferenceFrame::set_total_error);
	ClassDB::bind_method(D_METHOD("get_total_error"), &ReferenceFrame::get_total_error);
	
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "success"), "", "get_success");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "tolerance"), "set_tolerance", "get_tolerance");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "total_error"), "set_total_error", "get_total_error");
}
