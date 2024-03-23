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
}