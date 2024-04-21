#include "../M1_std_lib_general/std_lib_general.h"

namespace vicmil {
class LayoutElementAlert {
public:
    virtual void clicked() {}
    virtual void not_last_clicked() {}
    virtual void selected() {}
    virtual void deselected() {}
    virtual void deleted() {}
    virtual void moved() {}
};

namespace __layout_implementation__ {
struct _LayoutElement;

struct _LayoutElementMover {
    virtual void update(_LayoutElement& parent) {}
    //virtual void resized_window(_LayoutElement& parent){};
};

struct _LayoutElement {
    std::string info = ""; // TODO: use template to have info be anything!
    std::shared_ptr<LayoutElementAlert> alert;
    std::shared_ptr<_LayoutElementMover> element_mover;
    std::vector<pool_ptr<_LayoutElement>> parts = {};
    RectT<int> position = RectT<int>(0, 0, 0, 0);
    bool orphan = false; // If it does have any parents

    bool widget_enabled = false;
    double widget_depth = 0.5;

    bool widget_is_last_clicked = false;
    bool widget_is_clicked = false;
    bool widget_is_selected = false;
    void set_position(RectT<int> new_position) {
        if(position != new_position) {
            position = new_position;
            alert->moved();
        }
    }
    void delete_all_parts() {
        for(int i = 0; i < parts.size(); i++) {
            parts[i].erase();
        }
        parts = {};
    }
    _LayoutElement() {
        alert.reset(new LayoutElementAlert());
        element_mover.reset(new _LayoutElementMover());
    }
    virtual ~_LayoutElement() {
        alert->deleted();
        delete_all_parts();
    }
    static void set_parts_count(pool_ptr<_LayoutElement> elem, int count) {
        PrintExpr(elem.key);
        PrintExpr(elem.index);
        if(!elem.expired()) {
            PrintExpr(count);
            PrintExpr(elem.operator*().parts.size());

            // If there are too many parts, reduce the number of parts
            while(elem.operator*().parts.size() > count) {
                PrintExpr(count);
                PrintExpr(elem.operator*().parts.size());
                elem.operator*().parts.back().erase();
                elem.operator*().parts.pop_back();
            }

            // If there are too few parts, create more parts
            while(elem.operator*().parts.size() < count) {
                PrintExpr(count);
                PrintExpr(elem.operator*().parts.size());
                {
                    vicmil::pool_ptr<vicmil::__layout_implementation__::_LayoutElement> new_elem = elem.push(_LayoutElement());
                    Assert(!new_elem.expired());
                    elem.operator*().parts.push_back(new_elem);
                }
                Assert(!elem.operator*().parts.back().expired());
                Assert(!elem.operator*().parts[0].expired());
            }
            Assert(!elem.operator*().parts[0].expired());
            PrintExpr(count);
            PrintExpr(elem.operator*().parts.size());
            Assert(elem.operator*().parts.size() == count);
        }
        
    }
};

struct _LayoutElementManager {
    PoolAllocator<_LayoutElement> _layout_elements;
    pool_ptr<vicmil::__layout_implementation__::_LayoutElement> _window_element = pool_ptr<vicmil::__layout_implementation__::_LayoutElement>();

