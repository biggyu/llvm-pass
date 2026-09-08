// omp.h (minimal stub) - put in an include dir you control
#ifndef OMP_STUB_H
#define OMP_STUB_H
#ifdef __cplusplus
extern "C" {
#endif
double omp_get_wtime(void);
void omp_set_num_threads(int);
int omp_get_num_threads(void);
int omp_get_thread_num(void);
int omp_get_max_threads(void);
#ifdef __cplusplus
}
#endif
#endif