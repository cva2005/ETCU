/**
 * ����� Incrementor_t.
 */

#ifndef	_INC_H
#define	_INC_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct Incrementor {
	/**
	 * ���������� ������� �����. � ����� ������ ��� ����.
	 */
	int (*getNumber)(void *incp);
	/**
	 * ����������� ������� ����� �� ����. ����� ������� ������ �����
	 * ������ getNumber() ����� ���������� ����� �� ���� ������.
	 */
	void (*incrementNumber)(void *incp);
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
	void (*setMaximumValue)(void *incp, int maximumValue);
	void *Private;
} Incrementor_t;

#define PTR_ERROR	-1 /* ������ ��������� �� ������ Incrementor_t*/

/**
 * ������� � �������������� ��������� ������ Incrementor_t.
 * ��� �������� �������� ���������� ��������� ��
 * ��������� ������ �������� �� NULL
 */
Incrementor_t* createIncrementor(void);

/**
 * ������� ������ Incrementor_t.
 */
inline void deleteIncrementor(Incrementor_t* ptr);

#ifdef	__cplusplus
}
#endif

#endif	/* _INC_H */