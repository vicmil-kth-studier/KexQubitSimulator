#include "N1_quantum_simulation.h"

struct qubits_operation {
    int16_t op_type = 0; // The operation type(hadamar, phase shift etc.)
                         //    // op_type=0 means no operation
    int16_t qubit_type = 0; // Some circuits have different qubit types(eg. cnot with control and target)
};
enum QUBIT_OP {
    nothing = 0,
    hadamard,
    phase_shift,
    measure,
    cnot    // (0 is target, 1 are control)
};

struct QuantumCircuitOperation {
    std::vector<qubits_operation> operations; // What operation to apply to each qubit
};

/**
 * Qubit mappings are really important!
 * Say we have a curcuit with 3 qubits that we want to apply on a system with many more qubits
 *  then we need a way to specify which qubits we will work on in the larger system.
*/
class QubitMapping {
    std::vector<int> v;
};

class QuantumCircuit {
    std::vector<QuantumCircuitOperation> qubit_circuit;
    int qubit_count;
    int circuit_length;
    public:
    QuantumCircuit(int qubit_count_, int circuit_length_) {
        qubit_count = qubit_count_;
        circuit_length = circuit_length_;

        qubit_circuit.resize(qubit_count);
        for(int i = 0; i < qubit_count; i++) {
            qubit_circuit[i].operations.resize(circuit_length);
        }
    }
    void run_operation(QubitSystem& qubit_system, int qubit_circuit_index, QubitMapping qubit_mapping) {
        // This is the operation that will be applied to all qubits(even though some may not be affected)
        QuantumCircuitOperation operation = qubit_circuit[qubit_circuit_index];

        // Determine which qubits are affected!
        std::vector<int> affected_qubits;
        for(int i = 0; i < qubit_count; i++) {
            if(operation.operations[i].op_type != 0) { 
                affected_qubits.push_back(i);
            }
        }

        if(affected_qubits.size() == 0) {
            // No affected qubits, so we are done!
            return;
        }

        // Assert that all affected qubits are part of the same operation
        int16_t optype = operation.operations[affected_qubits[0]].op_type;
        for(int i = 1; i < affected_qubits.size(); i++) {
            assert(operation.operations[i].op_type == optype);
        }

        // Apply the qubit mapping


        // Handle depending on optype

    }
    /** Run the circuit on an existing quantum system */
    void run_circuit(QubitSystem& qubit_system, QubitMapping qubit_mapping) {
        for(int i = 0; i < circuit_length; i++) {
            run_operation(qubit_system, i, qubit_mapping);
        }
    }
};