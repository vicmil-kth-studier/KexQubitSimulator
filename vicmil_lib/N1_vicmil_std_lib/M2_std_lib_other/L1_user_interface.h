#include "../M1_std_lib_general/std_lib_general.h"

namespace vicmil {
namespace __layout__ {
    /**
     * A space element is the core builing block of an interface
     * It specifies how large a certain element can be(max and minimum size)
    */
    struct SpaceElement {
        int min_size = 0; // The minimum size in pixels
        int max_size = 1000000; // (max size in pixels, default basically inf.)
        int size_pick_priority = 0; // The ones with highest priority get their size picked first
        float weight = 1.0; // If multiple elements have the same priority and have to share space, it is distributed by weight
        SpaceElement() {}
        SpaceElement(int min_size_, int max_size_, int size_pick_priority_, float weight_) {
            min_size = min_size_;
            max_size = max_size_;
            size_pick_priority = size_pick_priority_;
            weight = weight_;
        }
        bool operator<(const SpaceElement& other) const {
            return size_pick_priority < other.size_pick_priority;
        }
        bool operator>(const SpaceElement& other) const {
            return size_pick_priority > other.size_pick_priority;
        }
    };

    /**
     * Sort space elements by priority, with highest priority first
     *  returns vector with indicies about how priorities are grouped
    */ 
    std::vector<std::vector<int>> _group_elements_by_priority(std::vector<SpaceElement> space_elements) {
        if(space_elements.size() == 0) {
            return {};
        }
        std::vector<std::pair<SpaceElement, int>> vec = vicmil::vec_sort_descend_and_get_indecies(space_elements);
        std::vector<std::vector<int>> return_vec = {{}};
        int current_priority = vec[0].first.size_pick_priority;
        for(int i = 0; i < vec.size(); i++) {
            if(current_priority == vec[i].first.size_pick_priority) {
                return_vec.back().push_back(vec[i].second);
            }
            else {
                return_vec.push_back({vec[i].second});
                current_priority = vec[i].first.size_pick_priority;
            }
        }
        return return_vec;
    }
    /**
     * Only keep elements in priority_groups that will fit(and remove the rest)
    */ 
    void _trim_priority_groups(std::vector<std::vector<int>>& priority_groups, 
                                const std::vector<SpaceElement>& space_elements, 
                                int max_size) {
        if(space_elements.size() == 0) {
            return;
        }
        // Get how many priority groups will fit
        int group_count = 0;
        int space_left = max_size;
        while(true) {
            int group_space = 0;
            for(int i = 0; i < priority_groups[group_count].size(); i++) {
                group_space += space_elements[priority_groups[group_count][i]].max_size;
            }
            if(group_space > space_left) {
                break;
            }
            space_left -= group_space;
            group_count++;
        }
        priority_groups.resize(group_count + 1);

        // Get how many elements in the last group that will fit
        std::vector<int>& last_priority_group = priority_groups.back();
        for(int i = 0; i < last_priority_group.size(); i++) {
            int size = space_elements[last_priority_group[i]].min_size;
            if(size > space_left) {
                last_priority_group.resize(i);
                return;
            }
            space_left -= size;
        }
    }
    /**
     * Sum the total weight of elements in priority group that has not been assigned any space left
    */
    float _get_priority_group_weight(const std::vector<SpaceElement>& space_elements, 
                                    std::vector<int>& priority_group, 
                                    const std::vector<int>& assigned_space_vec) {
        float total_weight = 0;
        for(int i = 0; i < priority_group.size(); i++) {
            int index = priority_group[i];
            if(assigned_space_vec[index] == 0) {
                total_weight += space_elements[index].weight;
            }
        }
        return total_weight;
    }
    /**
     * Assigns space to a list of space elements
     * The ones with the highest priority will be assigned their space first(max space)
     * If there is any space left, the rest will be divided by weight
    */
    std::vector<int> assign_space(const std::vector<SpaceElement>& space_elements, int max_size) {
        if(space_elements.size() == 0) {
            Print("no space elements");
            return {};
        }
        // Group elements by priority
        std::vector<std::vector<int>> priority_groups = _group_elements_by_priority(space_elements);
        _trim_priority_groups(priority_groups, space_elements, max_size);
        std::vector<int> return_vec = std::vector<int>(space_elements.size(), 0);

        // Assign space to higher priority groups
        int space_left = max_size;
        for(int i = 0; i < priority_groups.size() - 1; i++) {
            for(int j = 0; j < priority_groups[i].size(); j++) {
                Print("Assign space to higher priority group");
                int index = priority_groups[i][j];
                return_vec[index] = space_elements[index].max_size;
                space_left-= space_elements[index].max_size;
            }
        }

        // Assign space to lower priority groups based on weight
        std::vector<int>& last_priority_group = priority_groups.back();
        
        while(true) {
            // If the size is less than the weighted
            bool stop = true;
            float total_weight = _get_priority_group_weight(space_elements, last_priority_group, return_vec);
            int space_left_copy = space_left;
            if(total_weight == 0) {
                Print("total weight zero");
                return return_vec;
            }
            for(int i = 0; i < last_priority_group.size(); i++) {
                int index = last_priority_group[i];
                int size = space_left_copy * space_elements[index].weight / total_weight;
                if(return_vec[index] == 0 && size <= space_elements[index].min_size) {
                    Print("Assign space according to min space");
                    return_vec[index] = space_elements[index].min_size;
                    space_left-= space_elements[index].min_size;
                    stop = false;
                }
            }
            if(stop) {
                break;
            }
        }
        while(true) {
            // If the size is more than the weighted
            bool stop = true;
            float total_weight = _get_priority_group_weight(space_elements, last_priority_group, return_vec);
            int space_left_copy = space_left;
            if(total_weight == 0) {
                Print("total weight zero");
                return return_vec;
            }
            for(int i = 0; i < last_priority_group.size(); i++) {
                int index = last_priority_group[i];
                int size = space_left_copy * space_elements[index].weight / total_weight;
                if(return_vec[index] == 0 && size >= space_elements[index].max_size) {
                    Print("Assign space according to max space");
                    return_vec[index] = space_elements[index].max_size;
                    space_left-= space_elements[index].max_size;
                    stop = false;
                }
            }
            if(stop) {
                break;
            }
        }

        // Assign accordning to weight
        float total_weight = _get_priority_group_weight(space_elements, last_priority_group, return_vec);
        if(total_weight == 0) {
            Print("total weight zero");
            return return_vec;
        }
        int space_left_copy = space_left;
        for(int i = 0; i < last_priority_group.size(); i++) {
            Print("Assign space according to weight");
            int index = last_priority_group[i];
            int size = space_left_copy * space_elements[index].weight / total_weight;
            return_vec[index] = size;
        }
        return return_vec;
    }
    void TEST_assign_space() {
        std::vector<SpaceElement> space_elements = {
            SpaceElement(),
            SpaceElement(),
            SpaceElement()
        };
        int max_size = 1000;
        std::vector<int> space = assign_space(space_elements, max_size);
        AssertEq(space[0], space[1], 1.5);
        AssertEq(space[1], space[2], 1.5);
        AssertEq(space[0] + space[1] + space[2], max_size, 3);
    }
    AddTest(TEST_assign_space);

