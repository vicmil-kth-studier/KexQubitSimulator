#include "L1_user_interface_implementation.h"

namespace vicmil {


class LayoutElement {
public:
pool_ptr<vicmil::__layout_implementation__::_LayoutElement> _ptr;

// Parts
void parts_align(std::vector<vicmil::RectAlignment> alignment, std::vector<int> width, std::vector<int> height) {
    Assert(alignment.size() == width.size());
    Assert(alignment.size() == height.size());
    // Ensure that there is the correct number of parts
    _ptr.operator*().set_parts_count(_ptr, alignment.size());
    
    // Create aligner
    std::shared_ptr<vicmil::__layout_implementation__::_PartsAlign> element_mover = 
        std::make_shared<vicmil::__layout_implementation__::_PartsAlign>();
    element_mover->alignment = alignment;
    element_mover->width = width;
    element_mover->height = height;
    _ptr.operator*().element_mover = element_mover;

    // Update the position of parts
    _ptr.operator*().element_mover->update(_ptr.operator*());
}
void parts_divide_by_size(std::vector<int> width, std::vector<int> height, bool vertical = false, bool reversed = false) {
    Assert(width.size() == height.size());
    // Ensure that there is the correct number of parts
    _ptr.operator*().set_parts_count(_ptr, width.size());
    
    // Create aligner
    std::shared_ptr<vicmil::__layout_implementation__::_PartsDivideBySize> element_mover = 
        std::make_shared<vicmil::__layout_implementation__::_PartsDivideBySize>();
    element_mover->width = width;
    element_mover->height = height;
    element_mover->vertical = vertical;
    element_mover->reversed = reversed;

    _ptr.operator*().element_mover = element_mover;

    // Update the position of parts
    _ptr.operator*().element_mover->update(_ptr.operator*());
}
void parts_divide_by_proportion(std::vector<double> proportion, bool vertical = false, bool reversed = false) {
    // Ensure that there is the correct number of parts
    Print("Set parts count");
    _ptr.operator*().set_parts_count(_ptr, proportion.size());
    
    // Create aligner
    Print("create aligner");
    std::shared_ptr<vicmil::__layout_implementation__::_PartsDivideByProportion> element_mover = 
        std::make_shared<vicmil::__layout_implementation__::_PartsDivideByProportion>();
    Print("Setup aligner");
    element_mover->proportion = proportion;
    element_mover->vertical = vertical;
    element_mover->reversed = reversed;
    _ptr.operator*().element_mover = element_mover;

    // Update the position of parts
    Print("Update");
    Assert(!_ptr.expired());
    _ptr.operator*().element_mover->update(_ptr.operator*());
}
void parts_delete_all() {
    _ptr.operator*().delete_all_parts();
}
LayoutElement get_part(unsigned int index) {
    LayoutElement return_elem;
    Assert(!_ptr.expired());
    Assert(_ptr.operator*().parts.size() > index);
    Print("Set return element pointer");
    PrintExpr(_ptr.operator*().parts.size());
    return_elem._ptr = _ptr.operator*().parts[index]; 
    Assert(!return_elem.is_deleted());
    PrintExpr("get_part exit");
    return return_elem;
}
LayoutElement extract_part(int index) {
    LayoutElement return_elem;
    return_elem._ptr = _ptr.operator*().parts[index]; 
    return_elem._ptr.operator*().orphan = true;
    vicmil::vec_remove(_ptr.operator*().parts, index);
    return return_elem;
}
void insert_part(int index, LayoutElement part) {
    Assert(part._ptr.operator*().orphan == true);
    vicmil::vec_insert(_ptr.operator*().parts, index, part._ptr);
    part._ptr.operator*().orphan = false;
}
int part_count() {return _ptr.operator*().parts.size(); }

// Widget
void enable_widget(bool enable=true, double depth=0) {
    _ptr.operator*().widget_enabled = enable;
    _ptr.operator*().widget_depth = depth;
}
bool widget_selected() { return _ptr.operator*().widget_is_selected; }
bool widget_is_clicked() { return _ptr.operator*().widget_is_clicked; }
bool widget_is_last_clicked() { return _ptr.operator*().widget_is_last_clicked; }

// Position
void set_position(vicmil::RectT<int> position) { _ptr.operator*().position = position; }
vicmil::RectT<int> get_position() { return _ptr.operator*().position; }

// Info
void set_info(std::string info) {_ptr.operator*().info = info; }
std::string get_info() {return _ptr.operator*().info; }

// Other
bool is_deleted() {return _ptr.expired();}
void set_alert(std::unique_ptr<LayoutElementAlert> alert) {_ptr.operator*().alert.reset(alert.release());}
};

class LayoutElementManager {
public:
std::shared_ptr<vicmil::__layout_implementation__::_LayoutElementManager> _ptr = 
    std::make_shared<vicmil::__layout_implementation__::_LayoutElementManager>();
void update(bool mouse_clicked, int mouse_x, int mouse_y) {_ptr->update(mouse_clicked, mouse_x, mouse_y); }
void set_position(vicmil::RectT<int> position) {_ptr->set_position(position); }
LayoutElement get_last_clicked_element() {
    LayoutElement return_elem;
    return_elem._ptr = _ptr->last_clicked;
    return return_elem;
}
LayoutElement get_last_selected_element() {
    LayoutElement return_elem;
    return_elem._ptr = _ptr->last_selected;
    return return_elem;
}
LayoutElement get_window_element() {
    LayoutElement return_elem;
    return_elem._ptr = _ptr->_window_element;
    return return_elem;
}
};
}