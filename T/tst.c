#include <stdlib.h>
#include <stdint.h>
#include "tst.h"
#include "tst_imp.h"

/**
 * ������� � �������������� ��������� ������ Incrementor_t.
 * ��� �������� �������� ���������� ��������� ��
 * ��������� ������ �������� �� NULL
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
 * ������� ������ Incrementor_t.
 */
void deleteIncrementor(Incrementor_t* ptr) {
	if (ptr == NULL) return;
	free(ptr->Private);
	free(ptr);
}

/**
 * ���������� ������� �����. � ����� ������ ��� ����.
 * ���������� ��� ������ ��� ������ ��������� �� ������.
 */
int getNumber(void *incp) {
	if (incp == NULL) return PTR_ERROR;
	return ((incPrivate_t *)(((Incrementor_t *)&incp)->Private))->Number;
}

/**
 * ����������� ������� ����� �� ����. ����� ������� ������ �����
 * ������ getNumber() ����� ���������� ����� �� ���� ������.
 */
void incrementNumber(void *incp) {
	if (incp == NULL) return;
	int num = ((incPrivate_t *)(((Incrementor_t *)&incp)->Private))->Number;
	if (++num > ((incPrivate_t *)(((Incrementor_t *)&incp)->Private))->MaximumValue) num = 0;
	((incPrivate_t *)(((Incrementor_t *)&incp)->Private))->Number = num;
}

/**
 * ������������� ������������ �������� �������� �����.
 * ����� ��� ������ incrementNumber() ������� ����� ���������
 * ����� ��������, ��� ����������, �.�. getNumber() ��������
 * ����� ���������� ����, � ����� ���� ����� ����������
 * ������ incrementNumber() � ��� �����.
 * �� ��������� �������� -- ������������ �������� int.
 * ���� ��� ����� ������������� �������� ����� ��������
 * ��������� ������������ ��������, �� ����� ���� ��������.
 * ������ ��������� ���������� ��� ����� ������ ����.
 */
void setMaximumValue(void *incp, int maximumValue) {
	if (incp == NULL) return; /* ������ ��������� */
	if (maximumValue > INT_VAL_MAX(sizeof(int)))
		maximumValue = INT_VAL_MAX(sizeof(int));
	((incPrivate_t *)(((Incrementor_t *)&incp)->Private))->MaximumValue = maximumValue;
	int num = ((incPrivate_t *)(((Incrementor_t *)&incp)->Private))->Number;
	if (num > maximumValue) ((incPrivate_t *)(((Incrementor_t *)&incp)->Private))->Number = maximumValue;
}