    /**
     * A rectangle on the screen
     * The screen can be split into a set of rectangles!
    */
    struct LayoutRectProperties {
        SpaceElement width;
        SpaceElement height;
        bool split_horizontal = true; // Otherwise split vertical
        std::vector<int> children_indecies = {};
    };

    /**
     * You can add attached elements to window elements; 
     * that means that when the element updates position,
     * the attached elements update function is called
    */
    class AttachedElement {
    public:
        virtual void updated_position_event() = 0; // Called when position is updated
    };

    /**
     * The class determining how the window is organized
    */
    struct _WindowLayout {
        int _elements_counter = 0;
        std::map<int, RectT<int>> layout_position;
        std::map<int, LayoutRectProperties> layout_properties;
        std::map<int, int> layout_parent;
        std::map<int, std::weak_ptr<AttachedElement>> anchors;
        std::map<int, std::weak_ptr<AttachedElement>> widgets;

        _WindowLayout() {
            layout_position[0] = RectT<int>(0, 0, 1000, 1000);
            layout_properties[0] = LayoutRectProperties();
            layout_parent[0] = 0;
            _elements_counter++;
        }

        std::vector<SpaceElement> _get_children_space_elements(int index) {
            LayoutRectProperties& properties = layout_properties[index];
            std::vector<SpaceElement> children_space_elements = std::vector<SpaceElement>(properties.children_indecies.size());
            if(properties.split_horizontal) {
                for(int i = 0; i < properties.children_indecies.size(); i++) {
                    int child_index = properties.children_indecies[i];
                    children_space_elements[i] = layout_properties[child_index].width;
                }
                return children_space_elements;
            }
            else {
                for(int i = 0; i < properties.children_indecies.size(); i++) {
                    int child_index = properties.children_indecies[i];
                    children_space_elements[i] = layout_properties[child_index].height;
                }
                return children_space_elements;
            }
        }
        void _update_child(int index, int size, int pos) {
            LayoutRectProperties& child_properties = layout_properties[index];
            LayoutRectProperties& parent_properties = layout_properties[layout_parent[index]];
            RectT<int> parent_pos = layout_position[layout_parent[index]];
            if(parent_properties.split_horizontal) {
                layout_position[index] = RectT<int>(pos + parent_pos.x, parent_pos.y, size, parent_pos.h);
            }
            else {
                layout_position[index] = RectT<int>(parent_pos.x, parent_pos.y + pos, parent_pos.w, size);
            }
            update_rect(index);
        }
        void _update_attached_elements(int index) {
            if(anchors.count(index) != 0 && !anchors[index].expired()) {
                anchors[index].lock()->updated_position_event();
            }
            if(widgets.count(index) != 0 && !widgets[index].expired()) {
                widgets[index].lock()->updated_position_event();
            }
        } 
        void update_rect(int index) {
            _update_attached_elements(index);
            LayoutRectProperties& properties = layout_properties[index];
            std::vector<SpaceElement> space_elements = _get_children_space_elements(index);
            std::vector<int> spaces = {};
            if(properties.split_horizontal) {
                spaces = assign_space(space_elements, layout_position[index].w);
            }
            else {
                spaces = assign_space(space_elements, layout_position[index].h);
            }
            int pos = 0;
            for(int i = 0; i < properties.children_indecies.size(); i++) {
                int child_index = properties.children_indecies[i];
                PrintExpr(child_index << " " << pos << " " << spaces[i]);
                _update_child(child_index, spaces[i], pos);
                pos += spaces[i];
            }
        }

