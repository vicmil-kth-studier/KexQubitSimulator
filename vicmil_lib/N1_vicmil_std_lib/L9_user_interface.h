#include "L8_typing.h"

namespace vicmil {
    /**
     * General template class for rectangle
    */
    template<class T>
    class RectT {
        public:
        T x = 0;
        T y = 0;
        T w = 0;
        T h = 0;
        RectT() {}
        RectT(T x_, T y_, T w_, T h_) {
            x = x_;
            y = y_;
            w = w_;
            h = h_;
        }
        T min_x() {
            return x;
        }
        T max_x() {
            return x + w;
        }
        T min_y() {
            return y;
        }
        T max_y() {
            return y + h;
        }
        T center_x() {
            return x + (w / 2.0);
        }
        T center_y() {
            return y + (h / 2.0);
        }
        bool is_inside_rect(T x_, T y_) {
            if(!in_range<T>(x_, x, x + w)) {
                return false;
            }
            if(!in_range<T>(y_, y, y + h)) {
                return false;
            }
            return true;
        }
        std::string to_string() {
            return "x: " + std::to_string(x) + 
                    "   y: " + std::to_string(y) + 
                    "   w: " + std::to_string(w) + 
                    "   h: " + std::to_string(h);
        }
        std::string to_string_min_max() {
            return "min_x: " + std::to_string(min_x()) + 
                    "   min_y: " + std::to_string(min_y()) + 
                    "   max_x: " + std::to_string(max_x()) + 
                    "   max_y: " + std::to_string(max_y());
        }
        bool operator==(const RectT<T> other) {
            return 
                x == other.x &&
                y == other.y &&
                w == other.w &&
                h == other.h;
        }
        bool operator!=(const RectT<T> other) {
            return !(*this == other);
        }
    };
    typedef RectT<double> Rect;

    template<class T>
    bool is_overlap(RectT<T> rect1, RectT<T> rect2) {
        Assert(rect1.w > 0);
        Assert(rect1.h > 0);
        Assert(rect2.w > 0);
        Assert(rect2.h > 0);
        if(rect1.min_x() >= rect2.max_x() || rect2.min_x() >= rect1.max_x()) {
            return false;
        }
        if(rect1.min_y() >= rect2.max_y() || rect2.min_y() >= rect1.max_y()) {
            return false;
        }
        return true;
    }

    /**
     * Get the smallest rectangle so that no part of rect1 and rect2 is outside it
    */
    template<class T>
    Rect get_bounding_rect(RectT<T> rect1, RectT<T> rect2) {
        T min_x = std::min(rect1.min_x(), rect2.min_x());
        T min_y = std::min(rect1.min_y(), rect2.min_y());
        T max_x = std::max(rect1.max_x(), rect2.max_x());
        T max_y = std::max(rect1.max_y(), rect2.max_y());
        return Rect(min_x, min_y, max_x - min_x, max_y - min_y);
    }

namespace __layout__ {

    class AttachedElement {
    public:
        virtual void update() = 0;
    };

    struct _WindowLayoutElement {
        RectT<int> pixel_position = RectT<int>(0, 0, 100, 100);
        int min_width = 0;
        int max_width = -1; // -1 means max size
        int min_height = 0;
        int max_height = -1; // -1 means max size
        double weight = 1;
        int priority = 0;
        bool split_horizontal = true; // Otherwise split vertical
        std::vector<int> children_indecies = {};
        int parent_index = -1;
        std::weak_ptr<class AttachedElement> attached_element = std::weak_ptr<AttachedElement>();
        //bool updated = false; // If it has been updated since last update call
    };

    struct _WindowLayout {
    public:
        std::map<int, _WindowLayoutElement> _layout_elements = {};
        int _elements_counter = 0;
        bool element_exists(int index) {
            return _layout_elements.count(index) != 0;
        }
    };

    class WindowLayoutElement {
    public:
        std::weak_ptr<class _WindowLayout> _layout_ref = std::weak_ptr<class _WindowLayout>();
        int _element_index = -1;
        // Can be split vertical, or horizontal
        // The screen will be split according to:
        // First it will be assigned to elements with absolute pixel count
        // Second if there is space, the rest will go to weighted elements

