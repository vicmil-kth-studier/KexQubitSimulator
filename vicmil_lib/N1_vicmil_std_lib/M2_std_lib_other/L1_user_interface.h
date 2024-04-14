#include "../M1_std_lib_general/std_lib_general.h"

namespace vicmil {
struct _Widget {
    bool is_selected = false;
    bool is_last_clicked_widget = false;
    bool was_clicked = false;
    vicmil::RectT<int> position = vicmil::RectT<int>(0, 0, 0, 0);
    float depth = 0;
};
/**
 * A widget is an element on the screen in the form of a rectangle
 *  it is important to keep track of which widgets are on the screen,
 *  and in case widgets are stacked, which widget the mouse is pointing at
 * 
 *  This implementation aims to solve that problem
 * 
 * The WidgetManager is the one that keeps track of all the widgets
 * it can create new widgets. When update is called, it determines
 * which widget is the last clicked, and if any widgets are clicked 
*/
struct Widget {
    vicmil::list_ptr<_Widget> _ptr;
    Widget() {}
    Widget(vicmil::list_ptr<_Widget> ptr_) : _ptr(ptr_) {}
    _Widget& lock() {
        return _ptr.lock();
    }
    bool expired() {
        return _ptr.expired();
    }
    void remove() {
        _ptr.remove();
    }
    vicmil::RectT<int> get_position() {
        if(!expired()) {
            return lock().position;
        }
        return vicmil::RectT<int>(0, 0, 0, 0);
    }
    bool is_selected() {
        if(!expired()) {
            return lock().is_selected;
        }
        return false;
    }
    bool is_last_clicked_widget() {
        if(!expired()) {
            return lock().is_last_clicked_widget;
        }
        return false;
    }
    bool was_clicked() {
        if(!expired()) {
            return lock().was_clicked;
        }
        return false;
    }
};
struct WidgetManager {
    vicmil::ObjectList<_Widget> widgets;
    Widget create_widget() {
        Widget new_;
        new_._ptr = widgets.push_back(_Widget());
        return new_;
    }
    struct MouseInput {
        int x = 0;
        int y = 0;
        bool was_clicked = false;
        MouseInput() {}
        MouseInput(int x_, int y_, bool was_clicked_) : x(x_), y(y_), was_clicked(was_clicked_) {}
    };
    void _reset_widget_states(MouseInput mouse_input) {
        auto it = widgets.begin();
        while(it != widgets.end()) {
            _Widget& widget = it.lock();
            widget.is_selected = false;
            widget.was_clicked = false;
            if(mouse_input.was_clicked) {
                widget.is_last_clicked_widget = false;
            }
            it.operator++();
        }
    }
    void _make_selected_widget(_Widget& widget, MouseInput mouse_input) {
        widget.is_selected = true;
        if(mouse_input.was_clicked) {
            widget.is_last_clicked_widget = true;
            widget.was_clicked = true;
        }
    }
    // Returns an expired weak ptr if it fails
    void _update_selected_widget(MouseInput mouse_input) {
        _Widget* selected_widget = nullptr;
        auto it = widgets.begin();
        while(it != widgets.end()) {
            _Widget& widget = it.lock();
            if(widget.position.is_inside_rect(mouse_input.x, mouse_input.y)) {
                if(selected_widget==nullptr || widget.depth < selected_widget->depth) {
                    selected_widget = &it.lock();
                }
            }
            it.operator++();
        }
        if(selected_widget != nullptr) {
            _make_selected_widget(*selected_widget, mouse_input);
        }
    }
    int update(WidgetManager::MouseInput mouse_input) {
        _reset_widget_states(mouse_input);
        _update_selected_widget(mouse_input);
        return 0;
    }
    int update(int mouse_x, int mouse_y, bool mouse_clicked) {
        return update(WidgetManager::MouseInput(mouse_x, mouse_y, mouse_clicked));
    }
};





/**
 * Layout rectangles are a way to divide the screen, great for creating an user interface
*/
struct _LayoutRect {
    vicmil::RectT<int> position = vicmil::RectT<int>(0, 0, 0, 0);
    vicmil::list_ptr<_LayoutRect> entire_window = vicmil::list_ptr<_LayoutRect>();