    // Widget
    pool_ptr<vicmil::__layout_implementation__::_LayoutElement> last_selected = pool_ptr<vicmil::__layout_implementation__::_LayoutElement>();
    pool_ptr<vicmil::__layout_implementation__::_LayoutElement> last_clicked = pool_ptr<vicmil::__layout_implementation__::_LayoutElement>();
    void update(bool mouse_clicked, int mouse_x, int mouse_y) {
        auto top_element_iter = _layout_elements.end();

        // Iterate through elements and find the one that is on top
        auto iter = _layout_elements.begin();
        while(iter != _layout_elements.end()) {
            if(iter.operator*().position.is_inside_rect(mouse_x, mouse_y)) {
                // Reset state
                iter.operator*().widget_is_clicked = false;
                iter.operator*().widget_is_selected = false;
                if(mouse_clicked) {
                    iter.operator*().widget_is_last_clicked = false;
                }
                
                // Determine if it is selected
                if(top_element_iter == _layout_elements.end()) {
                    // Make top element!
                    top_element_iter = iter;
                }
                else if(top_element_iter.operator*().widget_depth > iter.operator*().widget_depth) {
                    // Make top element!
                    top_element_iter = iter;
                }
                else if(top_element_iter.operator*().widget_depth == iter.operator*().widget_depth) {
                    // Let the more recently allocated objects be on top in case of conflict
                    if(_layout_elements.get_allocation_nr(top_element_iter._index) < _layout_elements.get_allocation_nr(iter._index)) {
                        top_element_iter = iter;
                    }
                }
            }
            iter.operator++();
        }

        pool_ptr<vicmil::__layout_implementation__::_LayoutElement> top_element;
        if(top_element_iter != _layout_elements.end()) {
            top_element.index = top_element_iter._index;
            top_element.pool_allocator = top_element_iter._allocator;
            top_element.key = _layout_elements.get_allocation_nr(top_element_iter._index);
        }

        // Update old last selected
        if(last_selected != top_element && !last_selected.expired()) {
            last_selected.operator*().alert->deselected();
        }
        if(mouse_clicked && last_clicked != top_element && !last_clicked.expired()) {
            last_clicked.operator*().alert->not_last_clicked();
        }

        // Assign new last selected
        if(!top_element.expired()) {
            last_selected = top_element;
            last_selected.operator*().alert->selected();
            last_selected.operator*().widget_is_selected = true;
            if(mouse_clicked) {
                last_clicked = top_element;
                last_clicked.operator*().alert->clicked();
                last_clicked.operator*().widget_is_clicked = true;
                last_clicked.operator*().widget_is_last_clicked = true;
            }
        }
    }
    void set_position(vicmil::RectT<int> position) {
        _window_element.operator*().set_position(position);
    }
    _LayoutElementManager() {
        // Create window element
        _window_element = _layout_elements.push(_LayoutElement());
        set_position(RectT<int>(0, 0, 1000, 1000));
    }
};

struct _PartsAlign: _LayoutElementMover {
    std::vector<vicmil::RectAlignment> alignment;
    std::vector<int> width;
    std::vector<int> height;
    void update(_LayoutElement& parent) override {
        Print("_PartsAlign update");
        // Iterate through parts and try to align them
        for(int i = 0; i < parent.parts.size(); i++) {
            if(i >= width.size()) {
                return;
            }
            if(alignment[i] != vicmil::RectAlignment::NO_ALIGNMENT) {
                RectT<int> position = parent.parts[i].operator*().position;
                position.w = width[i];
                position.h = height[i];
                vicmil::align_rect(position, parent.position, alignment[i]);
                parent.parts[i].operator*().set_position(position);
            }
        }
    }
};
struct _PartsDivideBySize: _LayoutElementMover {
    std::vector<int> width;
    std::vector<int> height;
    bool vertical;
    bool reversed;
    void update(_LayoutElement& parent) override {
        Print("_PartsDivideBySize update");
        vicmil::RectT<int> position = parent.position;
        int max_iter = std::min(parent.parts.size(), width.size());
        // Iterate through parts and try to fit them according to size
        for(int i2 = 0; i2 < max_iter; i2++) {
            int i = i2;
            if(!reversed) {
                position.w = width[i];
                position.h = height[i];
                vicmil::cut_rect_to_fit(position, parent.position);
                parent.parts[i].operator*().set_position(position);
                if(vertical) {
                    position.y += height[i];
                }
                else {
                    position.x += width[i];
                }
            }
            else {
                i = max_iter - i2 - 1;
                if(vertical) {
                    position.x = parent.position.max_x() - width[i];
                    position.y = position.max_y() - height[i];
                }
                else {
                    position.x = position.max_x() - width[i];
                    position.y = parent.position.max_y() - height[i];
                }
                position.w = width[i];
                position.h = height[i];
                vicmil::cut_rect_to_fit(position, parent.position);
                parent.parts[i].operator*().set_position(position);
            }
        }
    }
};
struct _PartsDivideByProportion: _LayoutElementMover {
    std::vector<double> proportion;
    bool vertical;
    bool reversed;
    void update(_LayoutElement& parent) override {
        Print("_PartsDivideByProportion update");
        vicmil::RectT<int> position = parent.position;
        int max_iter = std::min(parent.parts.size(), proportion.size());
        PrintExpr(max_iter);
        // Iterate through parts and try to fit them according to size
        for(int i2 = 0; i2 < max_iter; i2++) {
            int i = i2;
            if(reversed) {
                i = max_iter - i2 - 1;
            }
            if(parent.parts[i].expired()) {
                continue;
            }
            PrintExpr(i);
            int width = parent.position.w;
            int height = parent.position.h;
            if(vertical) {
                height = parent.position.h * proportion[i];
            }
            else {
                width = parent.position.w * proportion[i];
            }
            if(!reversed) {
                position.w = width;
                position.h = height;
                vicmil::cut_rect_to_fit(position, parent.position);
                Print("set position");
                parent.parts[i].operator*().set_position(position);
                if(vertical) {
                    position.y += height;
                }
                else {
                    position.x += width;
                }
            }
            else {
                
                if(vertical) {
                    position.x = parent.position.max_x() - width;
                    position.y = position.max_y() - height;
                }
                else {
                    position.x = position.max_x() - width;
                    position.y = parent.position.max_y() - height;
                }
                position.w = width;
                position.h = height;
                vicmil::cut_rect_to_fit(position, parent.position);
                Print("set position");
                parent.parts[i].operator*().set_position(position);
            }
            Print("next iteration!");
        }
        Print("_PartsDivideByProportion update exit");
    }
};
}
}