        WindowLayoutElement _get_window_layout_element(int index) {
            WindowLayoutElement new_element;
            new_element._element_index = index;
            new_element._layout_ref = _layout_ref;
            return new_element;
        }

        int set_vertical_split() {
            if(_layout_ref.expired() || !_layout_ref.lock()->element_exists(_element_index)) {
                return -1;
            }
            _layout_ref.lock()->_layout_elements[_element_index].split_horizontal = false;
            return 0;
        }
        int set_horizontal_split() {
            if(_layout_ref.expired() || !_layout_ref.lock()->element_exists(_element_index)) {
                return -1;
            }
            _layout_ref.lock()->_layout_elements[_element_index].split_horizontal = true;
            return 0;
        }

        // Set the desired pixel dimensions(set -1 for unspecified)
        int set_size(int min_width, int max_width, int min_height, int max_height) {
            if(_layout_ref.expired() || !_layout_ref.lock()->element_exists(_element_index)) {
                return -1;
            }
            _WindowLayoutElement& element = _layout_ref.lock()->_layout_elements[_element_index];
            element.min_width = min_width;
            element.max_width = max_width;
            element.min_height = min_height;
            element.max_height = max_height;
            return 0;
        }
        int set_size(int width, int height) {
            return set_size(width, width, height, height);
        }
        int set_width(int min_width, int max_width) {
            if(_layout_ref.expired() || !_layout_ref.lock()->element_exists(_element_index)) {
                return -1;
            }
            _WindowLayoutElement& element = _layout_ref.lock()->_layout_elements[_element_index];
            return set_size(min_width, max_width, element.min_height, element.max_height);
        }
        int set_height(int min_height, int max_height) {
            if(_layout_ref.expired() || !_layout_ref.lock()->element_exists(_element_index)) {
                return -1;
            }
            _WindowLayoutElement& element = _layout_ref.lock()->_layout_elements[_element_index];
            return set_size(element.min_width, element.max_width, min_height, max_height);
        }

        // Default is 0. The order to hide elements if necessary, lowest first
        //  (in case of twist, it will hide the last child element first)
        int set_priority_level(int priority) {
            if(_layout_ref.expired() || !_layout_ref.lock()->element_exists(_element_index)) {
                return -1;
            }
            _layout_ref.lock()->_layout_elements[_element_index].priority = priority;
            return 0;
        }

        // Where the element is on screen
        RectT<int> get_position() {
            if(_layout_ref.expired() || !_layout_ref.lock()->element_exists(_element_index)) {
                return RectT<int>(0, 0, 0, 0);
            }
            return _layout_ref.lock()->_layout_elements[_element_index].pixel_position;
        }

        // Set a class to be called each time the element is updated
        int set_attached_element(std::weak_ptr<AttachedElement> attached_element_) {
            if(_layout_ref.expired() || !_layout_ref.lock()->element_exists(_element_index)) {
                return -1;
            }
            Print("Set attached element!");
            _layout_ref.lock()->_layout_elements[_element_index].attached_element = attached_element_;
            return 0;
        }


