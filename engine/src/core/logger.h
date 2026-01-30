/*

   Этот код реализует систему логирования с различными уровнями важности
   сообщений, такими как ошибки, предупреждения, информация, отладка и следы
   выполнения (trace)

*/

#pragma once

#include "../defines.h"

/* Конфигурация уровня логирования: */
#define LOG_WARN_ENABLED                                                       \
  1 // позволяет логировать сообщения уровня предупреждений (warn).
#define LOG_INFO_ENABLED                                                       \
  1 // позволяет логировать сообщения уровня информации (info).
#define LOG_DEBUG_ENABLED                                                      \
  1 // позволяет логировать сообщения уровня отладки (debug).
#define LOG_TRACE_ENABLED                                                      \
  1 // позволяет логировать сообщения на уровне трассировки (trace).

/* Отключение логирования для релизных сборок */
#if KRELEASE == 1
#define LOG_DEBUG_ENABLED 0
#define LOG_TRACE_ENABLED 0
#endif

/* Определение уровней логирования */
typedef enum log_level {
  LOG_LEVEL_FATAL = 0, // Сообщения критического уровня. Эти ошибки обычно
                       // означают, что программа не может продолжать работать.
  LOG_LEVEL_ERROR = 1, // Ошибки, которые могут быть важными, но программа может
                       // продолжить работать.
  LOG_LEVEL_WARN = 2, // Предупреждения, которые не являются критическими, но
                      // требуют внимания.
  LOG_LEVEL_INFO = 3, // Информационные сообщения, полезные для общего понимания
                      // того, что делает программа.
  LOG_LEVEL_DEBUG =
      4, // Сообщения для отладки, которые показывают детали работы программы.
  LOG_LEVEL_TRACE = 5 // Очень детализированные сообщения для отслеживания
                      // состояния программы в реальном времени.
} log_level;

/* Функции для инициализации и завершения логирования */
b8 initialize_logging();
void shutdown_logging();

/* Основная функция для вывода логов */
// level — уровень логирования.
// message — само сообщение (строка, которая будет выведена в лог).
// Вызов этой функции также поддерживает использование переменных аргументов
// (через ...), что позволяет использовать её как форматированную строку,
// например, с printf.
KAPI void log_output(log_level level, const char *message, ...);

/* Макросы для удобного логирования */

// Используется для логирования фатальных ошибок, которые обычно означают
// невозможность продолжить выполнение программы
#define KFATAL(message, ...)                                                   \
  log_output(LOG_LEVEL_FATAL, message, ##__VA_ARGS__);

// Этот макрос логирует ошибки. Если макрос KERROR уже определён в другом месте,
// он не будет переопределяться.
#ifndef KERROR
#define KERROR(message, ...)                                                   \
  log_output(LOG_LEVEL_ERROR, message, ##__VA_ARGS__);
#endif

// Логирует предупреждения только если LOG_WARN_ENABLED равен 1. Если
// предупреждения отключены, макрос не делает ничего.
#if LOG_WARN_ENABLED == 1
#define KWARN(message, ...) log_output(LOG_LEVEL_WARN, message, ##__VA_ARGS__);
#else
#define KWARN(message, ...)
#endif

// Логирует информационные сообщения только если LOG_INFO_ENABLED равен 1.
#if LOG_INFO_ENABLED == 1
#define KINFO(message, ...) log_output(LOG_LEVEL_INFO, message, ##__VA_ARGS__);
#else
#define KINFO(message, ...)
#endif

// Логирует сообщения для отладки только если LOG_DEBUG_ENABLED равен 1.
#if LOG_DEBUG_ENABLED == 1
#define KDEBUG(message, ...)                                                   \
  log_output(LOG_LEVEL_DEBUG, message, ##__VA_ARGS__);
#else
#define KDEBUG(message, ...)
#endif

// Логирует трассировочные сообщения только если LOG_TRACE_ENABLED равен 1.
#if LOG_TRACE_ENABLED == 1
#define KTRACE(message, ...)                                                   \
  log_output(LOG_LEVEL_TRACE, message, ##__VA_ARGS__);
#else
#define KTRACE(message, ...)
#endif