        // The width and height of the layout in pixels
        void set_size(int width, int height) {
            layout_position[0] = RectT<int>(0, 0, width, height);
            update_rect(0);
        }
        int create_child(int index) {
            int child_index = _elements_counter;
            _elements_counter++;
            layout_properties[index].children_indecies.push_back(child_index);

            // Setup child
            layout_properties[child_index] = LayoutRectProperties();
            layout_parent[child_index] = index;
            layout_position[child_index] = RectT<int>(0, 0, 0, 0);
            return child_index;
        }
    };

    class LayoutRectReference {
    public:
        int index = 0;
        std::weak_ptr<_WindowLayout> window_layout = std::weak_ptr<_WindowLayout>();
        LayoutRectReference() {}
        LayoutRectReference(int index_, std::weak_ptr<_WindowLayout> window_layout_) {
            index = index_;
            window_layout = window_layout_;
        }
        void update() {
            window_layout.lock()->update_rect(window_layout.lock()->layout_parent[index]);
        }
        RectT<int> get_position() {
            return window_layout.lock()->layout_position[index];
        }
        RectT<int> get_entire_window_position() {
            return window_layout.lock()->layout_position[0];
        }
        LayoutRectProperties& get_properties() {
            return window_layout.lock()->layout_properties[index];
        }

        LayoutRectReference get_reference(int index_) {
            return LayoutRectReference(index_, window_layout);
        }
        LayoutRectReference create_child() {
            int child_index = window_layout.lock()->create_child(index);
            return get_reference(child_index);
        }
    };

    /**
     * The class holding a reference to WindowLayout
     * (This is the start class for all layout things)
    */
    class WindowLayout {
    public:
        std::shared_ptr<_WindowLayout> _window_layout = std::make_shared<_WindowLayout>();
        LayoutRectReference get_entire_window_reference() {
            return LayoutRectReference(0, _window_layout);
        }
        void set_size(int width, int height) {
            _window_layout->set_size(width, height);
        }
    };

    /**
     * Ensure that it is the right format for having a middle element
    */
    void _make_middle_element_format(LayoutRectReference rect_reference) {
        rect_reference.get_properties().split_horizontal = true;
        std::vector<int>& children_horizontal = rect_reference.get_properties().children_indecies;
        if(children_horizontal.size() > 3) {
            children_horizontal.resize(3);
        }
        while(children_horizontal.size() < 3) {
            rect_reference.create_child();
        }
        LayoutRectReference mid_child = rect_reference.get_reference(children_horizontal[1]);
        mid_child.get_properties().split_horizontal = false;
        std::vector<int>& children_vertical = mid_child.get_properties().children_indecies;
        if(children_vertical.size() > 3) {
            children_vertical.resize(3);
        }
        while(children_vertical.size() != 3) {
            mid_child.create_child();
        }
    }

