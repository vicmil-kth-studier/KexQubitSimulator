#include "N1_quantum_simulation.h"

namespace qubit_circuit {

const int standard_gate_index = 0;
const int hadamar_gate_index = 1;

/**
 * Stores what operation a qubit is trying to apply
*/
struct QubitGateSetting {
    std::string op_str = ".."; // How the operation will be presented
    int gate_index = standard_gate_index;
    int index_in_gate = 0; // eg. control or target qubit
    QubitGateSetting(std::string op_str_, int gate_index_, int index_in_gate_) {
        op_str = op_str_;
        gate_index = gate_index_;
        index_in_gate = index_in_gate_;
    }
};
std::vector<QubitGateSetting> gate_settings = std::vector<QubitGateSetting>({
    QubitGateSetting("..", standard_gate_index, 0),
    QubitGateSetting("H_", hadamar_gate_index, 0),
    QubitGateSetting("T_", standard_gate_index, 0),
    QubitGateSetting("CC", standard_gate_index, 0),
    QubitGateSetting("CT", standard_gate_index, 1),
});

// Returns -1 in case of error
int run_hadamar(QubitSystem& qubit_system, std::vector<int> qubit_settings) {
    for(int i = 0; i < qubit_settings.size(); i++) {
        int gate_index = gate_settings[qubit_settings[i]].gate_index;
        if(gate_index == hadamar_gate_index) {
            qubit_system.hadamar(i);
        }
    }
    return 0;
}

// Get the op index from a list of qubits, returns -1 in case of conflict
int get_gate_index(std::vector<int> qubit_settings) {
    for(int i = 0; i < qubit_settings.size(); i++) {
        int check_gate_index = gate_settings[qubit_settings[i]].gate_index;
        if(check_gate_index != standard_gate_index) {
            // Assert all other qubits are either that op index or standard
            for(int j = 0; j < qubit_settings.size(); j++) {
                int gate_index = gate_settings[qubit_settings[j]].gate_index;
                if(gate_index != standard_gate_index && gate_index != check_gate_index) {
                   return -1; // Conflict!
                }
            }
            return check_gate_index;
        }
    }
    return standard_gate_index;
}

class Operation {
    std::vector<int> qubit_settings = {};
public:
    // Returns -1 in case of error
    int apply_operation(QubitSystem& qubit_system) {
        int gate_index = get_gate_index(qubit_settings);
        if(gate_index == -1) {
            return -1; // Conflicting operation
        }
        if(gate_index == standard_gate_index) {
            return 0; // No operations necessary!
        }
        if(gate_index == hadamar_gate_index) {
            return run_hadamar(qubit_system, qubit_settings);
        }
        return -1; // Unknown gate!
    }

    void set_qubit_setting(int qubit_num, int setting) {
        DebugExpr(setting);
        Assert(setting < gate_settings.size());
        int gate_index = gate_settings[setting].gate_index;
        DebugExpr(gate_index);
        if(gate_index == standard_gate_index) {
            Debug("gate index is standard!");
            // The standard settings is the same as doing nothing
            if(qubit_settings.size() > qubit_num) {
                qubit_settings[qubit_num] = 0;
            }
        }
        else {
            Debug("gate index is not standard!");
            while(qubit_settings.size() < qubit_num+1) {
                qubit_settings.push_back(standard_gate_index);
                Debug("increase size!");
            }
            DebugExpr(qubit_settings.size());
            qubit_settings[qubit_num] = setting;
            DebugExpr(qubit_settings[qubit_num]);
        }
    }
    int get_qubit_setting(int qubit_num) {
        if(qubit_settings.size() > qubit_num) {
            return qubit_settings[qubit_num];
        }
        return 0;
    }
    int get_max_qubit() {
        //Debug("get_max_qubit");
        int max_qubit = -1;
        for(int i = 0; i < qubit_settings.size(); i++) {
            // Find the max qubit that actually does somethings
            if(qubit_settings[i] != 0) {
                max_qubit = i;
            }
        }
        DebugExpr(max_qubit);
        //qubit_settings.resize(max_qubit + 1);
        return max_qubit;
    }
};

class QuantumCircuit {
    std::vector<Operation> operations = {};
public:
    QuantumCircuit() {}
    /** Run the circuit on an existing quantum system */
    void run_circuit(QubitSystem& qubit_system) {
        for(int i = 0; i < operations.size(); i++) {
            operations[i].apply_operation(qubit_system);
        }
    }
    void set_qubit_setting(int operation_index, int qubit_index, int qubit_gate_setting) {
        Debug("Add enough operations");
        DebugExpr(operation_index);
        while(operations.size() < operation_index + 1) {
            operations.push_back(Operation());
        }
        operations[operation_index].set_qubit_setting(qubit_index, qubit_gate_setting);
    }
    int get_qubit_setting(int operation_index, int qubit_index) {
        if(operations.size() < operation_index + 1) {
            return 0;
        }
        return operations[operation_index].get_qubit_setting(qubit_index);
    }
    int get_operations_count() {
        /*while(operations.size() > 0) {
            // Clean out any operations at the end not used
            if(operations.back().get_max_qubit() == -1) {
                operations.pop_back();
            }
        }*/
        return operations.size();
    }
    int get_max_qubit() { // Returns -1 if there are no operations with qubits
        int max_qubit = -1;
        for(int j = 0; j < operations.size(); j++) {
            max_qubit = std::max(max_qubit, operations[j].get_max_qubit());
        }
        return max_qubit;
    }
};

class QuantumCircuitInterface {
    public:
    int min_qubit_count = 1;
    QuantumCircuit circuit = QuantumCircuit();
    vicmil::MouseState prev_mouse_state = vicmil::MouseState();

