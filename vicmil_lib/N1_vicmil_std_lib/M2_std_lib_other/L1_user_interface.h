#include "../M1_std_lib_general/std_lib_general.h"

/**
 * The things to pay attention to here is (The rest is mostly just sugar that is needed for the implementation to work)
 * - WidgetManager
 * - LayoutRectManager
 * - LayoutRectWidget
 * 
 * At the bottom you can also find some util widgets for more advanced stuff such as
 * - Displaying a dropdown menu system
 * - Displaying and interacting with a graph
*/

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
    bool expired() { // Get if the widget has been removed
        return _ptr.expired();
    }
    void remove() { // Remove widget
        _ptr.remove();
    }
    vicmil::RectT<int> get_position() { // Get the position of the widget
        if(!expired()) {
            return lock().position;
        }
        return vicmil::RectT<int>(0, 0, 0, 0);
    }
    void set_position(vicmil::RectT<int> new_position) { // Set the position of the widget
        if(!expired()) {
            lock().position = new_position;
        }
    }
    void set_depth(double depth) { // Set the depth(higher depths means it will be placed under widgets with lower depth)
        if(!expired()) {
            lock().depth = depth;
        }
    }
    bool is_selected() { // Determine if the mouse is pointing at the widget(And there are no widgets stacked ontop)
        if(!expired()) {
            return lock().is_selected;
        }
        return false;
    }
    bool is_last_clicked_widget() { // Determine if this widget is the last widget clicked on by the mouse
        if(!expired()) {
            return lock().is_last_clicked_widget;
        }
        return false;
    }
    bool was_clicked() { // Determine if the widget was clicked in the last mouse update
        if(!expired()) {
            return lock().was_clicked;
        }
        return false;
    }
    Widget create_widget() { // Create a new widget, also belonging to the same widget manager
        return Widget(_ptr);
    }
};
struct WidgetManager {
    vicmil::ObjectList<_Widget> widgets; // List of widgets(stored in a way so they are automatically popped if deallocated)
    Widget create_widget() { // Create a new widget
        Widget new_;
        new_._ptr = widgets.push_back(_Widget());
        return new_;
    }
    struct MouseInput { // Mouse input class, populated by user with library of choice!
        int x = 0;
        int y = 0;
        bool was_clicked = false;
        MouseInput() {}
        MouseInput(int x_, int y_, bool was_clicked_) : x(x_), y(y_), was_clicked(was_clicked_) {}
    };
    void _reset_widget_states(MouseInput mouse_input) { // Reset all widget states(so that none are clicked)
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
    void _make_selected_widget(_Widget& widget, MouseInput mouse_input) { // Make a widget the selected widget
        widget.is_selected = true;
        if(mouse_input.was_clicked) {
            widget.is_last_clicked_widget = true;
            widget.was_clicked = true;
        }
    }
    void _update_selected_widget(MouseInput mouse_input) { // Make the appropriated widget selected
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
    int update(WidgetManager::MouseInput mouse_input) { // Make the appropriate widget selected
        _reset_widget_states(mouse_input);
        _update_selected_widget(mouse_input);
        return 0;
    }
    int update(int mouse_x, int mouse_y, bool mouse_clicked) { // Make the appropriate widget selected
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
/**
 * The LayoutRectManager is responsible for storing all the layout rectangles
 * It also updates the rectangles accordingly if the screen is resized
*/
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
/**
 * Create multiple copies of LayoutRect that will sync with the original layouts position
 * Great for having multiple update elements on a single layout element
*/
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
/**
 * Automatically update a widgets position as the LayoutRect changes position
*/
struct WidgetRect {
    std::shared_ptr<_WidgetRect> _ptr = std::make_shared<_WidgetRect>();
    WidgetRect() {}
    WidgetRect(WidgetManager& widget_manager, LayoutRect rect) {
        _ptr->widget = widget_manager.create_widget();
        _ptr->layout = rect;
        _ptr->layout.set_update_element(_ptr);
        _ptr->update();
    }
    WidgetRect(Widget& parent_widget, LayoutRect rect) {
        _ptr->widget = parent_widget.create_widget();
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
/**
 * Split the screen based on proportion
 * You can for example create sub-elements that will take up 10% of the original LayoutRect
*/
struct PropSplit {
    std::shared_ptr<_PropSplit> _ptr = std::make_shared<_PropSplit>();
    PropSplit() {}
    PropSplit(LayoutRect rect) {
        _ptr->layout = rect;
        _ptr->layout.set_update_element(_ptr);
    }
    LayoutRect push_back(float prop) { // Add a new proportion element
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
/**
 * Split the screen based on size
 * You can for example create sub-elements that will take up 100 pixels each inside the original LayoutRect
*/
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
/**
 * Align a rectangle according to other rectangle
*/
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
}


struct _LayoutRectWidget {
    Widget _hidden_widget; // Used for creating new widgets
    Widget widget = Widget(); // Optional widget

    LayoutRect sync_layout; // The layout syncing with parent

    LayoutRect widget_layout; // The layout attached to the optional widget
    LayoutRect sub_layouts_layout; // The layout attached to the sublayouts
    void set_aligned_layout(LayoutRect new_aligned_layout) {
        auto copy_ = __layout__::Copy(new_aligned_layout);
        widget_layout = copy_.create_copy();
        sub_layouts_layout = copy_.create_copy();
    }
    _LayoutRectWidget(LayoutRect sync_layout_, Widget hidden_widget_) {
        sync_layout = sync_layout_;
        _hidden_widget = hidden_widget_;
        set_aligned_layout(sync_layout_);
    }
    _LayoutRectWidget() {}
    ~_LayoutRectWidget() {
        widget_layout.remove();
        sub_layouts_layout.remove();
        widget.remove();
    }
};
/**
 * This is the result of all features in __layout__ combined!
 * It can do a lot! No need to use lesser classes
*/
struct LayoutRectWidget {
    std::shared_ptr<std::vector<LayoutRectWidget>> sub_layouts = std::make_shared<std::vector<LayoutRectWidget>>();
    std::shared_ptr<_LayoutRectWidget> _ptr = std::make_shared<_LayoutRectWidget>();
    LayoutRectWidget() {}
    LayoutRectWidget(LayoutRect sync_layout_, Widget hidden_widget_) {
        (*_ptr) = _LayoutRectWidget(sync_layout_, hidden_widget_);
    }
    LayoutRectWidget(LayoutRect layout_, WidgetManager& widget_manager) {
        Widget hidden_widget = widget_manager.create_widget();
        hidden_widget.set_position(RectT<int>(0, 0, 0, 0));
        (*_ptr) = _LayoutRectWidget(layout_, hidden_widget);
    }
    void enable_widget(double depth = 0) {
        __layout__::WidgetRect widget_rect = __layout__::WidgetRect(_ptr->_hidden_widget, _ptr->widget_layout);
        _ptr->widget = widget_rect.widget();
        _ptr->widget.set_depth(depth);
    }
    void enable_sublayout_widgets() {
        for(int i = 0; i < sub_layouts->size(); i++) {
            (*sub_layouts)[i].enable_widget();
        }
    }
    void align(vicmil::RectAlignment alignment, int width, int height) {
        auto aligned_layout_ = __layout__::AlignRect(_ptr->sync_layout, alignment);
        aligned_layout_.set_size(width, height);
        _ptr->set_aligned_layout(aligned_layout_.get_aligned_layout());
    }
    void clear_sublayouts() {
        (*sub_layouts) = {};
    }
    void split_sizes_horizontal(std::vector<int> width, std::vector<int> height) {
        clear_sublayouts();
        __layout__::SizeSplit split_ = __layout__::SizeSplit(_ptr->sub_layouts_layout);
        split_.set_split_horizontal();
        for(int i = 0; i < width.size(); i++) {
            vicmil::LayoutRect new_layout = split_.push_back(width[i], height[i]);
            (*sub_layouts).push_back(LayoutRectWidget(new_layout, _ptr->_hidden_widget));
        }
    }
    void split_sizes_vertical(std::vector<int> width, std::vector<int> height) {
        clear_sublayouts();
        __layout__::SizeSplit split_ = __layout__::SizeSplit(_ptr->sub_layouts_layout);
        split_.set_split_vertical();
        for(int i = 0; i < width.size(); i++) {
            vicmil::LayoutRect new_layout = split_.push_back(width[i], height[i]);
            (*sub_layouts).push_back(LayoutRectWidget(new_layout, _ptr->_hidden_widget));
        }
    }
    void split_proportion_horizontal(std::vector<double> prop) {
        clear_sublayouts();
        __layout__::PropSplit split_ = __layout__::PropSplit(_ptr->sub_layouts_layout);
        split_.set_split_horizontal();
        for(int i = 0; i < prop.size(); i++) {
            vicmil::LayoutRect new_layout = split_.push_back(prop[i]);
            (*sub_layouts).push_back(LayoutRectWidget(new_layout, _ptr->_hidden_widget));
        }
    }
    void split_proportion_vertical(std::vector<double> prop) {
        clear_sublayouts();
        __layout__::PropSplit split_ = __layout__::PropSplit(_ptr->sub_layouts_layout);
        split_.set_split_vertical();
        for(int i = 0; i < prop.size(); i++) {
            vicmil::LayoutRect new_layout = split_.push_back(prop[i]);
            (*sub_layouts).push_back(LayoutRectWidget(new_layout, _ptr->_hidden_widget));
        }
    }
    void create_synced_copies(int count) {
        clear_sublayouts();
        __layout__::Copy copy_ = __layout__::Copy(_ptr->sub_layouts_layout);
        for(int i = 0; i < count; i++) {
            vicmil::LayoutRect new_layout = copy_.create_copy();
            (*sub_layouts).push_back(LayoutRectWidget(new_layout, _ptr->_hidden_widget));
        }
    }
    void set_position(RectT<int> position) { // TODO: make it not delete all sublayouts
        auto aligned_layout_ = __layout__::AlignRect(_ptr->sync_layout, vicmil::RectAlignment::NO_ALIGNMENT);
        aligned_layout_.set_size(position.w, position.h);
        aligned_layout_.get_aligned_layout().set_position(position);
        _ptr->set_aligned_layout(aligned_layout_.get_aligned_layout());
    }
    RectT<int> get_position() {return _ptr->sub_layouts_layout.get_position(); }
    bool is_selected() {return _ptr->widget.is_selected(); }
    bool was_clicked() {return _ptr->widget.was_clicked(); }
    bool is_last_clicked_widget() {return _ptr->widget.is_last_clicked_widget(); }
    LayoutRectWidget get_index(int index) {return (*sub_layouts)[index]; }
    int index_count() {return sub_layouts->size(); }
    std::vector<int> get_selected_subwidget() { // Get if any subwidgets have been selected
        for(int i = 0; i < index_count(); i++) {
            if (get_index(i).is_selected()) {
                return {i};
            }
        }
        for(int i = 0; i < index_count(); i++) {
            std::vector<int> subwidget_selection = get_index(i).get_selected_subwidget();
            if (subwidget_selection.size() > 0) {
                std::vector<int> return_vec = {i};
                vicmil::vec_extend(return_vec, subwidget_selection);
                return return_vec;
            }
        }
        return {};
    }
    std::vector<int> get_last_clicked_subwidget() { // Get if any subwidgets have been the last ones to be clicked
        for(int i = 0; i < index_count(); i++) {
            if (get_index(i).is_last_clicked_widget()) {
                return {i};
            }
        }
        for(int i = 0; i < index_count(); i++) {
            std::vector<int> subwidget_selection = get_index(i).get_last_clicked_subwidget();
            if (subwidget_selection.size() > 0) {
                std::vector<int> return_vec = {i};
                vicmil::vec_extend(return_vec, subwidget_selection);
                return return_vec;
            }
        }
        return {};
    }
};

/**
 * Here we put layout elements that build on __layout__ elements
*/
namespace __layout2__ {
struct Directory { // Used to show the dropdown structure of dropdown
    std::vector<Directory> subdirectories = {};
    Directory(int size) {
        subdirectories.resize(size);
    }
    Directory() {}
    int size() {
        return subdirectories.size();
    }
};

struct _DropDown {
    Directory _structure = Directory(); // How the options are organized
    LayoutRectWidget _layout_rect = LayoutRectWidget();

    void _make_selection(std::vector<int>& new_selection) {
        int width = 100;
        int height = 10;
        int count = _structure.size();
        _layout_rect.align(RectAlignment::BOTTOM_align_LEFT, width, count*height);
        _layout_rect.split_sizes_vertical(std::vector<int>(count, width), std::vector<int>(count, height));
        _layout_rect.enable_sublayout_widgets();

        Directory& structure_ref = _structure;
        LayoutRectWidget& layout_rect_ref = _layout_rect;
        for(int i = 0; i < new_selection.size(); i++) {
            int index = new_selection[i];
            Assert(structure_ref.subdirectories.size() > index);
            Assert(layout_rect_ref.index_count() > index);
            structure_ref = structure_ref.subdirectories[index];
            layout_rect_ref = layout_rect_ref.get_index(index);

            layout_rect_ref.create_synced_copies(1);
            layout_rect_ref = layout_rect_ref.get_index(0); // To avoid option being aligned somewhere else

            int count = _structure.size();
            layout_rect_ref.align(RectAlignment::RIGHT_align_TOP, width, count*height);
            _layout_rect.split_sizes_vertical(std::vector<int>(count, width), std::vector<int>(count, height));
            _layout_rect.enable_sublayout_widgets();
        }
    }
    void make_selection(std::vector<int>& new_selection) {
        Directory& structure_ref = _structure;
        LayoutRectWidget& layout_rect_ref = _layout_rect;

        for(int i = 0; i < new_selection.size(); i++) {
            int index = new_selection[i];
            Assert(structure_ref.size() > index);
            if(structure_ref.size() != layout_rect_ref.index_count()) {
                // Need to update the structure to reflect the new selection!
                _make_selection(new_selection);
                return;
            }
            structure_ref = structure_ref.subdirectories[index];
            layout_rect_ref = layout_rect_ref.get_index(index);
        }
        if(layout_rect_ref.index_count() != 0) {
            layout_rect_ref.clear_sublayouts();
        }
    }
    std::vector<int> get_selected_by_mouse() {
        return _layout_rect.get_selected_subwidget();
    }
    std::vector<int> get_last_clicked_by_mouse() {
        return _layout_rect.get_last_clicked_subwidget();
    }
};
struct DropDown {
    std::shared_ptr<_DropDown> _ptr = std::make_shared<_DropDown>();

    DropDown() {}
    DropDown(LayoutRect layout, WidgetManager& widget_manager) {
        _ptr->_layout_rect = LayoutRectWidget(layout, widget_manager);
    }
    DropDown(LayoutRect layout, Widget hidden_widget) {
        _ptr->_layout_rect = LayoutRectWidget(layout, hidden_widget);
    }
    // The empty string is the parent option for everything
    void set_directory_size(std::vector<int> path, int new_size) {
        Directory& structure_ref = _ptr->_structure;
        for(int i = 0; i < path.size(); i++) {
            int index = path[i];
            if(structure_ref.size() <= i) {
                structure_ref.subdirectories.resize(i+1);
            }
            structure_ref = structure_ref.subdirectories[i];
        }
        structure_ref.subdirectories.resize(new_size);
    }
    void make_selection(std::vector<int> selection) {
        _ptr->make_selection(selection);
    }
    std::vector<int> get_selected_by_mouse() {
        return _ptr->get_selected_by_mouse();
    }
    std::vector<int> get_last_clicked_by_mouse() {
        return _ptr->get_last_clicked_by_mouse();
    }
};

/**
 * Use a set of dots(for example a graph) and have some util functions for it such as
 * - Automatically syncing graph position with window layout
 * - Fitting the dots in the approriate manner
 * - Getting the dot closest to where the mouse is pointing
*/
struct Dots {
    LayoutRectWidget layout;
    Rect dots_boundry;
    std::vector<double> x;
    std::vector<double> y;
    RectT<int> get_layout_position() {
        return layout.get_position();
    }
    void get_dot_pixel_position(int index, int& x, int& y) {
        ThrowNotImplemented();
    }
    /**
     * Get which dot is closest to the mouse
    */
    int get_closest_dot_index_to_mouse(WidgetManager::MouseInput mouse_input) {
        ThrowNotImplemented();
    }
    /**
     * Get how far a certain dot is from the mouse
    */
    double get_dot_mouse_pixel_dist(int index, WidgetManager::MouseInput mouse_input) {
        ThrowNotImplemented();
    }
    void fit_dots(int pixel_count_to_edge) { // Update the dots boundry to fit the dots(so they are a certain dist from edge)
        ThrowNotImplemented();
    }
};
}
}