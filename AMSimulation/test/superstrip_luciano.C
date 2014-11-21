unsigned superstrip_luciano(unsigned lay, float phi, float eta,
                 const float unit_phi=M_PI/1600., const float unit_eta=4.4) {
    unsigned h = 0;
    //lay = compressLayer(lay);  // transform lay

    const unsigned n_phi = M_PI*2. / unit_phi + 1e-3;
    const unsigned n_eta = 2.2*2. / unit_eta + 1e-3;

    if (lay < 16) {
        phi += M_PI;  // -M_PI is the lowest phi value
        eta += 2.2;  // -2.2 is the lowest eta value

        phi = (phi < 0.) ? 0. : (phi >= M_PI*2. ? M_PI*2. - 1e-3 : phi);
        eta = (eta < 0.) ? 0. : (eta >= 2.2*2. ? 2.2*2. - 1e-3 : eta);

        h = (lay * n_eta * n_phi) + unsigned(eta / unit_eta) * n_phi + unsigned(phi / unit_phi);

    } else if (lay < 18) {
        h = lay * n_eta * n_phi;

    } else {
        h = 18 * n_eta * n_phi;

    }

    return h;
}
