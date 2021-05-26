#include <string.h>
#include <stdbool.h>
#include "pid_r.h"

void pid_r_init (pid_r_instance *S) {
	memset(S->i, 0, 7u * sizeof(float32_t));
	S->Xi = NO_INTEGRAL_ZONE;
}
