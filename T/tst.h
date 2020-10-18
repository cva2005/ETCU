#ifndef	_TST_H
#define	_TST_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct Incrementor {
	/**
	 * Возвращает текущее число. В самом начале это ноль.
	 */
	int (*getNumber)(void *incp);
	/**
	 * Увеличивает текущее число на один. После каждого вызова этого
	 * метода getNumber() будет возвращать число на один больше.
	 */
	void (*incrementNumber)(void *incp);
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
	void (*setMaximumValue)(void *incp, int maximumValue);
	void *Private;
} Incrementor_t;

#define PTR_ERROR	-1 /* ошибка указателя на объект Incrementor_t*/

/**
 * Создает и инициализирует экземпляр класса Incrementor_t.
 * При успешном создании возвращает указатель на
 * созданный объект отличный от NULL
 */
Incrementor_t* createIncrementor(void);

/**
 * Удаляет объект Incrementor_t.
 */
inline void deleteIncrementor(Incrementor_t* ptr);

#ifdef	__cplusplus
}
#endif

#endif	/* _TST_H */
