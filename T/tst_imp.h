#ifndef	_TST_IMP_H
#define	_TST_IMP_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct _incPrivate {
	int Number;
	int MaximumValue;
} incPrivate_t;

/**
 * ¬ычисление максимального числа int
 * (зависит от аппаратной реализации).
 * ¬ данной редакции максимальна€ разр€дность
 * CPU не должна превышать 64 бита.
 */
#define INT64_SIZE 	8 /* размер в байтах типа INT_64 */
#define INT32_SIZE 	4 /* размер в байтах типа INT_32 */

#define INT_VAL_MAX(size) (size == INT64_SIZE) ? INT64_MAX : (\
		(size == INT32_SIZE) ? INT32_MAX : INT16_MAX)

int getNumber(void *incp);
void incrementNumber(void *incp);
void setMaximumValue(void *incp, int maximumValue);

#ifdef	__cplusplus
}
#endif

#endif	/* _TST_IMP_H */