        // Get the element sizes based on min-max requirements
        static std::vector<int> _get_element_sizes(
            const std::vector<int>& size_min, 
            const std::vector<int>& size_max, 
            const std::vector<double>& weight,
            int total_size_) {
            Assert(total_size_ >= vicmil::vec_sum(size_min));

            std::vector<int> return_vec = {};
            return_vec.resize(weight.size(), -1);

            int current_total_size = total_size_;
            double current_total_weight = vicmil::vec_sum(weight);

            // Iterate until everyone meets their minimum requirements
            bool stop = false;
            while(stop == false && current_total_size > 0 && current_total_weight > 0) {
                stop = true;
                int size_left = current_total_size;
                double weight_left = current_total_weight;
                for(int i = 0; i < weight.size(); i++) {
                    if(return_vec[i] != -1) {
                        continue; // Already set
                    }
                    int size = current_total_size * weight[i] / current_total_weight;
                    if(size < size_min[i]) {
                        // If some go below requirements, they get their required size and are done!
                        return_vec[i] = size_min[i];
                        size_left -= size_min[i];
                        weight_left -= weight[i];
                        stop = false; // We are clearly not done! Do another iteration
                    }
                }
                current_total_size = size_left;
                current_total_weight = weight_left;
            }
            
            // Iterate until everyone meets their maximum requirements
            stop = false;
            while(stop == false && current_total_size > 0 && current_total_weight > 0) {
                stop = true;
                int size_left = current_total_size;
                double weight_left = current_total_weight;
                for(int i = 0; i < weight.size(); i++) {
                    if(return_vec[i] != -1) {
                        continue; // Already set
                    }
                    int size = current_total_size * weight[i] / current_total_weight;
                    if(size > size_max[i]) {
                        // If some go below requirements, they get their required size and are done!
                        return_vec[i] = size_max[i];
                        size_left -= size_max[i];
                        weight_left -= weight[i];
                        stop = false; // We are clearly not done! Do another iteration
                    }
                }
                current_total_size = size_left;
                current_total_weight = weight_left;
            }
            
            // Assign the rest according to weight
            for(int i = 0; i < weight.size(); i++) {
                if(return_vec[i] != -1) {
                    continue; // Already set
                }
                int size = current_total_size * weight[i] / current_total_weight;
                return_vec[i] = size;
            }

            return return_vec;
        }

        /**
         * Hide elements with lowest priority first
         * (Hide means setting min size and max size to 0)
        */
        static void _hide_elements_if_all_doesnt_fit(
            std::vector<int>& size_min, 
            std::vector<int>& size_max, 
            const std::vector<int>& priorities,
            int total_size) {
            int min_size_sum = vicmil::vec_sum(size_min);
            if(min_size_sum <= total_size) {
                return; // Everything already fits!
            }

            DebugExpr(min_size_sum);
            DebugExpr(total_size);
            Assert(min_size_sum >= 0);
            Assert(total_size >= 0);
            Assert(size_min.size() > 0);
            Assert(size_max.size() > 0);
            Assert(priorities.size() > 0);
            
            // Sort the elements based on priorities
            std::vector<std::pair<int, int>> priorities_with_index = vicmil::vec_sort_descend_and_get_indecies(priorities);
            Assert(priorities_with_index.size() > 0);

            int i = priorities_with_index.size() - 1;
            // Start from the back and reduce elements until they are in acceptable range
            while(min_size_sum > total_size) {
                Assert(i >= 0);
                int index = priorities_with_index[i].second;
                min_size_sum -= size_min[index];
                size_min[index] = 0;
                size_max[index] = 0;
                i -= 1;
            }
        }

        /**
         * Get a list of all childrens minimum sizes
         *  (in the horizontal or vertical direction depending on split direction)
         * eg. {child0.min_width, child1.min_width, child2.min_width, ...}
        */
        std::vector<int> get_min_size_of_all_children() {
            std::vector<int> return_vec = {};

            _WindowLayoutElement& element = _layout_ref.lock()->_layout_elements[_element_index];
            for(int i = 0; i < element.children_indecies.size(); i++) {
                _WindowLayoutElement& child = _layout_ref.lock()->_layout_elements[element.children_indecies[i]];
                if(element.split_horizontal) {
                    return_vec.push_back(child.min_width);
                }
                else {
                    return_vec.push_back(child.min_height);
                }
            }
            return return_vec;
        }