    /**
     * Returns an element that is in the middle, with buffers to the sides
    */
    LayoutRectReference make_element_in_middle(LayoutRectReference rect_reference, 
        SpaceElement left, SpaceElement right, 
        SpaceElement top, SpaceElement bottom) {
        _make_middle_element_format(rect_reference);
        std::vector<int>& children_horizontal = rect_reference.get_properties().children_indecies;
        Assert(children_horizontal.size() == 3);

        LayoutRectReference mid_child = rect_reference.get_reference(children_horizontal[1]);
        std::vector<int>& children_vertical = mid_child.get_properties().children_indecies;
        Assert(children_vertical.size() == 3);

        // Get children references(Those that are relevant)
        LayoutRectReference left_child = rect_reference.get_reference(children_horizontal[0]);
        LayoutRectReference right_child = rect_reference.get_reference(children_horizontal[2]);
        LayoutRectReference top_child = rect_reference.get_reference(children_vertical[0]);
        LayoutRectReference bottom_child = rect_reference.get_reference(children_vertical[2]);
        LayoutRectReference mid_mid_child = rect_reference.get_reference(children_vertical[1]);
        
        // Setup properties
        rect_reference.get_properties().split_horizontal = true;
        mid_child.get_properties().width = SpaceElement(0, 100000, 0, 1);
        mid_child.get_properties().height = SpaceElement(0, 100000, 0, 1);
        mid_child.get_properties().split_horizontal = false;

        left_child.get_properties().width = left;
        right_child.get_properties().width = right;
        top_child.get_properties().height = top;
        bottom_child.get_properties().height = bottom;
        return mid_mid_child;
    }

    LayoutRectReference make_element_in_middle_from_weight(
        LayoutRectReference rect_reference, float left, float right, float top, float bottom) {
        SpaceElement left_ = SpaceElement(0, 100000, 0, left);
        SpaceElement right_ = SpaceElement(0, 100000, 0, right);
        SpaceElement top_ = SpaceElement(0, 100000, 0, top);
        SpaceElement bottom_ = SpaceElement(0, 100000, 0, bottom);
        return make_element_in_middle(rect_reference, left_, right_, top_, bottom_);
    }

    LayoutRectReference make_element_in_middle_from_size(
        LayoutRectReference rect_reference, int left, int right, int top, int bottom) {
        SpaceElement left_ = SpaceElement(left, left, 1, 1);
        SpaceElement right_ = SpaceElement(right, right, 1, 1);
        SpaceElement top_ = SpaceElement(top, top, 1, 1);
        SpaceElement bottom_ = SpaceElement(bottom, bottom, 1, 1);
        return make_element_in_middle(rect_reference, left_, right_, top_, bottom_);
    }

    /**
     * Anchors can be attached to window elements, they float above. This can be great for menus for example!
    */

    class _Anchor: public AttachedElement {
    public:
        LayoutRectReference attached_element;
        RectAlignment alignment;
        
        std::shared_ptr<_WindowLayout> window_layout = std::make_shared<_WindowLayout>();
        LayoutRectReference anchor_element;

        RectT<int> _get_anchor_pos() { // Get the anchor position
            RectT<int> attached_element_position = attached_element.get_position();
            int element_max_width = attached_element.get_properties().width.max_size;
            int element_max_height = attached_element.get_properties().height.max_size;
            RectT<int> new_anchor_pos = RectT<int>(0, 0, element_max_width, element_max_height);
            align_rect(new_anchor_pos, attached_element_position, alignment);
            cut_rect_to_fit(new_anchor_pos, attached_element.get_entire_window_position());

            return new_anchor_pos;
        }
        void _update_side_buffers() {
            RectT<int> entire_window_pos = attached_element.get_entire_window_position();
            RectT<int> anchor_pos = _get_anchor_pos();
            int buffer_left = anchor_pos.min_x() - entire_window_pos.min_x();
            int buffer_right = entire_window_pos.max_x() - anchor_pos.max_x();
            int buffer_up = anchor_pos.min_y() - entire_window_pos.min_y();
            int buffer_down = entire_window_pos.max_y() - anchor_pos.max_y();

            LayoutRectReference entire_window = LayoutRectReference(0, window_layout);
            anchor_element = make_element_in_middle_from_size(entire_window, 
                buffer_left,
                buffer_right,
                buffer_up,
                buffer_down
            );
        }
        void updated_position_event() override {
            Print("Anchor update()");
            // Update the entire window position
            _update_side_buffers();
            window_layout->update_rect(0);
        }
    };
    class Anchor {
        std::shared_ptr<_Anchor> anchor;
        void set_layout_element(LayoutRectReference layout_element_) {
            anchor->attached_element = layout_element_;
            layout_element_.window_layout.lock()->anchors[layout_element_.index] = anchor;
        }
        LayoutRectReference get_anchor_element() {
            return anchor->anchor_element;
        }
        void set_anchor_alignment(RectAlignment alignment) {
            if(anchor->alignment != alignment) {
                anchor->alignment = alignment;
                anchor->window_layout->update_rect(0);
            }
        }
    };

