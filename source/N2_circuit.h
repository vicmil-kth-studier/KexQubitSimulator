#include "N1_quantum_simulation.h"

namespace qubit_circuit {
class QuantumCircuitOperation {
public:
    virtual void apply_operation(QubitSystem& qubit_system){};
    virtual std::string get_qubit_str(int qubit_num){return ".."; }; // Should be two letters
    virtual int get_max_qubit() {return -1; }
};
class Hadamar: public QuantumCircuitOperation {
public:
    std::vector<int> qubits;
    Hadamar(std::vector<int> qubits_) {
        qubits = qubits_;
    }
    void apply_operation(QubitSystem& qubit_system) override {
        for(int i = 0; i < qubits.size(); i++) {
            int qubit_num = qubits[i];
            qubit_system.hadamar(qubit_num);
        }
    }
    std::string get_qubit_str(int qubit_num) override { // Should be two letters
        for(int i = 0; i < qubits.size(); i++) {
            if(qubits[i] == qubit_num) {
                return "H_";
            }
        }
        return "..";
    }
    int get_max_qubit() override {
        int max_qubit = -1;
        for(int i = 0; i < qubits.size(); i++) {
            if(max_qubit < qubits[i]) {
                qubits[i] = max_qubit;
            }
        }
        return max_qubit;
    }
};

class QuantumCircuit {
public:
    std::vector<QuantumCircuitOperation> operations;
    QuantumCircuit() {}
    void add_operation(QuantumCircuitOperation operation) {
        operations.push_back(operation);
    }
    /** Run the circuit on an existing quantum system */
    void run_circuit(QubitSystem& qubit_system) {
        for(int i = 0; i < operations.size(); i++) {
            operations[i].apply_operation(qubit_system);
        }
    }
    int get_qubit_count() {
        int qubit_count = 0;
        for(int j = 0; j < operations.size(); j++) {
            qubit_count = std::max(qubit_count, operations[j].get_max_qubit() + 1);
        }
        return qubit_count;
    }
};

class QuantumCircuitInterface {
    public:
    int min_qubit_count = 1;
    QuantumCircuit circuit;
    vicmil::app::TextConsole circuit_console = vicmil::app::TextConsole();
    vicmil::MouseState prev_mouse_state = vicmil::MouseState();
    vicmil::app::TextConsole my_output_console = vicmil::app::TextConsole();
    std::string info_str;
    int operation_index = 0;
    std::string to_string() {
        int qubit_count = std::max(circuit.get_qubit_count(), min_qubit_count);
        min_qubit_count = qubit_count;
        int circuit_length = circuit.operations.size();
        std::string output_str = "";
        for(int j = 0; j < qubit_count; j++) {
            output_str += "q" + std::to_string(j) + ":   ";
            for(int i = 0; i < circuit_length; i++) {
                output_str += "__";
                output_str += circuit.operations[i].get_qubit_str(j);
                output_str += "__";
            }
            output_str += "__..__"; // Add empty operation at the end
            output_str += "\n";
            output_str += "\n";
        }
        output_str += "+  -\n";
        output_str += ".., H_, T_, CT, CC, M_";
        return output_str;
    }
    void _char_pressed(int char_x, int char_y) {
        // Plus pressed
        if(char_x == 0 && char_y == min_qubit_count*2) {
            min_qubit_count += 1;
        }
        // Minus pressed
        if(char_x == 3 && char_y == min_qubit_count*2) {
            min_qubit_count -= 1;
            if(min_qubit_count < 1) {
                min_qubit_count = 1;
            }
        }
        // Operation Selection Pressed
        if(char_y == min_qubit_count*2+1) {
            int new_op_index_ = char_x / 3;
            if(new_op_index_ > 0 && new_op_index_<5) {
                operation_index = new_op_index_;
            }
        }
        // Operation Modification Pressed
        if(char_y < min_qubit_count*2 && char_x < (circuit.operations.size()+1)*6 + 6) {
            int qubit_num = char_y / 2;
            int operation_num = (char_x - 6) / 6;
            // Try and assign the operation
            // TODO
        }
    }
    void update(vicmil::MouseState mouse_state = vicmil::MouseState()) {
        double mouse_x = vicmil::x_pixel_to_opengl(mouse_state.x(), vicmil::app::globals::main_app->graphics_setup.width);
        double mouse_y = vicmil::y_pixel_to_opengl(mouse_state.y(), vicmil::app::globals::main_app->graphics_setup.height);
        if(mouse_state.left_button_is_pressed() && !prev_mouse_state.left_button_is_pressed()) {
            // New click with the mouse!
            // Determine on which charater the press is on
            double rel_mouse_x = mouse_x - circuit_console.min_x;
            double rel_mouse_y = circuit_console.max_y - mouse_y;

            double letter_width_with_spacing = vicmil::graphics_help::get_letter_width_with_spacing(circuit_console.letter_width);
            double letter_height_with_spacing = vicmil::graphics_help::get_letter_height_with_spacing(circuit_console.letter_width, vicmil::app::globals::main_app->graphics_setup.get_window_aspect_ratio());

            int char_x = rel_mouse_x / letter_width_with_spacing;
            int char_y = rel_mouse_y / letter_height_with_spacing;

            info_str = "x: " + std::to_string(char_x) + "    y: " + std::to_string(char_y);
            _char_pressed(char_x, char_y);
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