#pragma once
#include "defines.h"

/*
 * Memory layout - схема расположения памяти:
 * 
 * Динамический массив хранит метаданные перед элементами:
 * 
 * [ u64 capacity ]  - количество элементов, которое может вместить массив
 * [ u64 length   ]  - количество элементов, которые сейчас содержатся
 * [ u64 stride   ]  - размер каждого элемента в байтах
 * [ void* elements ] - указатель на начало данных элементов
 * 
 * Затем следуют сами элементы в непрерывном блоке памяти.
 * 
 * Визуально:
 * ┌─────────────┬──────────┬──────────┬─────────────────────────────────┐
 * │ capacity    │ length   │ stride   │ element 0 │ element 1 │ ...     │
 * │ (8 байт)    │ (8 байт) │ (8 байт) │ (stride)  │ (stride)  │         │
 * └─────────────┴──────────┴──────────┴─────────────────────────────────┘
 * ↑                                                            ↑
 * начало блока памяти                          данные элементов
 */

/*
 * Индексы для доступа к полям метаданных массива.
 * Используются как смещения от начала блока памяти.
 */
enum {
    DARRAY_CAPACITY,    // индекс поля capacity (смещение 0)
    DARRAY_LENGTH,      // индекс поля length   (смещение 8)
    DARRAY_STRIDE,      // индекс поля stride   (смещение 16)
    DARRAY_FIELD_LENGTH // количество полей (используется для проверок)
};

/*
 * Создаёт новый динамический массив.
 * Выделяет память для метаданных и элементов.
 * 
 * Параметры:
 *   length - начальная ёмкость массива
 *   stride - размер одного элемента в байтах
 * 
 * Возвращает:
 *   Указатель на первый элемент массива (НЕ на начало блока памяти!)
 *   NULL в случае ошибки выделения памяти
 */
KAPI void* _darray_create(u64 length, u64 stride);

/*
 * Уничтожает динамический массив.
 * Освобождает всю связанную с ним память.
 * 
 * Параметры:
 *   array - указатель на массив (указатель на первый элемент)
 */
KAPI void _darray_destroy(void* array);

/*
 * Получает значение поля метаданных массива.
 * 
 * Параметры:
 *   array - указатель на массив
 *   field - индекс поля (DARRAY_CAPACITY, DARRAY_LENGTH, DARRAY_STRIDE)
 * 
 * Возвращает:
 *   Значение поля в виде u64
 */
KAPI u64 _darray_field_get(void* array, u64 field);

/*
 * Устанавливает значение поля метаданных массива.
 * 
 * Параметры:
 *   array - указатель на массив
 *   field - индекс поля
 *   value - новое значение
 */
KAPI void _darray_field_set(void* array, u64 field, u64 value);

/*
 * Изменяет размер массива (увеличивает ёмкость).
 * Используется внутренне, когда нужно больше места.
 * 
 * Параметры:
 *   array - указатель на массив
 * 
 * Возвращает:
 *   Новый указатель на массив (может измениться при перераспределении)
 */
KAPI void* _darray_resize(void* array);

/*
 * Добавляет элемент в конец массива.
 * 
 * Параметры:
 *   array      - указатель на массив
 *   value_ptr  - указатель на значение для добавления
 * 
 * Возвращает:
 *   Новый указатель на массив (может измениться)
 */
KAPI void* _darray_push(void* array, const void* value_ptr);

/*
 * Удаляет элемент с конца массива.
 * 
 * Параметры:
 *   array    - указатель на массив
 *   dest     - указатель, куда скопировать удаляемый элемент (может быть NULL)
 */
KAPI void _darray_pop(void* array, void* dest);

/*
 * Удаляет элемент по указанному индексу.
 * 
 * Параметры:
 *   array  - указатель на массив
 *   index  - индекс удаляемого элемента
 *   dest   - указатель, куда скопировать удаляемый элемент (может быть NULL)
 * 
 * Возвращает:
 *   Новый указатель на массив (может измениться)
 */
KAPI void* _darray_pop_at(void* array, u64 index, void* dest);

/*
 * Вставляет элемент по указанному индексу.
 * 
 * Параметры:
 *   array      - указатель на массив
 *   index      - индекс для вставки
 *   value_ptr  - указатель на значение для вставки
 * 
 * Возвращает:
 *   Новый указатель на массив (может измениться)
 */
KAPI void* _darray_insert_at(void* array, u64 index, void* value_ptr);

/*
 * Константы конфигурации:
 */
#define DARRAY_DEFAULT_CAPACITY 1   // Начальная ёмкость по умолчанию
#define DARRAY_RESIZE_FACTOR 2      // Коэффициент увеличения при перераспределении

/*
 * Макросы для удобного использования с типами:
 */

// Создаёт динамический массив для указанного типа
#define darray_create(type) \
    _darray_create(DARRAY_DEFAULT_CAPACITY, sizeof(type))

// Создаёт динамический массив с указанной начальной ёмкостью
#define darray_reserve(type, capacity) \
    _darray_create(capacity, sizeof(type))

// Уничтожает массив
#define darray_destroy(array) _darray_destroy(array);

// Добавляет элемент в конец массива (с автоматическим определением типа)
#define darray_push(array, value)           \
    {                                       \
        typeof(value) temp = value;         \
        array = _darray_push(array, &temp); \
    }
/*
 * Примечание: можно было бы использовать __auto_type для temp,
 * но IntelliSense для VSCode помечает его как неизвестный тип.
 * typeof() работает хорошо, хотя оба являются расширениями GNU.
 */

// Удаляет элемент с конца массива
#define darray_pop(array, value_ptr) \
    _darray_pop(array, value_ptr)

// Вставляет элемент по указанному индексу
#define darray_insert_at(array, index, value)           \
    {                                                   \
        typeof(value) temp = value;                     \
        array = _darray_insert_at(array, index, &temp); \
    }

// Удаляет элемент по указанному индексу
#define darray_pop_at(array, index, value_ptr) \
    _darray_pop_at(array, index, value_ptr)

// Очищает массив (устанавливает length = 0, но не освобождает память)
#define darray_clear(array) \
    _darray_field_set(array, DARRAY_LENGTH, 0)

// Получает текущую ёмкость массива
#define darray_capacity(array) \
    _darray_field_get(array, DARRAY_CAPACITY)

// Получает текущее количество элементов
#define darray_length(array) \
    _darray_field_get(array, DARRAY_LENGTH)

// Получает размер элемента в байтах
#define darray_stride(array) \
    _darray_field_get(array, DARRAY_STRIDE)

// Устанавливает количество элементов (осторожно!)
#define darray_length_set(array, value) \
    _darray_field_set(array, DARRAY_LENGTH, value)