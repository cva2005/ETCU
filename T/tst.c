#include <stdlib.h>
#include <stdint.h>
#include "tst.h"
#include "tst_imp.h"

/**
 * Создает и инициализирует экземпляр класса Incrementor_t.
 * При успешном создании возвращает указатель на
 * созданный объект отличный от NULL
 */
Incrementor_t* createIncrementor(void) {
	Incrementor_t* inc = malloc(sizeof(Incrementor_t));
	if (inc != NULL) {
		inc->Private = malloc(sizeof(incPrivate_t));
		if (inc->Private == NULL) {
			free(inc);
		} else {
			incPrivate_t *priv = inc->Private;
			priv->Number = 0;
			priv->MaximumValue = INT_VAL_MAX(sizeof(int));
			inc->getNumber = getNumber;
			inc->incrementNumber = incrementNumber;
			inc->setMaximumValue = setMaximumValue;
		}
	}
	return inc;
}

/**
 * Удаляет объект Incrementor_t.
 */
void deleteIncrementor(Incrementor_t* ptr) {
	if (ptr == NULL) return;
	free(ptr->Private);
	free(ptr);
}

/**
 * Возвращает текущее число. В самом начале это ноль.
 * Возвращает код ошибки при ошибке указателя на объект.
 */
int getNumber(void *incp) {
	if (incp == NULL) return PTR_ERROR;
	return ((incPrivate_t *)(((Incrementor_t *)&incp)->Private))->Number;
}

/**
 * Увеличивает текущее число на один. После каждого вызова этого
 * метода getNumber() будет возвращать число на один больше.
 */
void incrementNumber(void *incp) {
	if (incp == NULL) return;
	int num = ((incPrivate_t *)(((Incrementor_t *)&incp)->Private))->Number;
	if (++num > ((incPrivate_t *)(((Incrementor_t *)&incp)->Private))->MaximumValue) num = 0;
	((incPrivate_t *)(((Incrementor_t *)&incp)->Private))->Number = num;
}

/**
 * Устанавливает максимальное значение текущего числа.
 * Когда при вызове incrementNumber() текущее число достигает
 * этого значения, оно обнуляется, т.е. getNumber() начинает
 * снова возвращать ноль, и снова один после следующего
 * вызова incrementNumber() и так далее.
 * По умолчанию максимум -- максимальное значение int.
 * Если при смене максимального значения число начинает
 * превышать максимальное значение, то число надо обнулить.
 * Нельзя позволять установить тут число меньше нуля.
 */
void setMaximumValue(void *incp, int maximumValue) {
	if (incp == NULL) return; /* ошибка указателя */
	if (maximumValue > INT_VAL_MAX(sizeof(int)))
		maximumValue = INT_VAL_MAX(sizeof(int));
	((incPrivate_t *)(((Incrementor_t *)&incp)->Private))->MaximumValue = maximumValue;
	int num = ((incPrivate_t *)(((Incrementor_t *)&incp)->Private))->Number;
	if (num > maximumValue) ((incPrivate_t *)(((Incrementor_t *)&incp)->Private))->Number = maximumValue;
}
