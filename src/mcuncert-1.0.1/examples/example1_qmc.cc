#include <iostream>
#include <cmath> // for the absolute value function

#include "mcuncert/QMCUncertaintyCalculator.hh"
#include "CmdLine.hh"


// Van der Corput sequence
static double vdc(unsigned long n, double base = 2)
{
    double vdc = 0, denom = 1;
    while (n)
    {
        vdc += fmod(n, base) / (denom *= base);
        n /= base; // note: conversion from 'double' to 'int'
    }
    return vdc;
}

// Usage printer
static void print_usage(const char* progname);

// Defintion of the function to integrate (if changed, update the exact solution below)
long double f(long double x)
{
  return x*(x-1.0L); //sin(1.0L/(x+1e-16);
}
// Exact solution of the integral of f(x)
const long double exact_sol = -pow(6.0L,-1.0L);



int main(int argc, char** argv){

    // Parse input arguments
	CmdLine cmdline(argc,argv);

	if (argc == 1){
		print_usage(cmdline.progname());
		return 0;
	}

    unsigned mmax;
    long double tol;

    try {
        cmdline.optend();
        if (cmdline.argc() != 2)
            throw CmdLineError("Wrong number of command line arguments");
        cmdline >> tol >> mmax;
    }
    catch (const CmdLineError& e) {
        std::cerr << "Error in " << cmdline.progname() << ": "
        << e.str() << std::endl;
        print_usage(cmdline.progname());
        return 1;
    }

    
    // Main algorithm
    mcuncert::AbsUncertaintyCalculator * integration = new mcuncert::QMCUncertaintyCalculator;

    long double x = 0.0L;
    integration->addPoint(f(x));

    for (unsigned l = 0; l < mmax; ++l){
        for (unsigned long k = pow(2,l); k < pow(2,l+1); k++){ // We add the next 2^l points and check the error bound
            x = vdc(k,2);
            integration->addPoint(f(x));
        }
        if ((integration->meanUncertainty()) < tol && !(integration->nec_cond_fail())){ // Stopping condition (when the tolerance is met)
            std::cout << "Solution: " << integration->mean() << ".\n";
            std::cout << "Real error: " << std::abs(integration->mean()-exact_sol) << " < " << tol << ".\n";
            std::cout << "Number of points used: " << pow(2.0L,l+1.0L) << ".\n";
            break;
        }
        else if ((l == mmax-1) && !(integration->nec_cond_fail())){
            std::cout << "Maximum budget reached. Solution: " << integration->mean() << ".\n";
            std::cout << "Real error (not guaranteed): " << std::abs(integration->mean()-exact_sol) << ".\n";
        }
        else if (l == mmax-1){
            std::cout << "Integrand not in the cone. Solution was: " << integration->mean() << ".\n";
            std::cout << "Real error (not guaranteed): " << std::abs(integration->mean()-exact_sol) << ".\n";
        }
    }
    
    delete integration;
    return 0;
}


static void print_usage(const char* progname)
{
    std::cout << "\nUsage: " << progname << " tol mmax\n" << std::endl;
    std::cout << "The following arguments are required:" << std::endl;
    std::cout << "  tol    is the error tolerance (for instance, 1e-3)" << std::endl;
    std::cout << "  mmax   is the power such that 2^mmax is the maximum budget (depending on the machine, after 24 the user might run out of memory)." << std::endl;
    std::cout << std::endl;
}
