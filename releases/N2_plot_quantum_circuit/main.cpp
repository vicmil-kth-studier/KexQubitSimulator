#define USE_DEBUG
#define DEBUG_KEYWORDS "!vicmil_lib,/source,main(),init()" 
#include "../../source/quantum_computer_include.h"

using namespace vicmil;
using namespace qubit_circuit;

class QuantumCircuitInterface {
    public:
    // Technical stuff
    int selected_qubit_setting = 1;
    QuantumCircuit circuit = QuantumCircuit();
    QubitSystem system_solution = QubitSystem(1);


    // Graphics stuffs
    int min_qubit_count = 1;
    vicmil::app::TextConsole circuit_window = vicmil::app::TextConsole();
    vicmil::app::TextConsole output_window = vicmil::app::TextConsole();
    MouseTracker mouse_tracker;
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
        output_window.clear();

        if(min_qubit_count > MAX_QUBIT_COUNT) {
            min_qubit_count = MAX_QUBIT_COUNT;
        }

        // Rerun quantum system
        system_solution = QubitSystem(min_qubit_count);

        for(int i = 0; i < circuit.get_operations_count(); i++) {
            int result = circuit.run_operation(system_solution, i);
            if(result != 0) {
                // Something went wrong!
                output_window.log("Something went wrong in operation: " + std::to_string(i + 1));
                return;
            }
        }

        output_window.log("Program successfully ran!\n");
        output_window.log("State vector:");
        output_window.log(system_solution.state_vector_to_str());

        output_window.log("\n Run instance id: " + std::to_string((int)vicmil::get_time_since_epoch_ms())); // As a way to see it is updating
        // Perform the measurement
        output_window.log("\nMeasurement:");
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
        output_window.log(measurement_str);
    }
    void update(vicmil::MouseState mouse_state = vicmil::MouseState()) {
        mouse_tracker.update(mouse_state);
        if(mouse_tracker.mouse_left_clicked() && 
            vicmil::app::mouse_inside_rect(circuit_window.window_pos, mouse_state)) {
            double mouse_x = vicmil::app::get_mouse_pos_x(mouse_state);
            double mouse_y = vicmil::app::get_mouse_pos_y(mouse_state);
            int char_x = (mouse_x - circuit_window.window_pos.min_x()) / circuit_window.get_letter_width_with_spacing();
            int char_y = (circuit_window.window_pos.max_y() - mouse_y) / circuit_window.get_letter_height_with_spacing();

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
        circuit_window.clear();
        circuit_window.log(to_string());
        circuit_window.log(info_str);
        circuit_window.log("\n Press like here to do a new measurement!");
        circuit_window.draw();

        output_window.draw();
    }
};

FPSCounter fps_counter;
QuantumCircuitInterface quantum_circuit_interface;

const int FPS = 30;
int update_count = 0;

void render() {
    clear_screen();

    // Update camera
    vicmil::app::globals::main_app->camera.screen_aspect_ratio = 
        vicmil::app::globals::main_app->graphics_setup.get_window_aspect_ratio();

    fps_counter.record_frame();
    double fps = fps_counter.get_fps();
    std::string info_str = "fps: " + std::to_string(fps);

    MouseState mouse_state = MouseState();
    info_str += "   x: " + std::to_string(vicmil::app::get_mouse_pos_x(mouse_state));
    info_str += "   y: " + std::to_string(vicmil::app::get_mouse_pos_y(mouse_state));

    vicmil::app::draw2d_text(info_str, 0.0, 1.0, 0.02);

    //text_console.draw();

    quantum_circuit_interface.update();
    quantum_circuit_interface.draw();

    vicmil::app::TextConsole info_console;
    info_console.window_pos = Rect(-1, 0.7, 1, 1.7);
    info_console.log("The meaning of the gate symbols:");
    info_console.log("..: Empty position");
    info_console.log("H_: Hadamar");
    info_console.log("T_: Phase shift");
    info_console.log("CC: Control bit of CNOT");
    info_console.log("CT: Target bit of CNOT");
    info_console.draw();
}

// Runs at a fixed framerate
void game_loop() {
    update_count += 1;
    // Do nothing yet!
}

void init() {
    Print("C++ init!");
    vicmil::app::set_render_func(VoidFuncRef(render));
    vicmil::app::set_game_update_func(VoidFuncRef(game_loop));
    vicmil::app::set_game_updates_per_second(FPS);
    fps_counter = FPSCounter();

    quantum_circuit_interface.output_window.letter_width = 0.01;
    quantum_circuit_interface.output_window.window_pos = Rect(0, -1, 1, 1.7);
    quantum_circuit_interface.circuit_window.window_pos = Rect(-1, -1, 1, 1.7);
}


// Handle emscripten
void native_app() {
    while(true) {
        vicmil::app::app_loop_handler(vicmil::VoidFuncRef(init));
    }
}
void emscripten_update() {
    vicmil::app::app_loop_handler(vicmil::VoidFuncRef(init));
}
int main(int argc, char *argv[]) {
    Debug("Main!");
    #ifdef __EMSCRIPTEN__
        std::cout << "Emscripten!" << std::endl;
        emscripten_set_main_loop(emscripten_update, 0, 1);
    #else
        native_app();
    #endif

    return 0;
};