        /**
         * Get a list of all childrens maximum sizes
         *  (in the horizontal or vertical direction depending on split direction)
         * eg. {child0.max_width, child1.max_width, child2.max_width, ...}
        */
        std::vector<int> get_max_size_of_all_children() {
            std::vector<int> return_vec = {};

            _WindowLayoutElement& element = _layout_ref.lock()->_layout_elements[_element_index];
            for(int i = 0; i < element.children_indecies.size(); i++) {
                _WindowLayoutElement& child = _layout_ref.lock()->_layout_elements[element.children_indecies[i]];
                if(element.split_horizontal) {
                    if(child.max_width < 0) {
                        return_vec.push_back(element.pixel_position.w );
                    }
                    else {
                        return_vec.push_back(child.max_width);
                    }
                }
                else {
                    if(child.max_height < 0) {
                        return_vec.push_back(element.pixel_position.h );
                    }
                    else {
                        return_vec.push_back(child.max_height);
                    }
                }
            }
            return return_vec;
        }

        int _parent_update() {
            if(_layout_ref.expired() || !_layout_ref.lock()->element_exists(_element_index)) {
                return -1;
            }
            _WindowLayoutElement& element = _layout_ref.lock()->_layout_elements[_element_index];
            if(element.parent_index >= 0) {
                _get_window_layout_element(element.parent_index)._update();
            }
            return 0;
        }

        int update() {
            _WindowLayoutElement& element = _layout_ref.lock()->_layout_elements[_element_index];
            DebugExpr(element.pixel_position.to_string());
            _parent_update();
            DebugExpr(element.pixel_position.to_string());
            _update();
            DebugExpr(element.pixel_position.to_string());
            return 0;
        }

        void _set_child_position(int pos, int size, int child_index) {
            _WindowLayoutElement& element = _layout_ref.lock()->_layout_elements[_element_index];
            _WindowLayoutElement& child = _layout_ref.lock()->_layout_elements[element.children_indecies[child_index]];

            vicmil::RectT<int> new_position = element.pixel_position;
            if(size == 0) {
                new_position = RectT<int>(0, 0, 0, 0);
            }
            else if(element.split_horizontal) {
                new_position.x = pos;
                new_position.w = size;
                if(child.max_height >= 0) {
                    new_position.h = std::min(element.pixel_position.h, child.max_height);
                }
            }
            else {
                new_position.y = pos;
                new_position.h = size;
                if(child.max_width >= 0) {
                    new_position.w = std::min(element.pixel_position.w, child.max_width);
                }
            }

            //if(child.pixel_position != new_position) {
            child.pixel_position = new_position;
            _get_window_layout_element(element.children_indecies[child_index])._update();
            //}
        }

        // Update children elements
        int _update() {
            if(_layout_ref.expired() || !_layout_ref.lock()->element_exists(_element_index)) {
                return -1;
            }
            _WindowLayoutElement& element = _layout_ref.lock()->_layout_elements[_element_index];
            
            // Call attached element
            if(!element.attached_element.expired()) {
                element.attached_element.lock()->update();
            }
            else {
                Print("element.attached_element expired");
            }

            // Fetch children parameters
            std::vector<int> children_size_min = get_min_size_of_all_children();
            std::vector<int> children_size_max = get_max_size_of_all_children();

            std::vector<int> children_priority_list = {};
            std::vector<double> children_weight_list = {};
            for(int i = 0; i < element.children_indecies.size(); i++) {
                _WindowLayoutElement& child = _layout_ref.lock()->_layout_elements[element.children_indecies[i]];
                children_priority_list.push_back(child.priority);
                children_weight_list.push_back(child.weight);
            }
            Assert(children_size_min.size() == children_size_max.size());
            Assert(children_size_min.size() == children_priority_list.size());
            Assert(children_size_min.size() == children_weight_list.size());

            int total_size = -1;
            if(element.split_horizontal) {
                total_size = element.pixel_position.w;
            }
            else {
                total_size = element.pixel_position.h;
            }
            Assert(total_size >= 0);

            _hide_elements_if_all_doesnt_fit(
                children_size_min,
                children_size_max,
                children_priority_list,
                total_size
            );

            std::vector<int> sizes = _get_element_sizes(
                children_size_min,
                children_size_max,
                children_weight_list,
                total_size
            );

            int pos;
            if(element.split_horizontal) {
                pos = element.pixel_position.x;
            } else {
                pos = element.pixel_position.y;
            }

            for(int i = 0; i < element.children_indecies.size(); i++) {
                _set_child_position(pos, sizes[i], i);
                pos += sizes[i];
            }

            return 0;
        }

