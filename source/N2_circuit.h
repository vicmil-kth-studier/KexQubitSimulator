#include "N1_quantum_simulation.h"

namespace qubit_circuit {

const int standard_gate_index = 0;
const int hadamar_gate_index = 1;
const int phase_shift_gate_index = 2;
const int cnot_gate_index = 3;

const int MAX_QUBIT_COUNT = 10;

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
    QubitGateSetting("T_", phase_shift_gate_index, 0),
    QubitGateSetting("CC", cnot_gate_index, 0),
    QubitGateSetting("CT", cnot_gate_index, 1),
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
int run_phase_shift(QubitSystem& qubit_system, std::vector<int> qubit_settings) {
    for(int i = 0; i < qubit_settings.size(); i++) {
        int gate_index = gate_settings[qubit_settings[i]].gate_index;
        if(gate_index == phase_shift_gate_index) {
            qubit_system.phase_shift_pi_over_4(i);
        }
    }
    return 0;
}
int run_cnot(QubitSystem& qubit_system, std::vector<int> qubit_settings) {
    // Find the control and target qubits!
    std::vector<int> control_qubits = {};
    std::vector<int> target_qubits = {};
    for(int i = 0; i < qubit_settings.size(); i++) {
        int gate_index = gate_settings[qubit_settings[i]].gate_index;
        int index_in_gate = gate_settings[qubit_settings[i]].index_in_gate;
        if(gate_index == cnot_gate_index && index_in_gate == 0) {
            control_qubits.push_back(i);
        }
        else if(gate_index == cnot_gate_index && index_in_gate == 1) {
            target_qubits.push_back(i);
        }
    }
    if(control_qubits.size() != 1 || target_qubits.size() != 1) {
        return -1; // Not the right amount of qubits in gate
    }
    qubit_system.cnot(control_qubits[0], target_qubits[0]);
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

int perform_operation(QubitSystem& qubit_system, std::vector<int> qubit_settings) {
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
    if(gate_index == phase_shift_gate_index) {
        return run_phase_shift(qubit_system, qubit_settings);
    }
    if(gate_index == cnot_gate_index) {
        return run_cnot(qubit_system, qubit_settings);
    }
    return -1; // Unknown gate!
}

class QuantumCircuit {
    std::vector<std::vector<int>> qubit_settings = {}; // qubit_settings[operation_num][qubit_num]
public:
    void set_qubit_setting(int qubit_num, int operation_num, int setting) {
        while(operation_num >= qubit_settings.size()) {
            qubit_settings.push_back({});
        }
        while(qubit_num >= qubit_settings[operation_num].size()) {
            qubit_settings[operation_num].push_back(0);
        }
        qubit_settings[operation_num][qubit_num] = setting;
    }
    int get_qubit_setting(int qubit_num, int operation_num) {
        if(operation_num >= qubit_settings.size()) {
            return 0;
        }
        if(qubit_num >= qubit_settings[operation_num].size()) {
            return 0;
        }
        return qubit_settings[operation_num][qubit_num];
    }
    std::vector<int> get_operation_qubit_settings(int operation_num) {
        if(operation_num >= qubit_settings.size()) {
            return {};
        }
        return qubit_settings[operation_num];
    }
    void _slim_qubits() {
        for(int i = 0; i < qubit_settings.size(); i++) {
            while(qubit_settings[i].size() > 0) {
                if(qubit_settings[i].back() == 0) {
                    qubit_settings[i].resize(qubit_settings[i].size() - 1);
                }
                else {
                    break;
                }
            }
        }
    }
    void _slim_operations() {
        while(qubit_settings.size() > 0) {
            if(qubit_settings.back().size() == 0) {
                qubit_settings.resize(qubit_settings.size() - 1);
            }
            else {
                break;
            }
        }
    }
    void slim() {
        _slim_qubits();
        _slim_operations();
    }
    int get_qubit_count() {
        int max_qubit_count = 0;
        for(int i = 0; i < qubit_settings.size(); i++) {
            max_qubit_count = std::max(max_qubit_count, (int)qubit_settings[i].size());
        }
        return max_qubit_count;
    }
    int get_operations_count() {
        return qubit_settings.size();
    }
    int run_operation(QubitSystem& qubit_system, int operation_num) {
        return perform_operation(qubit_system, get_operation_qubit_settings(operation_num));
        // return 0; 
    }
};

class TextWindow {
public:
    vicmil::MouseState prev_mouse_state = vicmil::MouseState();
    vicmil::MouseState mouse_state = vicmil::MouseState();
    vicmil::app::TextConsole console = vicmil::app::TextConsole();
    bool _mouse_left_clicked() {
        return (mouse_state.left_button_is_pressed() && 
            !prev_mouse_state.left_button_is_pressed());
    }
    bool mouse_inside_window() {
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
    void get_mouse_char_pos(int* char_x, int* char_y) {
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
    void update(const vicmil::MouseState& new_mouse_state) {
        prev_mouse_state = mouse_state;
        mouse_state = new_mouse_state;
    }
};

class QuantumCircuitInterface {
    public:
    // Technical stuff
    int selected_qubit_setting = 1;
    QuantumCircuit circuit = QuantumCircuit();
    QubitSystem system_solution = QubitSystem(1);


    // Graphics stuffs
    int min_qubit_count = 1;
    TextWindow circuit_window = TextWindow();
    TextWindow output_window = TextWindow();
    std::string info_str;

    // Keep track of which lines corresponds to what
    int qubit_settings_start = 0;
    int qubit_plus_minus_start = 0;
    int qubit_settings_selection_start = 0;

    QuantumCircuitInterface() {
        run_system();
    }
    std::string to_string() {
        std::string output_str = "";
        int line_num = 0;
        output_str += "Quantum circuit:";
        output_str += "\n";
        line_num++;
        output_str += "\n";
        line_num++;

        qubit_settings_start = line_num;
        int qubit_count = std::max(circuit.get_qubit_count(), min_qubit_count);
        // qubit_count = min_qubit_count;
        min_qubit_count = qubit_count;
        int circuit_length = circuit.get_operations_count();
        for(int j = 0; j < qubit_count; j++) {
            output_str += "q" + std::to_string(j) + ":   ";
            for(int i = 0; i < circuit_length+1; i++) {
                output_str += "__";
                int qubit_setting = circuit.get_qubit_setting(j, i);
                output_str += gate_settings[qubit_setting].op_str; // TODO: fix
                output_str += "__";
            }
            line_num += 1;
            output_str += "\n";
            line_num += 1;
            output_str += "\n";
        }
        output_str += "\n";
        line_num++;
        output_str += "\n";
        line_num++;
        output_str += "Press on the gates in circuit to change them, ex ..";
        output_str += "\n";
        line_num++;
        output_str += "You can also change the gates already set!";
        output_str += "\n";
        line_num++;
        output_str += "Currently selected: " + gate_settings[selected_qubit_setting].op_str;
        output_str += "\n";
        line_num++;
        output_str += "\n";
        line_num++;
        output_str += "\n";
        line_num++;
        output_str += "\n";
        line_num++;
        output_str += "Press on the gate you want to use";
        output_str += "\n";
        line_num++;
        output_str += "\n";
        line_num++;

        qubit_settings_selection_start = line_num;

        // Add the settings at the end
        for(int i = 0; i < gate_settings.size(); i++) {
            if(i != 0) {
                output_str += ", ";
            }
            output_str += gate_settings[i].op_str;
        }
        output_str += "\n";
        line_num += 1;
        output_str += "\n";
        line_num += 1;
        output_str += "\n";
        line_num += 1;

        output_str += "Press on + and - to change qubit count";
        output_str += "\n";
        line_num++;

        qubit_plus_minus_start = line_num;
        output_str += " +   - \n";
        line_num += 1;

        return output_str;
    }
    void check_qubit_setting_pressed(int char_x, int char_y) {
        int max_x = (circuit.get_operations_count() + 1) * 6 + 6;
        int max_y = min_qubit_count*2-1 + qubit_settings_start;
        if(!vicmil::in_range(char_y, qubit_settings_start, max_y)) {return;}
        if(!vicmil::in_range(char_x, 6, max_x)) {return;}

        int operation_num = (char_x - 6) / 6;
        int qubit_num = (char_y - qubit_settings_start) / 2;
        DebugExpr(operation_num);
        DebugExpr(qubit_num);
        circuit.set_qubit_setting(qubit_num, operation_num, selected_qubit_setting);
    }
    void check_plus_pressed(int char_x, int char_y) {
        if(char_y != qubit_plus_minus_start) {return;}
        if(!vicmil::in_range(char_x, 0, 2)) {return;}
        min_qubit_count += 1;
        if(min_qubit_count > MAX_QUBIT_COUNT) { // Cap number of qubits
            min_qubit_count = MAX_QUBIT_COUNT;
        }
    }
    void check_minus_pressed(int char_x, int char_y) {
        if(char_y != qubit_plus_minus_start) {return;}
        if(!vicmil::in_range(char_x, 4, 6)) {return;}
        min_qubit_count -= 1;
        if(min_qubit_count < 1) {
            min_qubit_count = 1;
        }
    }
    void check_setting_selection_pressed(int char_x, int char_y) {
        if(char_y != qubit_settings_selection_start) {return;}
        if(!vicmil::in_range(char_x, 0, gate_settings.size()*4)) {return;}
        
        selected_qubit_setting = char_x / 4;
    }
    void run_system() {
        output_window.console.clear();

        if(min_qubit_count > MAX_QUBIT_COUNT) {
            min_qubit_count = MAX_QUBIT_COUNT;
        }

        // Rerun quantum system
        system_solution = QubitSystem(min_qubit_count);
        system_solution._rand_gen.set_seed(vicmil::Timer().start_ms); // Set random seed(can be improved!)
        system_solution._rand_gen.rand();
        system_solution._rand_gen.rand();
        system_solution._rand_gen.rand();
        system_solution._rand_gen.rand();
        system_solution._rand_gen.rand();
        system_solution._rand_gen.rand();

        for(int i = 0; i < circuit.get_operations_count(); i++) {
            int result = circuit.run_operation(system_solution, i);
            if(result != 0) {
                // Something went wrong!
                output_window.console.log("Something went wrong in operation: " + std::to_string(i + 1));
                return;
            }
        }

        output_window.console.log("Program successfully ran!\n");
        output_window.console.log("State vector:");
        output_window.console.log(system_solution.state_vector_to_str());

        output_window.console.log("\n Run instance id: " + std::to_string(vicmil::Timer().start_ms)); // As a way to see it is updating
        // Perform the measurement
        output_window.console.log("\nMeasurement:");
        std::string measurement_str = "";
        for(int i = 0; i < min_qubit_count; i++) {
            if(i != 0) {
                measurement_str += ",  ";
            }
            if(i != 0 && i%5 == 0) {
                measurement_str += "\n";
            }
            bool measurement = system_solution.measure(i);
            measurement_str += "q"  + std::to_string(i) + ": " + std::to_string((int)measurement);
        }
        output_window.console.log(measurement_str);
    }
    void update(vicmil::MouseState mouse_state = vicmil::MouseState()) {
        circuit_window.update(mouse_state);
        if(circuit_window._mouse_left_clicked() && circuit_window.mouse_inside_window()) {
            int char_x; int char_y;
            circuit_window.get_mouse_char_pos(&char_x, &char_y);
            info_str = "\nchar pressed. x: " + std::to_string(char_x) + "    y: " + std::to_string(char_y);

            check_qubit_setting_pressed(char_x, char_y);
            check_plus_pressed(char_x, char_y);
            check_minus_pressed(char_x, char_y);
            check_setting_selection_pressed(char_x, char_y);
            circuit.slim();

            run_system();
        }
    }
    void draw() {
        circuit_window.console.clear();
        circuit_window.console.log(to_string());
        circuit_window.console.log(info_str);
        circuit_window.console.log("\n Press like here to do a new measurement!");
        circuit_window.console.draw();

        output_window.console.draw();



        // TODO: draw quantum system solution
        //output_console.clear();
        //output_console.log("Hello world!");
        //output_console.draw();
    }
};
}