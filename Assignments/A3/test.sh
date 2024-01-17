echo Checking that running galsim executable works
./galsim 1000 ../input_data/ellipse_N_01000.gal 200 1e-5 0 || exit 1
