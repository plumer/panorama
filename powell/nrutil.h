#ifndef _nrutil_already_included
#define _nrutil_already_included

#ifdef __cplusplus
extern "C" {
#endif

float *vector(int,int);
float **matrix(int,int,int,int);
float **convert_matrix();
double *dvector();
double **dmatrix();
int *ivector();
int **imatrix();
float **submatrix();
void free_vector(float *,int,int);
void free_dvector();
void free_ivector();
void free_matrix(float **,int,int,int,int);
void free_dmatrix();
void free_imatrix();
void free_submatrix();
void free_convert_matrix();
void nrerror(char []);

#ifdef __cplusplus
};
#endif

#endif
