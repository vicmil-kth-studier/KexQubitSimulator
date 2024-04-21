#define USE_DEBUG
#define DEBUG_KEYWORDS "" 

#include "../../std_lib_other.h"

int main() {
    // Lets create a basic interface to show that everything is working!
    Print("hello world!");

    Print("Create layout element manager");
    vicmil::LayoutElementManager manager = vicmil::LayoutElementManager();

    Print("Set position");
    manager.set_position(vicmil::RectT<int>(0, 0, 2000, 2000));

    Print("Get window layout");
    vicmil::LayoutElement window_layout = manager.get_window_element();
    vicmil::__layout_implementation__::_LayoutElement::set_parts_count(window_layout._ptr, 2);
    auto new_item = window_layout._ptr.push(vicmil::__layout_implementation__::_LayoutElement());
    Assert(!new_item.expired());

    Assert(window_layout._ptr.operator*().parts.size() == 2);
    Assert(!window_layout._ptr.operator*().parts[0].expired());

    // Lets try to split the window in two!
    Print("Split window in two");
    //window_layout.parts_divide_by_proportion({0.5, 0.5}, false, false);

    // Lets get the new elements position
    Print("Get left part");
    vicmil::LayoutElement left = window_layout.get_part(0);
    Assert(!left.is_deleted());
    Print("Get right part");
    vicmil::LayoutElement right = window_layout.get_part(1);
    Assert(!right.is_deleted());

    PrintExpr(left.get_position().to_string());
    PrintExpr(right.get_position().to_string());

    // Let us split the left part in two!
    left.parts_divide_by_proportion({0.3, 0.7}, true, false);
    vicmil::LayoutElement left_top = left.get_part(0);
    vicmil::LayoutElement left_bottom = left.get_part(1);

    PrintExpr(left_top.get_position().to_string());
    PrintExpr(left_bottom.get_position().to_string());

    // Lets enable widgets and see what happens when we simulate a click with the mouse
    right.enable_widget();
    left_top.enable_widget();
    left_bottom.enable_widget();

    manager.update(true, 10, 10);

    PrintExpr(manager.get_last_clicked_element().get_position().to_string());
    PrintExpr(left_top.widget_is_clicked());
    PrintExpr(left_bottom.widget_is_clicked());


    // Lets change the position of the entire window, and see what happens with the position of the parts
    manager.set_position(vicmil::RectT<int>(0, 0, 100, 100));
    PrintExpr(left_top.get_position().to_string());
    PrintExpr(left_bottom.get_position().to_string());
}