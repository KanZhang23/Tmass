#include <cmath>

double pdf2pdf(int & beamIn, int & pdgIn, double & xBj, double & Q ) {
    double factor = 1.0 - exp(-Q/10000.)*pow(pdgIn/200,2);
    if( beamIn == 0 ) {
       return pow((1.-xBj),4)/xBj*factor;
    } else {
       return pow((1.-xBj),3)/xBj*factor;
    }
}
