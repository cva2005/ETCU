#ifndef	_INC_IMP_H
#define	_INC_IMP_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct _incPrivate {
	int Number;
	int MaximumValue;
} incPrivate_t;

/**
 * ���������� ������������� ����� int
 * (������� �� ���������� ����������).
 * � ������ �������� ������������ �����������
 * CPU �� ������ ��������� 64 ����.
 */
#define INT64_SIZE 	8 /* ������ � ������ ���� INT_64 */
#define INT32_SIZE 	4 /* ������ � ������ ���� INT_32 */

#define INT_VAL_MAX(size) (size == INT64_SIZE) ? INT64_MAX : (\
		(size == INT32_SIZE) ? INT32_MAX : INT16_MAX)

int getNumber(void *incp);
void incrementNumber(void *incp);
void setMaximumValue(void *incp, int maximumValue);

#ifdef	__cplusplus
}
#endif

#endif	/* _INC_IMP_H */