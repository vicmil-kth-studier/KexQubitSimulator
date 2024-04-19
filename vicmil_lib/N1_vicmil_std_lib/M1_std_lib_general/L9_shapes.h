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
        T min_x() const {
            return x;
        }
        T max_x() const {
            return x + w;
        }
        T min_y() const {
            return y;
        }
        T max_y() const {
            return y + h;
        }
        T center_x() const {
            return x + (w / 2.0);
        }
        T center_y() const {
            return y + (h / 2.0);
        }
        bool is_inside_rect(T x_, T y_) const {
            if(!in_range<T>(x_, x, x + w)) {
                return false;
            }
            if(!in_range<T>(y_, y, y + h)) {
                return false;
            }
            return true;
        }
        std::string to_string() const {
            return "x: " + std::to_string(x) + 
                    "   y: " + std::to_string(y) + 
                    "   w: " + std::to_string(w) + 
                    "   h: " + std::to_string(h);
        }
        std::string to_string_min_max() const {
            return "min_x: " + std::to_string(min_x()) + 
                    "   min_y: " + std::to_string(min_y()) + 
                    "   max_x: " + std::to_string(max_x()) + 
                    "   max_y: " + std::to_string(max_y());
        }
        bool operator==(const RectT<T> other) const {
            return 
                x == other.x &&
                y == other.y &&
                w == other.w &&
                h == other.h;
        }
        bool operator!=(const RectT<T> other) const {
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

    /**
     * Align rectangle edges along x axis
     * @arg rect: The rectangle to move
     * @arg stationaty_rect: The other rectangle which is stationary
     * @arg rect_left_side: If the moving rectangle should align on left side(otherwise right)
     * @arg stationaty_rect_left_side: If the stationary rectangle should align on left side(otherwise right)
    */
    void align_rect_edge_x(RectT<int>& rect, RectT<int> stationaty_rect, bool rect_left_side, bool stationaty_rect_left_side) {
        if(stationaty_rect_left_side) {
            rect.x = stationaty_rect.x;
        }
        else {
            rect.x = stationaty_rect.x + stationaty_rect.w;
        }
        if(rect_left_side) {
            rect.x -= rect.w;
        }
    }
    /**
     * Align rectangle edges along x axis
     * @arg rect: The rectangle to move
     * @arg stationaty_rect: The other rectangle which is stationary
     * @arg rect_top_side: If the moving rectangle should align on top side(otherwise bottom)
     * @arg stationaty_rect_top_side: If the stationary rectangle should align on top side(otherwise bottom)
    */
    void align_rect_edge_y(RectT<int>& rect, RectT<int> stationaty_rect, bool rect_top_side, bool stationaty_rect_top_side) {
        if(stationaty_rect_top_side) {
            rect.y = stationaty_rect.y;
        }
        else {
            rect.y = stationaty_rect.y + stationaty_rect.h;
        }
        if(rect_top_side) {
            rect.y -= rect.h;
        }
    }

    /**
     * Cut of parts of the rectangle that are outside the boundry rectangle
    */
    void cut_rect_to_fit(RectT<int>& rect, const RectT<int> boundry) {
        int min_x = std::max(rect.min_x(), boundry.min_x());
        int max_x = std::min(rect.max_x(), boundry.max_x());
        int min_y = std::max(rect.min_y(), boundry.min_y());
        int max_y = std::min(rect.max_y(), boundry.max_y());
        rect.x = min_x;
        rect.y = min_y;
        rect.w = max_x - min_x;
        rect.h = max_y - min_y;
    }

    enum RectAlignment {
        LEFT_align_TOP,
        LEFT_align_MIDDLE_H,
        LEFT_align_BOTTOM,

        RIGHT_align_TOP,
        RIGHT_align_MIDDLE_H,
        RIGHT_align_BOTTOM,

        TOP_align_LEFT,
        TOP_align_MIDDLE_W,
        TOP_align_RIGHT,

        BOTTOM_align_LEFT,
        BOTTOM_align_MIDDLE_W,
        BOTTOM_align_RIGHT,

        MIDDLE,
        NO_ALIGNMENT,
        COPY_POSITION
    };
    void align_rect(RectT<int>& rect, const RectT<int> stationaty_rect, RectAlignment alignment) {
        if(alignment == RectAlignment::LEFT_align_TOP) {
            align_rect_edge_x(rect, stationaty_rect, true, true);
            align_rect_edge_y(rect, stationaty_rect, false, true);
        }
        else if(alignment == RectAlignment::LEFT_align_BOTTOM) {
            align_rect_edge_x(rect, stationaty_rect, true, true);
            align_rect_edge_y(rect, stationaty_rect, true, false);
        }
        else if(alignment == RectAlignment::RIGHT_align_TOP) {
            align_rect_edge_x(rect, stationaty_rect, false, false);
            align_rect_edge_y(rect, stationaty_rect, false, true);
        }
        else if(alignment == RectAlignment::RIGHT_align_BOTTOM) {
            align_rect_edge_x(rect, stationaty_rect, false, false);
            align_rect_edge_y(rect, stationaty_rect, true, false);
        }
        
        else if(alignment == RectAlignment::TOP_align_LEFT) {
            align_rect_edge_x(rect, stationaty_rect, false, true);
            align_rect_edge_y(rect, stationaty_rect, true, true);
        }
        else if(alignment == RectAlignment::TOP_align_RIGHT) {
            align_rect_edge_x(rect, stationaty_rect, true, false);
            align_rect_edge_y(rect, stationaty_rect, true, true);
        }
        else if(alignment == RectAlignment::BOTTOM_align_LEFT) {
            align_rect_edge_x(rect, stationaty_rect, false, true);
            align_rect_edge_y(rect, stationaty_rect, false, false);
        }
        else if(alignment == RectAlignment::BOTTOM_align_RIGHT) {
            align_rect_edge_x(rect, stationaty_rect, true, false);
            align_rect_edge_y(rect, stationaty_rect, false, false);
        }
        else if(alignment == RectAlignment::NO_ALIGNMENT) {
            // Do nothing
        }
        else if(alignment == RectAlignment::COPY_POSITION) {
            rect = stationaty_rect;
        }
        else {
            ThrowNotImplemented();
        }
    }

    /**
     * Tries to fit a list of rectangles into a bigger rectangle of a certain width and height
     * Returns true if it is succssfull!
     * Updates the positions of the rectangles so that they do not overlap
     * 
     * NOTE! Assumes the rectangles are already sorted on height in descending order
    */
    bool fit_rects_in_rect(std::vector<RectT<int>>& rects_height_descend, int width, int height) {
        if(rects_height_descend.size() == 0) {
            // Trivial case
            return true;
        }

        // Naive approach(Can be improved)
        // Tries to fit as many rects as possible on a certain y-level, if it fails, it moves to a new y-level
        int y_level = 0;
        int level_height = rects_height_descend[0].h;
        int x = 0;
        for(int i = 0; i < rects_height_descend.size(); i++) {
            if(rects_height_descend[i].w + x >= width) {
                // Cannot fit more rects on current y-level, move to new y-level
                y_level += level_height;
                level_height = rects_height_descend[i].h;
                x = 0;
            }
            if(rects_height_descend[i].h + y_level >= height) {
                // Height limit exceeded!
                return false;
            }
            rects_height_descend[i].x = x;
            rects_height_descend[i].y = y_level;
            x += rects_height_descend[i].w;
        }
        return true;
    }
    /**
     * Try to pack a list of rectangles into the smallest 2^n * 2^n grid as possible
     *   such that no rectangles overlap (no rotations)
     * Updates the rectangle positions and returns the grid size
     * 
     * Some names related to this problem
     *  - bin packing
     *  - rect packing
    */
    int rect_packing_pow2_sq(std::vector<RectT<int>>& rects) {
        // Lets start by calculating a minimum for the grid size
        //  - It has to be bigger or equal to the total area of the rectangles
        //  - It cannot be smaller than the biggest rectangle

        int area = 0;
        for(int i = 0; i < rects.size(); i++) {
            area += rects[i].h * rects[i].w;
        }

        int max_rect_size = 0;
        for(int i = 0; i < rects.size(); i++) {
            max_rect_size = std::max(rects[i].w, max_rect_size);
            max_rect_size = std::max(rects[i].h, max_rect_size);
        }
        unsigned int grid_size = upper_power_of_two(std::sqrt(area));
        grid_size = std::max(grid_size, upper_power_of_two(max_rect_size));
        

        // Lets sort the rectangles by height in descending order(while maintaining the index of the rectangles)
        std::vector<std::pair<int, int>> rects_height_and_index; // <height, rect index>
        for(int i = 0; i < rects.size(); i++) {
            rects_height_and_index.push_back(std::make_pair(rects[i].h, i));
        }
        std::sort(rects_height_and_index.begin(), 
            rects_height_and_index.end(), 
            [](const std::pair<int, int>& lhs, const std::pair<int, int>& rhs) {
                return lhs > rhs;
        });


        // Lets create a copy of rects but with the height sorted in descending order
        std::vector<RectT<int>> rects_height_descend;
        rects_height_descend.resize(rects.size());
        for(int i = 0; i < rects.size(); i++) {
            rects_height_descend[i] = rects[rects_height_and_index[i].second];
        }

        // Sanity check to see it is descending order
        for(int i = 0; i < std::min((int)rects_height_descend.size(), 10); i++) {
            if(i != 0) {
                // Make sure it is ascending!
                Assert(rects_height_descend[i-1].h >= rects_height_descend[i].h)
            }
            DebugExpr(rects_height_descend[i].to_string());
        }


        // Lets start trying to place the rectangles into the grid! And see if they fit
        //  If they don't fit, then we have to increase the size of the grid and try again...
        while(fit_rects_in_rect(rects_height_descend, grid_size, grid_size) == false) {
            grid_size = grid_size * 2;
        }

        // We found a match! Update the positions of the original rectangles
        for(int i = 0; i < rects.size(); i++) {
            rects[rects_height_and_index[i].second] = rects_height_descend[i];
        }
        return grid_size;
    }
}