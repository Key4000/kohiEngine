#include "containers/darray.h"
#include "core/kmemory.h"
#include "core/logger.h"

/*
 * Создаёт новый динамический массив.
 * 
 * Параметры:
 *   length - начальная ёмкость массива (количество элементов)
 *   stride - размер одного элемента в байтах
 * 
 * Возвращает:
 *   Указатель на первый элемент массива (после заголовка)
 */
void* _darray_create(u64 length, u64 stride) {
    // Размер заголовка: 3 поля × 8 байт каждое = 24 байта
    u64 header_size = DARRAY_FIELD_LENGTH * sizeof(u64);
    
    // Размер данных: ёмкость × размер элемента
    u64 array_size = length * stride;
    
    // Выделяем память: заголовок + данные
    u64* new_array = kallocate(header_size + array_size, MEMORY_TAG_DARRAY);
    
    // Обнуляем всю выделенную память
    kset_memory(new_array, 0, header_size + array_size);
    
    // Заполняем поля заголовка:
    new_array[DARRAY_CAPACITY] = length;  // Максимальное количество элементов
    new_array[DARRAY_LENGTH] = 0;         // Текущее количество элементов (пустой)
    new_array[DARRAY_STRIDE] = stride;    // Размер элемента в байтах
    
    // Возвращаем указатель на начало данных (после заголовка)
    return (void*)(new_array + DARRAY_FIELD_LENGTH);
}

/*
 * Уничтожает динамический массив и освобождает память.
 * 
 * Параметры:
 *   array - указатель на массив (указатель на первый элемент данных)
 */
void _darray_destroy(void* array) {
    // Получаем указатель на начало блока памяти (заголовок)
    // Отступаем на DARRAY_FIELD_LENGTH (3) позиций назад
    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    
    // Вычисляем общий размер выделенной памяти
    u64 header_size = DARRAY_FIELD_LENGTH * sizeof(u64);
    u64 total_size = header_size + header[DARRAY_CAPACITY] * header[DARRAY_STRIDE];
    
    // Освобождаем всю память
    kfree(header, total_size, MEMORY_TAG_DARRAY);
}

/*
 * Получает значение поля из заголовка массива.
 * 
 * Параметры:
 *   array - указатель на массив (данные)
 *   field - индекс поля (DARRAY_CAPACITY, DARRAY_LENGTH, DARRAY_STRIDE)
 * 
 * Возвращает:
 *   Значение поля в виде u64
 */
u64 _darray_field_get(void* array, u64 field) {
    // Переходим к заголовку
    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    
    // Возвращаем значение поля
    return header[field];
}

/*
 * Устанавливает значение поля в заголовке массива.
 * 
 * Параметры:
 *   array - указатель на массив (данные)
 *   field - индекс поля
 *   value - новое значение
 */
void _darray_field_set(void* array, u64 field, u64 value) {
    // Переходим к заголовку
    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    
    // Устанавливаем значение
    header[field] = value;
}

/*
 * Увеличивает ёмкость массива в DARRAY_RESIZE_FACTOR (2) раза.
 * Копирует все существующие элементы в новый массив.
 * 
 * Параметры:
 *   array - указатель на текущий массив
 * 
 * Возвращает:
 *   Указатель на новый массив со увеличенной ёмкостью
 */
void* _darray_resize(void* array) {
    // Получаем текущие параметры массива
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);
    
    // Создаём новый массив с увеличенной ёмкостью (в 2 раза)
    void* temp = _darray_create(
        (DARRAY_RESIZE_FACTOR * darray_capacity(array)),  // Новая ёмкость
        stride);                                           // Размер элемента
    
    // Копируем все существующие элементы из старого массива
    kcopy_memory(temp, array, length * stride);
    
    // Восстанавливаем длину в новом массиве
    _darray_field_set(temp, DARRAY_LENGTH, length);
    
    // Уничтожаем старый массив
    _darray_destroy(array);
    
    // Возвращаем новый массив
    return temp;
}

/*
 * Добавляет новый элемент в конец массива.
 * При необходимости увеличивает ёмкость массива.
 * 
 * Параметры:
 *   array     - указатель на массив
 *   value_ptr - указатель на данные для добавления
 * 
 * Возвращает:
 *   Указатель на массив (может измениться после реаллокации)
 */