        int erase() {
            if(_layout_ref.expired() || !_layout_ref.lock()->element_exists(_element_index)) {
                return -1;
            }
            _WindowLayoutElement& element = _layout_ref.lock()->_layout_elements[_element_index];
            WindowLayoutElement parent_ = _get_window_layout_element(element.parent_index);
            
            _erase();

            parent_._update();
            return 0;
        }

        int erase_children() {
            if(_layout_ref.expired() || !_layout_ref.lock()->element_exists(_element_index)) {
                return -1;
            }
            _WindowLayoutElement& element = _layout_ref.lock()->_layout_elements[_element_index];

            // Delete all children
            while(element.children_indecies.size() > 0) {
                int child_element_index = element.children_indecies.back();
                _get_window_layout_element(child_element_index)._erase();
            }
            return 0;
        }

        // Delete the element and all its children
        int _erase() {
            if(_layout_ref.expired() || !_layout_ref.lock()->element_exists(_element_index)) {
                return -1;
            }

            _WindowLayoutElement& element = _layout_ref.lock()->_layout_elements[_element_index];
            _WindowLayoutElement& parent_element = _layout_ref.lock()->_layout_elements[element.parent_index];

            // Remove reference from parent
            for(int i = 0; i < parent_element.children_indecies.size(); i++) {
                if(parent_element.children_indecies[i] == _element_index) {
                    vicmil::vec_remove(parent_element.children_indecies, i);
                    break;
                }
            }
            
            erase_children();

            // Remove window element from elements
            _layout_ref.lock()->_layout_elements.erase(_element_index);
            return 0;
        }

        // Create a new window element at bottom if vertical split / to the left if horizontal split
        WindowLayoutElement create_child_element() {
            if(_layout_ref.expired() || !_layout_ref.lock()->element_exists(_element_index)) {
                return WindowLayoutElement();
            }
            int child_index = _layout_ref.lock()->_elements_counter;
            _layout_ref.lock()->_elements_counter++;
            _layout_ref.lock()->_layout_elements[child_index] = _WindowLayoutElement();
            _layout_ref.lock()->_layout_elements[child_index].parent_index = _element_index;

            _layout_ref.lock()->_layout_elements[_element_index].children_indecies.push_back(child_index);
            return _get_window_layout_element(child_index);
        }
        std::vector<WindowLayoutElement> get_children() {
            if(_layout_ref.expired() || !_layout_ref.lock()->element_exists(_element_index)) {
                return {};
            }
            std::vector<int>& children_indecies = _layout_ref.lock()->_layout_elements[_element_index].children_indecies;
            std::vector<WindowLayoutElement> children = {};
            for(int i = 0; i < children_indecies.size(); i++) {
                children.push_back(_get_window_layout_element(children_indecies[i]));
            }
            return children;
        }
    };

    class WindowLayout {
    public:
        std::shared_ptr<_WindowLayout> _window_layout = std::make_shared<_WindowLayout>();
        WindowLayoutElement _entire_window_element;

        WindowLayout() {
            _window_layout->_layout_elements[_window_layout->_elements_counter] = _WindowLayoutElement();
            _entire_window_element = WindowLayoutElement();
            _entire_window_element._element_index = _window_layout->_elements_counter;
            _entire_window_element._layout_ref = _window_layout;
            _window_layout->_elements_counter++;
        }

        // The width and height of the layout in pixels
        void set_size(int width, int height) {
            _window_layout->_layout_elements[_entire_window_element._element_index].pixel_position =
                RectT<int>(0,0,width,height);
            _entire_window_element.update();
        }

