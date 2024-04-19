#include "L4_gpu_setup_general.h"
#include <list>

namespace vicmil {
/*struct DropDownWidget {
    // Setup
    vicmil::WidgetRect _dropdown_button;
    std::map<std::string, std::vector<std::string>> _option_organization = {}:

    DropDown() {}
    DropDown(vicmil::WidgetRect dropdown_button_) {
        _dropdown_button = dropdown_button_;
        _option_organization[""] = {};
        reset_selected();
    } 

    // Dynamic(updates during runtime)
    struct OptionSelection {
        std::string option_name;
        vicmil::__layout__::WidgetRectWithSubwidgets parent_widget;
        std::vector<std::string> suboptions = {};
        std::vector<vicmil::__layout__::WidgetRectWithSubwidgets> suboption_widgets = {};
        _OptionSelection(
                std::vector<std::string> suboptions_, 
                vicmil::__layout__::WidgetRectWithSubwidgets parent_widget_, // The widget representing this button
                std::string option_name_) 
            {
            suboptions = suboptions_;
            parent_widget = parent_widget_;
            option_name = option_name_;
            // TODO! Attach widgets by aligning them top-left OR left-top  
            for(int i = 0; i < suboptions.size(); i++) {
                auto new_ = vicmil::__layout__::WidgetRectWithSubwidgets(parent_widget.widget(), parent_widget.layout_copy());
                suboption_widgets.push_back(new_);
            }
        } 
        _OptionSelection() {}
    }; */
    /**
     * What options are selected
    */
    //std::vector<OptionSelection> selected_options = {};

    /**
     * Fetches the most recent selected options
    */
    /*void _update_selected_options() {
        while(selected_options.size() > 0) {
            if(selected_options.back().parent_layout.widget.is_selected()) {
                return;
            }
            for(int i = 0; i < selected_options.back().suboption_widgets.size(); i++) {
                std::string option_name = selected_options.back().suboptions[i];
                vicmil::__layout__::WidgetRect option_widget = selected_options.back().suboption_widgets[i];
                if(option_widget.widget().is_selected()) {
                    if(_option_organization.count(option_name) > 0) {
                        selected_options.push_back(OptionSelection(_option_organization[option_name], option_widget, option_name));
                    }
                    else {
                        selected_options.push_back(OptionSelection({}, option_widget, option_name));
                    }
                }
            }
            // If neither the option or its children are selected, it is deselected!
            selected_options.back().parent_widget.remove_subwidgets();
            selected_options.pop_back();
        }
        return;
    }*/
    /**
     * Get what options are selected as a list
    */
    /*std::vector<std::string> get_selected_options() {
        _update_selected_options();
        std::vector<std::string> selected_options_str_ = {};
        for(int i = 0; i < selected_options.size() i++) {
            selected_options_str_.push_back(selected_options[i].option_name);
        }
        return selected_options_str_;
    }*/
    /**
     * Reset what options have been selected
    */
    /*void reset_selected() {
        selected_options = {_OptionSelection(_option_organization[""], _dropdown_button, "")};
    }*/
    /**
     * Determines if it is selected by the mouse
    */
    /*bool is_selected() {
        _update_selected_options();
        return selected_options.size() != 0;
    }*/
    // The empty string is the parent option for everything
    /*void add_options(std::string parent_option, std::vector<std::string> child_options) {
        Assert(selected_options.size() < 2);
        option_organization[parent_option] = child_options;
        reset_selected();
    }
};*/

/*class Button: public Widget {
};
class Slider: Widget {

};
class Switch: Widget {

};
class TextBox {

};
class DropDown {

};
class Draggable { // For things you can drag on screen 

};
class ScatterPlot { // Plot where you can select data points

};*/
}