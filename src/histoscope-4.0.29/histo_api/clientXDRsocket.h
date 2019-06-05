/*******************************************************************************
*									       *
* clientXDRsocket.h -- Include file for clientXDRsocket.c.  Used for all XDR   *
*		 and socket operations.  This separate module is put into its  *
*		 own shareable image to hide the Multinet bcopy routine from   *
*		 the client program.  Thus even users of YBOS can use Histo-   *
*		 Scope.							       *
*									       *
* Copyright (c) 1993 Universities Research Association, Inc.		       *
* All rights reserved.							       *
* 									       *
* This material resulted from work developed under a Government Contract and   *
* is subject to the following license:  The Government retains a paid-up,      *
* nonexclusive, irrevocable worldwide license to reproduce, prepare derivative *
* works, perform publicly and display publicly by or for the Government,       *
* including the right to distribute to other Government contractors.  Neither  *
* the United States nor the United States Department of Energy, nor any of     *
* their employees, makes any warranty, express or implied, or assumes any      *
* legal liability or responsibility for the accuracy, completeness, or         *
* usefulness of any information, apparatus, product, or process disclosed, or  *
* represents that its use would not infringe privately owned rights.           *
*                                        				       *
* Fermilab Nirvana GUI Library						       *
* April 27, 1993							       *
*									       *
* Written by Joy Kyriakopulos						       *
*									       *
* Modification History:							       *
*									       *
*									       *
*******************************************************************************/

#ifdef VMS

extern bool_t hs_xdr_string(XDR *xdrs, char **string, int size);
extern bool_t hs_xdr_wrapstring(XDR *xdrs, char **string);
extern bool_t hs_xdr_int(XDR *xdrs, int *ival);
extern bool_t hs_xdr_float(XDR *xdrs, float *fval);
extern bool_t hs_xdr_array(XDR *xdrs, char **arrp, int sizep, int maxsize, 
				int elsize, xdrproc_t elproc);
extern void   hs_xdrmem_create(XDR *xdrs, char *buf, int size, enum xdr_op op);
extern void   hs_xdrstdio_create(XDR *xdrs, char *file, enum xdr_op op);
extern void   hs_xdr_destroy(XDR *xdrs);
extern int    hs_socket_read(unsigned short fd, char *buf, 
				unsigned int bytesToRead);
extern int    hs_socket_write(unsigned short fd, char *buf, 
				unsigned int bytesToWrite);
extern int    hs_socket_close(unsigned short VMS_Channel);
extern void   hs_socket_perror(char *msg);
extern int    hs_socket_errno(void);
extern int    hs_socket_ioctl(unsigned short fd, unsigned int request, 
				int *enableFlag);
extern int    hs_getsockopt(unsigned short fd, unsigned int level, unsigned int
				option, char *optVal, unsigned int optLen);
extern int    hs_setsockopt(unsigned short fd, unsigned int level, unsigned int
				option, char *optVal, unsigned int optLen);
extern int    hs_accept(unsigned short fd, struct sockaddr *addr, unsigned int
				*addrLen);
extern int    hs_socket(unsigned int addrFam, unsigned int type, unsigned int
				protocol);
extern int    hs_bind(unsigned short fd, struct sockaddr *name, unsigned int
				nameLen);
extern int    hs_getsockname(unsigned short fd, struct sockaddr *addr,
				unsigned int *addrLen);
extern int    hs_listen(unsigned short fd, unsigned int backlog);
extern char * hs_vms_errno_string(void);
extern unsigned long hs_ntohs(unsigned long val);
extern char * hs_gethostbyname(char *name);
extern int    hs_gethostname(char *name);

#endif /*VMS*/