void* _darray_push(void* array, const void* value_ptr) {
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);
    
    // Проверяем, есть ли место в массиве
    if (length >= darray_capacity(array)) {
        // Увеличиваем ёмкость
        array = _darray_resize(array);
    }
    
    // Вычисляем адрес для нового элемента:
    // начало массива + (количество элементов × размер элемента)
    u64 addr = (u64)array;
    addr += (length * stride);
    
    // Копируем данные в вычисленное место
    kcopy_memory((void*)addr, value_ptr, stride);
    
    // Увеличиваем счётчик элементов
    _darray_field_set(array, DARRAY_LENGTH, length + 1);
    
    return array;
}

/*
 * Удаляет последний элемент массива.
 * Копирует удаляемый элемент в dest (если не NULL).
 * 
 * Параметры:
 *   array - указатель на массив
 *   dest  - указатель для сохранения удаляемого элемента (может быть NULL)
 */
void _darray_pop(void* array, void* dest) {
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);
    
    // Вычисляем адрес последнего элемента:
    // начало массива + ((длина - 1) × размер элемента)
    u64 addr = (u64)array;
    addr += ((length - 1) * stride);
    
    // Если нужно, копируем удаляемый элемент
    if (dest) {
        kcopy_memory(dest, (void*)addr, stride);
    }
    
    // Уменьшаем счётчик элементов
    _darray_field_set(array, DARRAY_LENGTH, length - 1);
    
    // ПРИМЕЧАНИЕ: память не освобождается, элемент просто помечается как удалённый
}

/*
 * Удаляет элемент по указанному индексу.
 * Сдвигает все последующие элементы на одну позицию влево.
 * 
 * Параметры:
 *   array - указатель на массив
 *   index - индекс удаляемого элемента
 *   dest  - указатель для сохранения удаляемого элемента (может быть NULL)
 * 
 * Возвращает:
 *   Указатель на массив
 */
void* _darray_pop_at(void* array, u64 index, void* dest) {
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);
    
    // Проверка выхода за границы
    if (index >= length) {
        KERROR("Index outside the bounds of this array! Length: %i, index: %index", length, index);
        return array;
    }
    
    u64 addr = (u64)array;
    
    // Если нужно, сохраняем удаляемый элемент
    if (dest) {
        kcopy_memory(dest, (void*)(addr + (index * stride)), stride);
    }
    
    // Если удаляем не последний элемент, нужно сдвинуть остальные
    if (index != length - 1) {
        // Копируем блок элементов [index+1 .. length-1] в [index .. length-2]
        kcopy_memory(
            (void*)(addr + (index * stride)),            // Куда: позиция удаляемого элемента
            (void*)(addr + ((index + 1) * stride)),      // Откуда: следующий элемент
            stride * (length - index));                  // Сколько: все элементы после удаляемого
    }
    
    // Уменьшаем счётчик элементов
    _darray_field_set(array, DARRAY_LENGTH, length - 1);
    
    return array;
}

/*
 * Вставляет элемент по указанному индексу.
 * Сдвигает все последующие элементы на одну позицию вправо.
 * При необходимости увеличивает ёмкость массива.
 * 
 * Параметры:
 *   array     - указатель на массив
 *   index     - индекс для вставки
 *   value_ptr - указатель на данные для вставки
 * 
 * Возвращает:
 *   Указатель на массив (может измениться после реаллокации)
 */
void* _darray_insert_at(void* array, u64 index, void* value_ptr) {
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);
    
    // Проверка выхода за границы
    if (index >= length) {
        KERROR("Index outside the bounds of this array! Length: %i, index: %index", length, index);
        return array;
    }
    
    // Проверяем, нужно ли увеличивать ёмкость
    if (length >= darray_capacity(array)) {
        array = _darray_resize(array);
    }
    
    u64 addr = (u64)array;
    
    // Если вставляем не в конец, нужно сдвинуть существующие элементы
    if (index != length - 1) {
        // Копируем блок элементов [index .. length-1] в [index+1 .. length]
        kcopy_memory(
            (void*)(addr + ((index + 1) * stride)),      // Куда: на позицию после вставки
            (void*)(addr + (index * stride)),            // Откуда: позиция вставки
            stride * (length - index));                  // Сколько: все элементы начиная с index
    }
    
    // Записываем новый элемент на освободившееся место
    kcopy_memory((void*)(addr + (index * stride)), value_ptr, stride);
    
    // Увеличиваем счётчик элементов
    _darray_field_set(array, DARRAY_LENGTH, length + 1);
    
    return array;
}


