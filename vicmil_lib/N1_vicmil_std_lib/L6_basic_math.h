#pragma once
#include "L5_time.h"
#include <complex.h>


namespace vicmil {
const double PI  = 3.141592653589793238463;

/**
 * Determine if a value is in range
 * Returns true if min_v <= v <= max_v
*/
inline bool in_range(int v, int min_v, int max_v) {
    if(v < min_v) {
        return false;
    }
    if(v > max_v) {
        return false;
    }
    return true;
}

inline bool is_power_of_two(unsigned int x) {
    return !(x == 0) && !(x & (x - 1));
}
inline bool is_power_of_two(int x) {
    return !(x == 0) && !(x & (x - 1));
}
    
unsigned int upper_power_of_two(unsigned int x)
{
    int power = 1;
    while(power < x) {
        power*=2;
    }
    return power;

}

double modulo(double val, double mod) {
    if(val > 0) {
        return val - ((int)(val / mod)) * mod;
    }
    else {
        return val - ((int)((val-0.0000001) / mod) - 1) * mod;
    }
}

double degrees_to_radians(const double deg) {
    return deg * 2.0 * PI / 360.0;
}

double radians_to_degrees(const double rad) {
    return rad * 360.0 / (PI * 2.0);
}

/** Returns the amount of overlap between two ranges: (min1, max1) and (min2, max2)
 * @param min1 The minimum value in range1
 * @param max1 The maximum value in range1
 * @param min2 The minimum value in range2
 * @param max2 The maximum value in range2
 * @return the amount of overlap between ranges (min1, max1) and (min2, max2)
*/
double get_overlap(double min1, double max1, double min2, double max2) {
    // Scenarios
    // where (smaller --> greater)
    // (min1, max1, min2, max2)    -no overlap
    // (min1, min2, max1, max2)    - overlap
    // (min1, min2, max2, max1)    - overlap

    // (min2, max2, min1, max1)    -no overlap
    // (min2, min1, max2, max1)    - overlap
    // (min2, min1, max1, max2)    - overlap

    // How much do we have to move 1 left to avoid overlap?
    double left = max1 - min2;

    // How much do we have to move 1 right to avoid overlap?
    double right = max2 - min1;


    return std::min(left, right); // Negative result means there is no overlap!
}


// Computes r * e ^ (i * phi) = r(cos(phi) + i * sin(phi))
std::complex<double> exp_form_to_complex(double r, double phi) {
    return r * std::exp(phi * std::complex<double>(0, 1));
}

/**
Convert
real + i * imag
 ->
r * e ^ (i * phi)

returns r: the r value in  r * e ^ (i * phi)
returns phi: the phi value in  r * e ^ (i * phi), in the interval [0, 2PI)


Derivations
-----------
r * e ^ (i * phi) = r * (cos(phi) + i sin(phi))
-> 
real = r * cos(phi)
imag = r * sin(phi)

imag / real = tan(phi)
phi = atan(imag / real)
r = real / cos(phi)
*/
void complex_to_exp_form(std::complex<double> v, double* r, double* phi) {
    DisableLogging;
    double real = v.real(); // = r * cos(phi)
    double imag = v.imag(); // = r * sin(phi)
    DebugExpr(real);
    DebugExpr(imag);

    if(abs(real) < 0.000001 && abs(imag) < 0.000001) {
        *r = 0;
        *phi = 0;
        return;
    }
    if(abs(real) < 0.000001 && imag > 0) {
        *r = imag;
        *phi = PI/2.0;
        return;
    }
    if(abs(real) < 0.000001 && imag < 0) {
        *r = -imag;
        *phi = 3 * PI / 2.0;
        return;
    }
        
    double d = std::atan(imag / real); // Returns phase in interval [-PI/2, PI/2]
    DebugExpr(d);
    // We have to flip it 180deg if cosine(phase) is negative, eg. real part is negative
    if(real <= 0) {
        d += vicmil::PI;
    }
    *phi = vicmil::modulo(d+0.0000001, 2*vicmil::PI); // Make sure it is in interval [0, 2PI]
    *r = real / std::cos(*phi);
    return;
}

void TEST_complex_to_exp_form() {
    double phi = 0;
    double r = 0.5;
    double inc = vicmil::PI / 4;
    while(phi < 2 * vicmil::PI) {
        DebugExpr(phi);
        std::complex<double> v = exp_form_to_complex(r, phi);
        double r_new;
        double phi_new;
        complex_to_exp_form(v, &r_new, &phi_new);
        DebugExpr(phi_new);
        DebugExpr(r_new);
        assert(abs(phi_new - phi) < 0.0001);
        assert(abs(r_new - r) < 0.0001);
        phi += inc;
    }

    double r_new;
    double phi_new;
    complex_to_exp_form(0, &r_new, &phi_new);
    assert(abs(phi_new - 0) < 0.0001);
    assert(abs(r_new - 0) < 0.0001);
}
AddTest(TEST_complex_to_exp_form);
}