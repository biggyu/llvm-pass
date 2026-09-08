#include <stddef.h>
#include <sys/time.h>
double omp_get_wtime(void) {
    struct timeval tv; gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}
void omp_set_num_threads(int n) { (void)n; }
int omp_get_num_threads(void) { return 1; }
int omp_get_thread_num(void) { return 0; }
int omp_get_max_threads(void) { return 1; }