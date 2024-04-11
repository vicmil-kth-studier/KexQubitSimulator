#define USE_DEBUG
#define DEBUG_KEYWORDS "!vicmil_lib,/source,main(),init()" 
#include "../../source/quantum_computer_include.h"

class MainWindow {
public:
    vicmil::__layout__::WindowLayout layout = vicmil::__layout__::WindowLayout();
    vicmil::__layout__::WindowLayoutElement entire_window;
    vicmil::__layout__::WindowLayoutElement button_element;
    MainWindow() {
        // Create a button in the middle of the window
        layout.set_size(1000, 1000);
        entire_window = layout.get_window_element();
        entire_window.set_vertical_split();
        entire_window.create_child_element();
        auto temp = entire_window.create_child_element();
        entire_window.create_child_element();

        temp.set_horizontal_split();
        temp.create_child_element();
        button_element = temp.create_child_element();
        temp.create_child_element();
        entire_window.update();
    }
    std::vector<vicmil::general_gpu_setup::Triangle> visualize_layout() {
        std::vector<vicmil::general_gpu_setup::Triangle> triangles;
        vicmil::vec_extend(triangles, vicmil::visualize_layout_element(entire_window, 0.1, 10));
        return triangles;
    }
};