        // Get the window element for the entire screen
        WindowLayoutElement get_window_element() {
            return _entire_window_element;
        }
    };
    void TEST1_WindowLayout() {
        WindowLayout window_layout = WindowLayout();
        WindowLayoutElement whole_screen = window_layout.get_window_element();
        WindowLayoutElement left_screen = whole_screen.create_child_element();
        WindowLayoutElement right_screen = whole_screen.create_child_element();
        window_layout.set_size(1000, 800);
        Assert(left_screen.get_position() == RectT<int>(0, 0, 500, 800));
        Assert(right_screen.get_position() == RectT<int>(500, 0, 500, 800));

        whole_screen.set_vertical_split();
        whole_screen.update();

        Assert(left_screen.get_position() == RectT<int>(0, 0, 1000, 400));
        Assert(right_screen.get_position() == RectT<int>(0, 400, 1000, 400));
    }
    AddTest(TEST1_WindowLayout);

    void TEST2_WindowLayout() {
        WindowLayout window_layout = WindowLayout();
        WindowLayoutElement whole_screen = window_layout.get_window_element();
        WindowLayoutElement left_screen = whole_screen.create_child_element();
        left_screen.set_size(200, 200);
        WindowLayoutElement right_screen = whole_screen.create_child_element();
        window_layout.set_size(1000, 800);
        Assert(left_screen.get_position() == RectT<int>(0, 0, 200, 200));
        Assert(right_screen.get_position() == RectT<int>(200, 0, 800, 800));

        left_screen.set_size(600, 600);
        left_screen.update();
        DebugExpr(left_screen.get_position().to_string());
        Assert(left_screen.get_position() == RectT<int>(0, 0, 600, 600));
        Assert(right_screen.get_position() == RectT<int>(600, 0, 400, 800));

        // Assert right one disappears if both won't fit(and they have same priority)
        right_screen.set_size(600, 600);
        right_screen.update();
        Assert(left_screen.get_position() == RectT<int>(0, 0, 600, 600));
        DebugExpr(right_screen.get_position().to_string());
        Assert(right_screen.get_position() == RectT<int>(0, 0, 0, 0));

        // Assert left one dissapears if right one has higher priority
        right_screen.set_priority_level(2);
        right_screen.update();
        DebugExpr(right_screen.get_position().to_string());
        DebugExpr(left_screen.get_position().to_string());
        Assert(left_screen.get_position() == RectT<int>(0, 0, 0, 0));
        Assert(right_screen.get_position() == RectT<int>(0, 0, 600, 600));
        
        // Assert that left one gets back if we delete the right
        right_screen.erase();
        Assert(left_screen.get_position() == RectT<int>(0, 0, 600, 600));
        Assert(right_screen.get_position() == RectT<int>(0, 0, 0, 0));
    }
    AddTest(TEST2_WindowLayout);


