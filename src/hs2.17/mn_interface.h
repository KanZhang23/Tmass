#ifndef MN_INTERFACE_H
#define MN_INTERFACE_H

void mngetcommon_(int *NFCN, double *DCOVAR, int *NFCNMX);
void mnopen_(char *name, int *mode, int *lun, int *ierr, unsigned len);
void mnclos_(int *lun, int *ierr);
void mnsetnfcnmx_(int *NFCNMX);

#endif /* not MN_INTERFACE_H */
