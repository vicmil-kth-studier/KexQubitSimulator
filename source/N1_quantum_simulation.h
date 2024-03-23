#pragma once
#ifdef __EMSCRIPTEN__
#include "../vicmil_lib/N4_vicmil_emscripten/vicmil_emscripten.h"
#else
#include "../vicmil_lib/N3_vicmil_opengl/vicmil_opengl.h"
#endif


class QubitsState {
public:
    std::complex<double> v;
    static QubitsState from_prob_and_phase(double prob, double phase) {
        QubitsState new_state;
        double r = std::sqrt(prob);
        new_state.v = vicmil::exp_form_to_complex(r, phase);
        return new_state;
    }
    void get_prob_and_phase(double* prob, double* phase) const {
        double r;
        vicmil::complex_to_exp_form(v, &r, phase);
        *prob = r * r;
        return;
    }
    QubitsState set_prob(double to_prob) const {
        double _prob;
        double phase;
        get_prob_and_phase(&_prob, &phase);
        return from_prob_and_phase(to_prob, phase);
    }
    double get_prob() const {
        double prob;
        double _phase;
        get_prob_and_phase(&prob, &_phase);
        return prob;
    }

};


class QubitSystem {
    /*
        Each state index represents the prob and phase of each state
        s  q1 q2 q3
        0: 0  0  0
        1: 0  0  1
        2: 0  1  0
        3: 0  1  1
        4: 1  0  0
        5: 1  0  1
        6: 1  1  0
        7: 1  1  1
    */
    public:
    int _qubit_count;
    std::vector<QubitsState> _qubit_states = std::vector<QubitsState>();
    vicmil::RandomNumberGenerator _rand_gen;

    QubitSystem(int qubit_count) {
        _qubit_count = qubit_count;
        Assert(qubit_count < 24); // The system memory scales with 2^N, so 24 are many megabytes!
        _qubit_states.resize(std::pow(2, qubit_count));
        _qubit_states[0] = QubitsState::from_prob_and_phase(1, 0);
        _rand_gen = vicmil::RandomNumberGenerator();
    }

    int get_qubit_mask(int qubit_index) {
        return 1 << qubit_index;
    }

    bool is_qubit_enabled_in_state(int state_index, int qubit_index) {
        // lets do some bit operations
        int mask = get_qubit_mask(qubit_index);
        return mask & state_index;
    }

    bool measure(int qubit_num) {
        double r = _rand_gen.rand_between_0_and_1(); // Pick where in the probability distr we can find our value

        double sum = 0;
        bool qubit_val = false;
        for (int n=0; n<_qubit_states.size(); n++) {
            sum += _qubit_states[n].get_prob(); // cumulative probability
            if (sum >= r) {
                // We have found our value
                qubit_val = is_qubit_enabled_in_state(n, qubit_num);
                break;
            }
        }

        // Now we must collapse our vector to that value
        for (int n=0; n<_qubit_states.size(); n++) {
            // See if the value is the measured value
            if (qubit_val != is_qubit_enabled_in_state(n, qubit_num)) {
                _qubit_states[n] = QubitsState::from_prob_and_phase(0, 0); // Collapse the other values to 0
            }
        }

        // Normalize the vector again(so that the probabilities add up to 1)
        normalize();
        return qubit_val;
    }

    std::vector<bool> measure_all() {
        std::vector<bool> measurements;
        for (int n = 0; n < _qubit_count; n++) {
            bool measurement = measure(n);
            measurements.push_back(measurement);
        }
        return measurements;
    }
    

    // See what the total probability is(should always add up to 1)
    double get_total_probability() {
        // Calculate the total probability
        double prob_sum = 0;
        for (int n = 0; n < _qubit_states.size(); n++) {
            prob_sum += _qubit_states[n].get_prob(); // cumulative probability
        }

        return prob_sum;
    }
    

    void normalize() {
        double tot_prob = get_total_probability();

        // Normalize all the values
        for (int n = 0; n < _qubit_states.size(); n++) {
           double p = _qubit_states[n].get_prob();
           double new_prob = p / tot_prob;
           _qubit_states[n] = _qubit_states[n].set_prob(new_prob);
        }
    }


    void hadamar(int qubit_num) {
        // Now we must collapse our vector to that value
        for (int n = 0; n < _qubit_states.size(); n++) {
            // See if the value is the measured value
            if (!is_qubit_enabled_in_state(n, qubit_num)) {
                double n2 = n + get_qubit_mask(qubit_num); // Get the other qubit state
                QubitsState tmp_n = _qubit_states[n];
                QubitsState tmp_n2 = _qubit_states[n2];
                _qubit_states[n].v = (tmp_n.v + tmp_n2.v) * std::sqrt(0.5);
                _qubit_states[n2].v = (tmp_n.v - tmp_n2.v) * std::sqrt(0.5);
            }
        }
    }


    void phase_shift_pi_over_4(int qubit_num) {
        QubitsState phase_shift = QubitsState::from_prob_and_phase(1, vicmil::PI / 4);
        for (int n = 0; n < _qubit_states.size(); n++) {
            if (is_qubit_enabled_in_state(n, qubit_num)) {
                // apply phase shift if state has qubit set to 1
                _qubit_states[n].v = _qubit_states[n].v * phase_shift.v;
            }
        }
    }


    void cnot(int control_qubit_num, int target_qubit_num) {
        for (int n = 0; n < _qubit_states.size(); n++) {
            if (is_qubit_enabled_in_state(n, control_qubit_num)) {
                if (!is_qubit_enabled_in_state(n, target_qubit_num)) {
                    double n2 = n + get_qubit_mask(target_qubit_num); // Calculate the other state index;
                    QubitsState tmp_n = _qubit_states[n];
                    _qubit_states[n] = _qubit_states[n2];
                    _qubit_states[n2] = tmp_n;
                }
            }
        }
    }


    std::string state_vector_to_str() {
        std::string return_str = "";
        for (int n = 0; n < _qubit_count; n++) {
            return_str += " q" + std::to_string(_qubit_count - n - 1);
        }
        return_str += "\n";
        for (int state_ = 0; state_ < _qubit_states.size(); state_++) {
            for(int n = 0; n < _qubit_count; n++) {
                return_str += std::to_string((int)is_qubit_enabled_in_state(state_, _qubit_count - n - 1)) + "  ";
            }
            return_str += "|  ";
            double prob;
            double phase;
            _qubit_states[state_].get_prob_and_phase(&prob, &phase);
            return_str += "prob: " + std::to_string(prob*100.0) + "%     ";
            return_str += "phase: " + std::to_string(vicmil::radians_to_degrees(phase)) + "deg";
            return_str += "\n";
        }
        return return_str;
    }
    std::string state_vector_to_str_complex() {
        std::string return_str = "";
        for (int n = 0; n < _qubit_count; n++) {
            return_str += " q" + std::to_string(_qubit_count - n - 1);
        }
        return_str += "\n";
        for (int state_ = 0; state_ < _qubit_states.size(); state_++) {
            for(int n = 0; n < _qubit_count; n++) {
                return_str += std::to_string((int)is_qubit_enabled_in_state(state_, _qubit_count - n - 1)) + "  ";
            }
            return_str += "|  ";
            return_str += "real: " + std::to_string(_qubit_states[state_].v.real()) + "    ";
            return_str += "imag: " + std::to_string(_qubit_states[state_].v.imag());
            return_str += "\n";
        }
        return return_str;
    }
};