    class _Widget: public AttachedElement {
    public:
        bool _is_selected = false;
        bool _is_last_clicked_widget = false; 
        bool _was_clicked = false;
        LayoutRectReference _layout_element; // [Optional] attached layout element
        double depth = 0; // In case two widgets are ontop of each other, the one with lowest depth will be ontop
        RectT<int> position = RectT<int>(0, 0, 0, 0);
        void updated_position_event() override { // Called when layout element changes position
            position = _layout_element.get_position();
        }
        _Widget() {}
    };
    /**
     * A widget is an element on the screen that can be interacted with, for example pressed on with the mouse
     *  The WidgetManager keeps track of all the widgets.
     * A widget can be attached to a WindowElement, and therefore update its position dynamically!
    */
    class Widget {
    public:
        std::shared_ptr<_Widget> widget;
        Widget() {
            widget = std::make_shared<_Widget>();
        }
        bool is_selected() {return widget->_is_selected;} // If the mouse is hovering above widget
        bool was_clicked() {return widget->_was_clicked; }
        bool is_last_clicked_widget() {return widget->_is_last_clicked_widget;} // If mouse havn't pressed anything since it last pressed widget
        
        // The layout element will update the position of the widget automatically as it moves
        void set_layout_element(LayoutRectReference layout_element_) {
            widget->_layout_element = layout_element_;
            layout_element_.window_layout.lock()->widgets[layout_element_.index] = widget;
            widget->position = layout_element_.get_position();
        }
        LayoutRectReference get_layout_element() {
            return widget->_layout_element;
        }
        // Manually set widget position
        void set_position(RectT<int> position) {
            widget->position = position;
        }
        RectT<int> get_position() {
            return widget->position;
        }
    };

    struct MouseInput {
        int x = 0;
        int y = 0;
        bool was_clicked = false;
    };
    /**
     * The window manager keeps track of all the widgets, like which widget is the one the user is pointing at
     * (This can be good if you have widgets stacked ontop of each other)
     * It also makes sure to update all the widgets each time it gets an update call
    */
    class WidgetManager {
    public:
        std::list<std::weak_ptr<_Widget>> widgets = {};
        WidgetManager() {}
        void add_widget(const Widget& new_widget) {
            widgets.push_back(new_widget.widget);
        }
        void _reset_widget_states(MouseInput mouse_input) {
            auto it = widgets.begin();
            while(it != widgets.end()) {
                _Widget& widget = *(*it).lock();
                widget._is_selected = false;
                widget._was_clicked = false;
                if(mouse_input.was_clicked) {
                    widget._is_last_clicked_widget = false;
                }
                it++;
            }
        }
        void _set_selected_widget(std::weak_ptr<_Widget> widget_, MouseInput mouse_input) {
            if(widget_.expired()) {
                return;
            }
            _Widget& widget = *widget_.lock();
            widget._is_selected = true;
            if(mouse_input.was_clicked) {
                widget._is_last_clicked_widget = true;
                widget._was_clicked = true;
            }
        }
        // Returns an expired weak ptr if it fails
        std::weak_ptr<_Widget> _get_selected_widget(MouseInput mouse_input) {
            std::weak_ptr<_Widget> selected_widget = std::weak_ptr<_Widget>();
            auto it = widgets.begin();
            while(it != widgets.end()) {
                _Widget& widget = *(*it).lock();
                if(widget.position.is_inside_rect(mouse_input.x, mouse_input.y)) {
                    if(selected_widget.expired() || widget.depth < selected_widget.lock()->depth) {
                        selected_widget = *it;
                    }
                }
                it++;
            }
            return selected_widget;
        }
        int update(MouseInput mouse_input) {
            // Update widgets
            remove_expired_from_list(widgets);
            _reset_widget_states(mouse_input);
            std::weak_ptr<_Widget> selected_widget = _get_selected_widget(mouse_input);
            _set_selected_widget(selected_widget, mouse_input);
            return 0;
        }
    };
};
};