#include "kmemory.h"
#include "core/logger.h"
#include "platform/platform.h"

// TODO: Custom string lib - в будущем заменить на свою реализацию строк
#include <string.h>
#include <stdio.h>

/*
 * Структура для хранения статистики использования памяти.
 */
struct memory_stats {
    u64 total_allocated;  //общий объём выделенной памяти в байтах 
    u64 tagged_allocations[MEMORY_TAG_MAX_TAGS];  //массив с объёмами памяти по каждому тегу 
};


/*
 * Массив строковых представлений для каждого тега памяти.
 * Используется для красивого вывода в get_memory_usage_str().
 * Пробелы в конце для выравнивания в консоли.
 */
static const char* memory_tag_strings[MEMORY_TAG_MAX_TAGS] = {
    "UNKNOWN    ",  // MEMORY_TAG_UNKNOWN
    "ARRAY      ",  // MEMORY_TAG_ARRAY
    "DARRAY     ",  // MEMORY_TAG_DARRAY
    "DICT       ",  // MEMORY_TAG_DICT
    "RING_QUEUE ",  // MEMORY_TAG_RING_QUEUE
    "BST        ",  // MEMORY_TAG_BST
    "STRING     ",  // MEMORY_TAG_STRING
    "APPLICATION",  // MEMORY_TAG_APPLICATION
    "JOB        ",  // MEMORY_TAG_JOB
    "TEXTURE    ",  // MEMORY_TAG_TEXTURE
    "MAT_INST   ",  // MEMORY_TAG_MATERIAL_INSTANCE (сокращённо)
    "RENDERER   ",  // MEMORY_TAG_RENDERER
    "GAME       ",  // MEMORY_TAG_GAME
    "TRANSFORM  ",  // MEMORY_TAG_TRANSFORM
    "ENTITY     ",  // MEMORY_TAG_ENTITY
    "ENTITY_NODE",  // MEMORY_TAG_ENTITY_NODE
    "SCENE      "   // MEMORY_TAG_SCENE
};

/*
 * Глобальная переменная со статистикой памяти.
 * static - видна только в этом файле.
 */
static struct memory_stats stats;

/*
 * Инициализирует систему управления памятью.
 * Обнуляет всю статистику, начиная отсчёт с нуля.
 */
void initialize_memory() {
    // Используем платформенную функцию для обнуления
    platform_zero_memory(&stats, sizeof(stats));
}

/*
 * Завершает работу системы управления памятью.
 * Пока пустая функция, но в будущем здесь можно:
 * 1. Проверить на утечки (если total_allocated != 0)
 * 2. Вывести финальный отчёт
 * 3. Освободить внутренние ресурсы
 */
void shutdown_memory() {}

/*
 * Выделяет блок памяти с заданным размером и тегом.
 * 
 * Параметры:
 *   size - размер в байтах
 *   tag  - тег для категоризации
 * 
 * Возвращает:
 *   Указатель на выделенную и обнулённую память
 * 
 * Особенности:
 *   1. Предупреждение при использовании UNKNOWN тега
 *   2. Обновление статистики
 *   3. Автоматическое обнуление памяти после выделения
 */
void* kallocate(u64 size, memory_tag tag) {
    // Предупреждение разработчику, что нужно указать конкретный тег
    if (tag == MEMORY_TAG_UNKNOWN) {
        KWARN("kallocate called using MEMORY_TAG_UNKNOWN. Re-class this allocation.");
    }
    
    // Обновляем статистику
    stats.total_allocated += size;
    stats.tagged_allocations[tag] += size;
    
    // TODO: Добавить поддержку выравнивания (alignment)
    // Выделяем память через платформенный слой
    void* block = platform_allocate(size, FALSE);
    
    // Автоматически обнуляем выделенную память
    platform_zero_memory(block, size);
    
    return block;
}

