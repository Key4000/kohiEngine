#include "core/event.h"
#include "core/kmemory.h"
#include "containers/darray.h"

/*
 * Структура зарегистрированного события.
 * Хранит связку "слушатель + функция-обработчик".
 */
typedef struct registered_event {
    void* listener;        // Указатель на объект-слушатель (может быть NULL)
    PFN_on_event callback; // Функция-обработчик события
} registered_event;

/*
 * Запись для кода события.
 * Содержит динамический массив зарегистрированных обработчиков.
 */
typedef struct event_code_entry {
    registered_event* events; // Динамический массив registered_event
} event_code_entry;

// Максимальное количество кодов событий (16384 = 2^14)
#define MAX_MESSAGE_CODES 16384

/*
 * Состояние системы событий.
 * Использует таблицу поиска (lookup table) для быстрого доступа по коду события.
 */
typedef struct event_system_state {
    // Таблица зарегистрированных событий, индексированная по коду события
    event_code_entry registered[MAX_MESSAGE_CODES];
} event_system_state;

/*
 * Внутреннее состояние системы событий.
 * static - ограничивает видимость пределами этого файла.
 */
static b8 is_initialized = FALSE;      // Флаг инициализации системы
static event_system_state state;       // Экземпляр состояния системы

/*
 * Инициализирует систему событий.
 * Должна быть вызвана перед использованием любых других функций.
 * 
 * Возвращает:
 *   TRUE  - инициализация успешна
 *   FALSE - уже инициализирована или ошибка
 */
b8 event_initialize() {
    // Защита от повторной инициализации
    if (is_initialized == TRUE) {
        return FALSE;
    }
    
    // Сначала сбрасываем флаг (на случай ошибки)
    is_initialized = FALSE;
    
    // Обнуляем всё состояние системы
    kzero_memory(&state, sizeof(state));
    
    // Устанавливаем флаг успешной инициализации
    is_initialized = TRUE;
    
    return TRUE;
}

/*
 * Завершает работу системы событий.
 * Освобождает все динамически выделенные ресурсы.
 */
void event_shutdown() {
    // Освобождаем массивы событий для каждого кода
    for(u16 i = 0; i < MAX_MESSAGE_CODES; ++i){
        // Проверяем, есть ли зарегистрированные события для этого кода
        if(state.registered[i].events != 0) {
            // Уничтожаем динамический массив (освобождаем память)
            darray_destroy(state.registered[i].events);
            // Обнуляем указатель (защита от повторного освобождения)
            state.registered[i].events = 0;
        }
    }
}

/*
 * Регистрирует обработчик для определённого кода события.
 * 
 * Параметры:
 *   code      - код события для прослушивания
 *   listener  - указатель на объект-слушатель
 *   on_event  - функция-обработчик
 * 
 * Возвращает:
 *   TRUE  - успешная регистрация
 *   FALSE - система не инициализирована, дубликат или ошибка
 */
b8 event_register(u16 code, void* listener, PFN_on_event on_event) {
    // Проверка инициализации системы
    if(is_initialized == FALSE) {
        return FALSE;
    }
    
    // Если для этого кода ещё нет массива обработчиков - создаём его
    if(state.registered[code].events == 0) {
        state.registered[code].events = darray_create(registered_event);
    }
    
    // Проверяем, нет ли уже такой же регистрации
    u64 registered_count = darray_length(state.registered[code].events);
    for(u64 i = 0; i < registered_count; ++i) {
        // Ищем совпадение по listener (объекту-слушателю)
        if(state.registered[code].events[i].listener == listener) {
            // TODO: добавить предупреждение в лог
            return FALSE; // Дубликат найден
        }
    }
    
    // Дубликатов не найдено - создаём новую регистрацию
    registered_event event;
    event.listener = listener;    // Сохраняем указатель на слушателя
    event.callback = on_event;    // Сохраняем указатель на функцию
    
    // Добавляем в динамический массив для этого кода события
    darray_push(state.registered[code].events, event);
    
    return TRUE; // Успешная регистрация
}

/*
 * Отменяет регистрацию обработчика для определённого кода события.
 * 
 * Параметры:
 *   code      - код события
 *   listener  - указатель на объект-слушатель (должен совпадать)
 *   on_event  - функция-обработчик (должна совпадать)
 * 
 * Возвращает:
 *   TRUE  - успешное удаление регистрации
 *   FALSE - система не инициализирована, регистрация не найдена
 */
b8 event_unregister(u16 code, void* listener, PFN_on_event on_event) {
    // Проверка инициализации системы
    if(is_initialized == FALSE) {
        return FALSE;
    }
    
    // Если для этого кода нет зарегистрированных событий - выходим
    if(state.registered[code].events == 0) {
        // TODO: добавить предупреждение в лог
        return FALSE;
    }
    
    // Ищем точное совпадение listener + callback
    u64 registered_count = darray_length(state.registered[code].events);
    for(u64 i = 0; i < registered_count; ++i) {
        registered_event e = state.registered[code].events[i];
        
        // Проверяем полное совпадение
        if(e.listener == listener && e.callback == on_event) {
            // Нашли - удаляем из массива
            registered_event popped_event;
            darray_pop_at(state.registered[code].events, i, &popped_event);
            return TRUE; // Успешное удаление
        }
    }
    
    // Совпадение не найдено
    return FALSE;
}

/*
 * Вызывает (отправляет) событие всем зарегистрированным обработчикам.
 * Обработчики вызываются в порядке регистрации.
 * 
 * Параметры:
 *   code    - код события для отправки
 *   sender  - указатель на отправителя события
 *   context - контекст с данными события
 * 
 * Возвращает:
 *   TRUE  - событие было обработано (какой-то обработчик вернул TRUE)
 *   FALSE - система не инициализирована, нет обработчиков или никто не обработал
 */
b8 event_fire(u16 code, void* sender, event_context context) {
    // Проверка инициализации системы
    if(is_initialized == FALSE) {
        return FALSE;
    }
    
    // Если для этого кода нет зарегистрированных обработчиков
    if(state.registered[code].events == 0) {
        return FALSE;
    }
    
    // Вызываем все зарегистрированные обработчики
    u64 registered_count = darray_length(state.registered[code].events);
    for(u64 i = 0; i < registered_count; ++i) {
        registered_event e = state.registered[code].events[i];
        
        // Вызываем callback-функцию
        if(e.callback(code, sender, e.listener, context)) {
            // Обработчик вернул TRUE - событие обработано
            // Не вызываем остальные обработчики (цепочка прерывается)
            return TRUE;
        }
        // Если обработчик вернул FALSE - продолжаем цепочку
    }
    
    // Ни один обработчик не вернул TRUE (или обработчиков не было)
    return FALSE;
} 