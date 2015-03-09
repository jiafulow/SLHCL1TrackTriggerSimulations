#include <cmath>
//#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


// Default parameters: SF1.0, NZ1.0
unsigned superstrip_rational(unsigned lay, float phi, float z,
                             const float scale_phi=1.0, const float divide_z=1.0) {
    unsigned h = 0;
    //lay = compressLayer(lay);  // transform lay

    static const float units_phi[6] = {0.00381, 0.00439, 0.00459, 0.00485, 0.00523, 0.00575};
    static const float edges_z[6*2] = {-6.712700,26.979900, -6.779690,36.704800, -5.254170,47.751099, -9.531830,59.410301, -9.531830,78.737198, -9.531830,88.993500};

    float unit_phi = units_phi[0] * scale_phi;                    // innermost has the smallest unit
    float unit_z   = (edges_z[2*5+1] - edges_z[2*5]) / divide_z;  // outermost has the largest unit
    //if (unit_phi > M_PI*2.)  unit_phi = M_PI*2.;
    //if (unit_z   > 2.2*2. )  unit_z   = 2.2*2.;
    int n_phi = floor(M_PI*2. / unit_phi + 0.5);
    int n_z   = divide_z;
    assert(n_phi > 0 && n_z > 0);

    if (lay < 6) {  // barrel only
        unit_phi = units_phi[lay] * scale_phi;
        unit_z   = (edges_z[2*lay+1] - edges_z[2*lay]) / divide_z;
        //if (unit_phi > M_PI*2.)  unit_phi = M_PI*2.;
        //if (unit_z   > 2.2*2. )  unit_z   = 2.2*2.;

        phi -= -M_PI;           // -M_PI is the lowest phi value
        z   -= edges_z[2*lay];  // lowest z value in trigger tower

        int i_phi = floor(phi / unit_phi);
        int i_z   = floor(z   / unit_z);
        i_phi = (i_phi < 0) ? 0 : (i_phi >= n_phi) ? (n_phi - 1) : i_phi;  // proper range
        i_z   = (i_z   < 0) ? 0 : (i_z   >= n_z  ) ? (n_z   - 1) : i_z  ;  // proper range

        h = (lay * n_z * n_phi) + i_z * n_phi + i_phi;

    } else if (lay < 18) {
        h = lay * n_z * n_phi;

    } else {
        h = 18 * n_z * n_phi;

    }

    return h;
}