    // [Optional] An element that will be automatically updated if the element is updated
    struct Update {
        virtual void update_screen_size(){}; // (Will automatically be done to all elements, no need to call others)
        virtual void update(){}; 
        virtual void remove(){}; 
    };
    std::shared_ptr<_LayoutRect::Update> update_element = std::make_shared<_LayoutRect::Update>(_LayoutRect::Update());
    ~_LayoutRect() {
        update_element->remove();
    }
};
struct LayoutRect {
    vicmil::list_ptr<_LayoutRect> _ptr = vicmil::list_ptr<_LayoutRect>();
    LayoutRect() {}
    LayoutRect(vicmil::list_ptr<_LayoutRect> ptr_) {
        START_TRACE_FUNCTION();
        _ptr = ptr_;
        END_TRACE_FUNCTION();
    }
    LayoutRect create_new() {
        vicmil::list_ptr<_LayoutRect> new_;
        if(!expired()) {
            new_ = _ptr.push_back(_LayoutRect());
            new_.lock().entire_window = _ptr.lock().entire_window;
            return new_;
        }
        return LayoutRect();
    }
    vicmil::RectT<int> get_position() {
        if(!expired()) {
            return _ptr.lock().position;
        }
        return vicmil::RectT<int>(0, 0, 0, 0);
    }
    void set_position(vicmil::RectT<int> pos) {
        if(!expired() && _ptr.lock().position != pos) {
            _ptr.lock().position = pos;
            _ptr.lock().update_element->update();
        }
    }
    LayoutRect entire_window() {
        if(!expired()) {
            return _ptr.lock().entire_window;
        }
        return vicmil::list_ptr<_LayoutRect>();
    }
    void set_update_element(std::shared_ptr<_LayoutRect::Update> update_) {
        if(!expired()) {
            _ptr.lock().update_element->remove();
            _ptr.lock().update_element = update_;
        }
    }
    _LayoutRect& lock() {
        return _ptr.lock();
    }
    bool expired() {
        return _ptr.expired();
    }
    void remove() {
        _ptr.remove();
    }
};
struct LayoutRectManager {
    vicmil::ObjectList<_LayoutRect> _layout_rectangles = vicmil::ObjectList<_LayoutRect>();
    LayoutRect entire_screen_rect = LayoutRect();
    LayoutRectManager() {
        START_TRACE_FUNCTION();
        vicmil::_LayoutRect new_rect = vicmil::_LayoutRect();
        Debug("pass rect");
        vicmil::list_ptr<vicmil::_LayoutRect> layout_rects = _layout_rectangles.push_back(new_rect);
        Debug("set entire_screen_rect");
        entire_screen_rect = LayoutRect(layout_rects);
        Debug("Set entire screen rect entire window");
        entire_screen_rect.lock().entire_window = entire_screen_rect._ptr;
        END_TRACE_FUNCTION();
    }
    void set_screen_size(int w, int h) {
        if(entire_screen_rect.get_position() != vicmil::RectT<int>(0, 0, w, h)) {
            entire_screen_rect.set_position(vicmil::RectT<int>(0, 0, w, h));

            auto it = _layout_rectangles.begin();
            while(it != _layout_rectangles.end()) {
                it.lock().update_element->update_screen_size();
                it.operator++();
            }
        }
    }
};
void TEST_LayoutRectManager() {
    LayoutRectManager layout_manager = LayoutRectManager();
    //Assert(false);
}
AddTest(TEST_LayoutRectManager);

namespace __layout__ {
struct _WidgetRect: public _LayoutRect::Update {
    LayoutRect layout = LayoutRect();
    Widget widget = Widget();
    void update() override {
        widget.lock().position = layout.get_position();
    }
    void remove() override {
        layout.remove();
        widget.remove();
    }
    ~_WidgetRect() {
        remove();
    }
};
struct WidgetRect {
    std::shared_ptr<_WidgetRect> _ptr = std::make_shared<_WidgetRect>();
    WidgetRect() {}
    WidgetRect(WidgetManager& widget_manager, LayoutRect rect) {
        _ptr->widget = widget_manager.create_widget();
        _ptr->layout = rect;
        _ptr->layout.set_update_element(_ptr);
        _ptr->update();
    }
    bool was_clicked() {
        return _ptr->widget.lock().was_clicked;
    }
    bool is_selected() {
        return _ptr->widget.lock().is_selected;
    }
    bool is_last_clicked_widget() {
        return _ptr->widget.lock().is_last_clicked_widget;
    }
    Widget widget() {
        return _ptr->widget;
    }
};


struct _PropSplitElement {
    LayoutRect layout;
    float prop;
};
struct _PropSplit: public _LayoutRect::Update {
    LayoutRect layout;
    std::vector<_PropSplitElement> elements = {};
    bool split_horizontal;
    void update() override {
        const RectT<int> layout_pos = layout.get_position();
        RectT<int> pos = layout_pos;
        for(int i = 0; i < elements.size(); i++) {
            if(split_horizontal) {
                pos.w = elements[i].prop * layout_pos.w;
                vicmil::cut_rect_to_fit(pos, layout_pos);
                elements[i].layout.set_position(pos);
                pos.x += pos.w;
            }
            else {
                pos.h = elements[i].prop * layout_pos.h;
                vicmil::cut_rect_to_fit(pos, layout_pos);
                elements[i].layout.set_position(pos);
                pos.y += pos.h;
            }
        }
    }
    void remove() override {
        for(int i = 0; i < elements.size(); i++) {
            elements[i].layout.remove();
        }
    }
    ~_PropSplit() {
        remove();
    }
};
struct PropSplit {
    std::shared_ptr<_PropSplit> _ptr = std::make_shared<_PropSplit>();
    PropSplit() {}
    PropSplit(LayoutRect rect) {
        _ptr->layout = rect;
        _ptr->layout.set_update_element(_ptr);
    }
    LayoutRect push_back(float prop) {
        _ptr->elements.push_back(_PropSplitElement());
        _ptr->elements.back().prop = prop;
        _ptr->elements.back().layout = _ptr->layout.create_new();
        update();
        return _ptr->elements.back().layout;
    }
    void set_split_horizontal() {
        _ptr->split_horizontal = true;
        update();
    }
    void set_split_vertical() {
        _ptr->split_horizontal = false;
        update();
    }
    void update() {
        _ptr->update();
    }
};


struct _SizeSplitElement {
    LayoutRect layout;
    int height = 0;
    int width = 0;
};
struct _SizeSplit: public _LayoutRect::Update {
    LayoutRect layout;
    std::vector<_SizeSplitElement> elements = {};
    bool split_horizontal;
    void update() override {
        const RectT<int> layout_pos = layout.get_position();
        RectT<int> pos = layout_pos;
        for(int i = 0; i < elements.size(); i++) {
            pos.w = elements[i].width;
            pos.h = elements[i].height;
            if(split_horizontal) {
                vicmil::cut_rect_to_fit(pos, layout_pos);
                elements[i].layout.set_position(pos);
                pos.x += pos.w;
            }
            else {
                vicmil::cut_rect_to_fit(pos, layout_pos);
                elements[i].layout.set_position(pos);
                pos.y += pos.h;
            }
        }
    }
    void remove() override {
        for(int i = 0; i < elements.size(); i++) {
            elements[i].layout.remove();
        }
    }
    ~_SizeSplit() {
        remove();
    }
};
struct SizeSplit {
    std::shared_ptr<_SizeSplit> _ptr = std::make_shared<_SizeSplit>();
    SizeSplit() {}
    SizeSplit(LayoutRect rect) {
        _ptr->layout = rect;
        _ptr->layout.set_update_element(_ptr);
    }
    LayoutRect push_back(int width, int height) {
        _ptr->elements.push_back(_SizeSplitElement());
        _ptr->elements.back().width = width;
        _ptr->elements.back().height = height;
        _ptr->elements.back().layout = _ptr->layout.create_new();
        update();
        return _ptr->elements.back().layout;
    }
    void set_split_horizontal() {
        _ptr->split_horizontal = true;
        update();
    }
    void set_split_vertical() {
        _ptr->split_horizontal = false;
        update();
    }
    void update() {
        _ptr->update();
    }
};


/**
 * Align a rectangle according to other rectangle
*/
struct _AlignRect: public _LayoutRect::Update {
    LayoutRect layout = LayoutRect(); // The layout that should not be modified
    LayoutRect aligned_layout = LayoutRect(); // The layout that should be modified
    vicmil::RectAlignment alignment = vicmil::RectAlignment::TOP_align_LEFT;
    int width = 0;
    int height = 0;
    _AlignRect() {}
    void update() override {
        RectT<int> new_pos = aligned_layout.get_position();
        new_pos.w = width;
        new_pos.h = height;
        vicmil::align_rect(new_pos, layout.get_position(), alignment);
        vicmil::cut_rect_to_fit(new_pos, layout.entire_window().get_position());
        aligned_layout.set_position(new_pos);
    }
    void update_screen_size() override {
        update();
    }
    void remove() override {
        aligned_layout.remove();
    }
    ~_AlignRect() {
        remove();
    }
};
struct AlignRect {
    std::shared_ptr<_AlignRect> _ptr = std::make_shared<_AlignRect>();
    AlignRect() {}
    AlignRect(LayoutRect layout, vicmil::RectAlignment alignment) {
        _ptr->layout = layout;
        _ptr->layout.set_update_element(_ptr);
        _ptr->aligned_layout = _ptr->layout.create_new();
        _ptr->alignment = alignment;
    }
    void set_size(int width, int height) {
        if(!_ptr->aligned_layout.expired()) {
            _ptr->width = width;
            _ptr->height = height;
            _ptr->update();
        }
    }
    void set_alignment(vicmil::RectAlignment alignment) {
        _ptr->alignment = alignment;
    }
    LayoutRect get_aligned_layout() {
        return _ptr->aligned_layout;
    }
    void update() {
        _ptr->update();
    }
};

/**
 * Create multiple copies of LayoutRect that will copy the original layouts position
 * Great for having multiple update elements on a single layout element
*/
struct _Copy: public _LayoutRect::Update {
    LayoutRect layout = LayoutRect(); // The layout that should not be modified
    std::vector<LayoutRect> copies = {};
    void update() override {
        const RectT<int> layout_pos = layout.get_position();
        for(int i = 0; i < copies.size(); i++) {
            copies[i].set_position(layout_pos);
        }
    }
    void remove() override {
        for(int i = 0; i < copies.size(); i++) {
            copies[i].remove();
        }
    }
    ~_Copy() {
        remove();
    }
};
struct Copy {
    std::shared_ptr<_Copy> _ptr = std::make_shared<_Copy>();
    Copy() {}
    Copy(LayoutRect layout) {
        _ptr->layout = layout;
        _ptr->layout.set_update_element(_ptr);
    }
    LayoutRect create_copy() {
        _ptr->copies.push_back(_ptr->layout.create_new());
        update();
        return _ptr->copies.back();
    }
    void update() {
        _ptr->update();
    }
};
}
}