/*
 * Освобождает ранее выделенный блок памяти.
 * 
 * Параметры:
 *   block - указатель на блок
 *   size  - размер блока (должен совпадать с размером при выделении)
 *   tag   - тег (должен совпадать с тегом при выделении)
 * 
 * Особенности:
 *   1. Проверка тега (предупреждение для UNKNOWN)
 *   2. Обновление статистики
 *   3. Фактическое освобождение через платформенный слой
 */
void kfree(void* block, u64 size, memory_tag tag) {
    // Предупреждение для UNKNOWN тега
    if (tag == MEMORY_TAG_UNKNOWN) {
        KWARN("kfree called using MEMORY_TAG_UNKNOWN. Re-class this allocation.");
    }
    
    // Обновляем статистику (вычитаем освобождённую память)
    stats.total_allocated -= size;
    stats.tagged_allocations[tag] -= size;
    
    // TODO: Добавить поддержку выравнивания
    // Освобождаем память через платформенный слой
    platform_free(block, FALSE);
}

/*
 * Обнуляет блок памяти заданного размера.
 * Простая обёртка над platform_zero_memory.
 */
void* kzero_memory(void* block, u64 size) {
    return platform_zero_memory(block, size);
}

/*
 * Копирует данные из источника в назначение.
 * Обёртка над platform_copy_memory.
 */
void* kcopy_memory(void* dest, const void* source, u64 size) {
    return platform_copy_memory(dest, source, size);
}

/*
 * Заполняет блок памяти заданным значением.
 * Обёртка над platform_set_memory.
 */
void* kset_memory(void* dest, i32 value, u64 size) {
    return platform_set_memory(dest, value, size);
}

/*
 * Возвращает строку с подробной статистикой использования памяти.
 * Форматирует данные в читаемый вид с автоматическим выбором единиц измерения.
 * 
 * Формат вывода:
 * System memory use (tagged):
 *   UNKNOWN    : 0.00B
 *   ARRAY      : 1.50KB
 *   GAME       : 24.30MB
 *   TEXTURE    : 256.00MB
 * 
 * Особенности:
 *   1. Автоматический выбор единиц (B, KB, MB, GB)
 *   2. Выравнивание колонок
 *   3. Динамическое выделение строки (нужно освобождать)
 */
char* get_memory_usage_str() {
    // Константы для преобразования единиц
    const u64 gib = 1024 * 1024 * 1024;  // 1 гигабайт
    const u64 mib = 1024 * 1024;         // 1 мегабайт
    const u64 kib = 1024;                // 1 килобайт
    
    // Буфер для формирования строки (8000 байт должно хватить)
    char buffer[8000] = "System memory use (tagged):\n";
    u64 offset = strlen(buffer);  // Текущая позиция в буфере
    
    // Проходим по всем тегам и добавляем их статистику
    for (u32 i = 0; i < MEMORY_TAG_MAX_TAGS; ++i) {
        char unit[4] = "XiB";  // Шаблон для единиц: XiB, где X = B/K/M/G
        float amount = 1.0f;
        
        // Выбираем подходящую единицу измерения
        if (stats.tagged_allocations[i] >= gib) {
            unit[0] = 'G';  // Гигабайты
            amount = stats.tagged_allocations[i] / (float)gib;
        } else if (stats.tagged_allocations[i] >= mib) {
            unit[0] = 'M';  // Мегабайты
            amount = stats.tagged_allocations[i] / (float)mib;
        } else if (stats.tagged_allocations[i] >= kib) {
            unit[0] = 'K';  // Килобайты
            amount = stats.tagged_allocations[i] / (float)kib;
        } else {
            unit[0] = 'B';  // Байты
            unit[1] = 0;    // Обрезаем "iB", оставляем только "B"
            amount = (float)stats.tagged_allocations[i];
        }
        
        // Форматируем строку для текущего тега
        i32 length = snprintf(buffer + offset, 8000, 
                              "  %s: %.2f%s\n", 
                              memory_tag_strings[i], amount, unit);
        offset += length;  // Сдвигаем позицию для следующей записи
    }
    
    // Создаём копию строки для возврата (вызывающий должен освободить)
    char* out_string = _strdup(buffer);
    return out_string;
}