    vicmil::app::TextConsole circuit_console = vicmil::app::TextConsole();
    vicmil::app::TextConsole my_output_console = vicmil::app::TextConsole();
    std::string info_str;

    int selected_qubit_setting = 1;

    bool _mouse_left_clicked(vicmil::MouseState& mouse_state) {
        return (mouse_state.left_button_is_pressed() && 
            !prev_mouse_state.left_button_is_pressed());
    }
    static bool mouse_inside_window(vicmil::MouseState& mouse_state, vicmil::app::TextConsole& console) {
        double mouse_x = vicmil::x_pixel_to_opengl(mouse_state.x(), vicmil::app::globals::main_app->graphics_setup.width);
        double mouse_y = vicmil::y_pixel_to_opengl(mouse_state.y(), vicmil::app::globals::main_app->graphics_setup.height);
        if(console.min_x <= mouse_x && 
            console.max_x >= mouse_x && 
            console.min_y <= mouse_y && 
            console.max_y >= mouse_y) {
            return true;
        }
        return false;
    }
    static void get_mouse_char_pos(vicmil::MouseState& mouse_state, vicmil::app::TextConsole& console, int* char_x, int* char_y) {
        double mouse_x = vicmil::x_pixel_to_opengl(mouse_state.x(), vicmil::app::globals::main_app->graphics_setup.width);
        double mouse_y = vicmil::y_pixel_to_opengl(mouse_state.y(), vicmil::app::globals::main_app->graphics_setup.height);
        double rel_mouse_x = mouse_x - console.min_x;
        double rel_mouse_y = console.max_y - mouse_y;

        double letter_width_with_spacing = vicmil::graphics_help::get_letter_width_with_spacing(console.letter_width);
        double letter_height_with_spacing = vicmil::graphics_help::get_letter_height_with_spacing(
            console.letter_width, 
            vicmil::app::globals::main_app->graphics_setup.get_window_aspect_ratio());

        *char_x = rel_mouse_x / letter_width_with_spacing;
        *char_y = rel_mouse_y / letter_height_with_spacing;
    }
    std::string to_string() {
        int qubit_count = std::max(circuit.get_max_qubit() + 1, min_qubit_count);
        // qubit_count = min_qubit_count;
        min_qubit_count = qubit_count;
        int circuit_length = circuit.get_operations_count();
        std::string output_str = "";
        for(int j = 0; j < qubit_count; j++) {
            output_str += "q" + std::to_string(j) + ":   ";
            for(int i = 0; i < circuit_length+1; i++) {
                output_str += "__";
                int qubit_setting = circuit.get_qubit_setting(i, j);
                output_str += gate_settings[qubit_setting].op_str; // TODO: fix
                output_str += "__";
            }
            output_str += "\n";
            output_str += "\n";
        }
        output_str += "+  -\n";

        // Add the settings at the end
        for(int i = 0; i < gate_settings.size(); i++) {
            if(i != 0) {
                output_str += ", ";
            }
            output_str += gate_settings[i].op_str;
        }
        output_str += "\n";

        // Show the current setting
        output_str += "[" + gate_settings[selected_qubit_setting].op_str + "]";

        return output_str;
    }
    void check_qubit_setting_pressed(int char_x, int char_y) {
        int max_x = (circuit.get_operations_count() + 1) * 6 + 6;
        int max_y = min_qubit_count*2-1;
        DebugExpr(max_x);
        DebugExpr(max_y);
        if(!vicmil::in_range(char_y, 0, max_y)) {return;}
        if(!vicmil::in_range(char_x, 6, max_x)) {return;}
        int operation_num = (char_x - 6) / 6;
        int qubit_num = char_y / 2;
        DebugExpr(operation_num);
        DebugExpr(qubit_num);
        circuit.set_qubit_setting(operation_num, qubit_num, selected_qubit_setting);
    }
    void check_plus_pressed(int char_x, int char_y) {
        if(!vicmil::in_range(char_y, min_qubit_count*2, min_qubit_count*2)) {return;}
        if(!vicmil::in_range(char_x, 0, 0)) {return;}
        min_qubit_count += 1;
    }
    void check_minus_pressed(int char_x, int char_y) {
        if(!vicmil::in_range(char_y, min_qubit_count*2, min_qubit_count*2)) {return;}
        if(!vicmil::in_range(char_x, 3, 3)) {return;}
        min_qubit_count -= 1;
        if(min_qubit_count < 1) {
            min_qubit_count = 1;
        }
    }
    void check_setting_selection_pressed(int char_x, int char_y) {
        if(!vicmil::in_range(char_y, min_qubit_count*2+1, min_qubit_count*2+1)) {return;}
        if(!vicmil::in_range(char_x, 0, gate_settings.size()*4)) {return;}
        
        selected_qubit_setting = char_x / 4;
    }
    void update(vicmil::MouseState mouse_state = vicmil::MouseState()) {
        if(_mouse_left_clicked(mouse_state) && mouse_inside_window(mouse_state, circuit_console)) {
            int char_x; int char_y;
            get_mouse_char_pos(mouse_state, circuit_console, &char_x, &char_y);
            info_str = "x: " + std::to_string(char_x) + "    y: " + std::to_string(char_y);

            check_qubit_setting_pressed(char_x, char_y);
            check_plus_pressed(char_x, char_y);
            check_minus_pressed(char_x, char_y);
            check_setting_selection_pressed(char_x, char_y);
        }
        prev_mouse_state = mouse_state;
    }
    void draw() {
        circuit_console.clear();
        circuit_console.log(to_string());
        circuit_console.log(info_str);
        circuit_console.draw();
        //output_console.clear();
        //output_console.log("Hello world!");
        //output_console.draw();
    }
};
}