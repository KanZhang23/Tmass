#ifndef JetCorrectionsInterface_hh
#define JetCorrectionsInterface_hh

#define N_JET_CORRECTORS 16

#ifdef __cplusplus
extern "C" {
#endif
    void init_generic_corrections(unsigned icorr, int level, int nvtx,
                                  int conesize, int version, int syscode,
                                  int nrun, int mode);

    void set_sys_total_correction(unsigned icorr, int systype);

    int corrector_level(unsigned icorr);

    int corrector_valid(unsigned icorr);

    /* Note that *emf is an input/output argument (appears to be unused) */
    float generic_correction_scale(unsigned icorr, float ptin,
                                   float *emf, float detectorEta);

    /* mode = 0 corresponds to "original" corrections, any other value
     * of "mode" corresponds to the "smoothed" uncertainty.
     */
    float generic_correction_uncertainty(unsigned icorr, float ptin,
                                         float detectorEta, unsigned mode);
#ifdef __cplusplus
}
#endif

#endif // JetCorrectionsInterface_hh