    struct _Anchor: public AttachedElement {
        WindowLayoutElement attached_element;
        bool attach_top = false;
        bool attach_left = false;
        bool dock_horizontal = false; // Otherwise dock vertical
        WindowLayout window_layout = WindowLayout();
        WindowLayoutElement alignment_element_left;
        WindowLayoutElement alignment_element_right;
        WindowLayoutElement alignment_element_top;
        WindowLayoutElement alignment_element_bottom;
        WindowLayoutElement anchor_element;
        void _setup_left_alignment(RectT<int> attached_element_position,
            int element_max_width) {
            int size = 0;
            if(attach_left && dock_horizontal) {
                size = attached_element_position.min_x() - element_max_width;
            }
            else if(!attach_left && dock_horizontal) {
                size = attached_element_position.max_x();
            }
            else if(attach_left && !dock_horizontal) {
                size = attached_element_position.min_x();
            }
            else if(!attach_left && !dock_horizontal) {
                size = attached_element_position.max_x() - element_max_width;
            }
            size = std::max(size, 0);
            alignment_element_left.set_width(size, size);
        }
        void _setup_right_alignment(
            RectT<int> entire_window_position,
            RectT<int> attached_element_position
        ) {
            int size = 0;
            if(attach_left && dock_horizontal) {
                size = entire_window_position.w - attached_element_position.min_x();
            }
            size = std::max(size, 0);
            alignment_element_right.set_width(size, size);
        }
        void _setup_top_alignment(RectT<int> attached_element_position,
            int element_max_height) {
            int size = 0;
            if(attach_top && dock_horizontal) {
                size = attached_element_position.min_y();
            }
            else if(!attach_top && dock_horizontal) {
                size = attached_element_position.max_y() - element_max_height;
            }
            else if(attach_top && !dock_horizontal) {
                size = attached_element_position.min_y() - element_max_height;
            }
            else if(!attach_top && !dock_horizontal) {
                size = attached_element_position.max_y();
            }
            size = std::max(size, 0);
            alignment_element_top.set_height(size, size);

        }
        void _setup_bottom_alignment(
            RectT<int> entire_window_position,
            RectT<int> attached_element_position
        ) {
            int size = 0;
            if(attach_top && !dock_horizontal) {
                size = entire_window_position.h - attached_element_position.min_y();
            }
            size = std::max(size, 0);
            alignment_element_bottom.set_height(size, size);
        }
        void _update_anchor_alignment() {
            RectT<int> entire_window_position = window_layout.get_window_element().get_position();
            RectT<int> attached_element_position = attached_element.get_position();
            _WindowLayoutElement& element_ = window_layout._window_layout->_layout_elements[anchor_element._element_index];

            if(attached_element_position.w == 0 || attached_element_position.h == 0) {
                // Hide the anchor(by making no space left for it)
                alignment_element_left.set_width(entire_window_position.w, entire_window_position.w);
                return;
            }

            int element_max_width = element_.max_width;
            int element_max_height = element_.max_height;
            if(element_max_width < 0) {
                element_max_width = entire_window_position.w;
            }
            if(element_max_height < 0) {
                element_max_height = entire_window_position.h;
            }
            _setup_left_alignment(attached_element_position, element_max_width);
            _setup_right_alignment(entire_window_position, attached_element_position);
            _setup_top_alignment(attached_element_position, element_max_height);
            _setup_bottom_alignment(entire_window_position, attached_element_position);
        }
        void update() override {
            Print("Anchor update()");
            // Update the entire window position
            RectT<int> entire_window_position = attached_element._layout_ref.lock()->_layout_elements[0].pixel_position;
            window_layout._window_layout->_layout_elements[0].pixel_position = entire_window_position;
            _update_anchor_alignment();
            window_layout.get_window_element().update();
        }
        _Anchor() {
            // Setup alignment correctly
            WindowLayoutElement entire_window_element = window_layout.get_window_element();
            entire_window_element.set_horizontal_split();
            alignment_element_left = entire_window_element.create_child_element();
            WindowLayoutElement temp = entire_window_element.create_child_element();
            alignment_element_right = entire_window_element.create_child_element();

            temp.set_vertical_split();
            alignment_element_top = temp.create_child_element();
            anchor_element = temp.create_child_element();
            alignment_element_bottom = temp.create_child_element();

            alignment_element_left.set_priority_level(1);
            alignment_element_right.set_priority_level(1);
            alignment_element_top.set_priority_level(1);
            alignment_element_bottom.set_priority_level(1);
        }
    };
    class Anchor {
    public:
        std::shared_ptr<_Anchor> _anchor = std::shared_ptr<_Anchor>();
        Anchor(WindowLayoutElement layout_element) {
            _anchor = std::make_shared<_Anchor>(_Anchor());
            _anchor->attached_element = layout_element;
            layout_element.set_attached_element(_anchor);
        }
        Anchor() {}
        WindowLayoutElement get_window_layout_element() {
            return _anchor->anchor_element;
        }
        void set_size(int width, int height) {
            _anchor->anchor_element.set_size(width, height);
        }
        void set_attach_top() {
            _anchor->dock_horizontal = false;
            _anchor->attach_top = true;
        }
        void set_attach_bottom() {
            _anchor->dock_horizontal = false;
            _anchor->attach_top = false;
        }
        void set_attach_right() {
            _anchor->dock_horizontal = true;
            _anchor->attach_left = false;
        }
        void set_attach_left() {
            _anchor->dock_horizontal = true;
            _anchor->attach_left = true;
        }
        void update() {
            _anchor->anchor_element.update();
        }
    };
};
};