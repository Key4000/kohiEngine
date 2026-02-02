/*

Этот код реализует систему логирования и обработку сбоев утверждений в
приложении. Он включает в себя функции для инициализации логирования, вывод
логов различных уровней (например, ошибки, предупреждения, отладка) и обработку
сбоя утверждения. Всё это направлено на отслеживание и обработку состояния
приложения во время выполнения

*/

#include "logger.h"
#include "../defines.h"
#include "asserts.h"
#include "../platform/platform.h"

// TODO: temporary
#include <stdarg.h>
#include <stdio.h>
#include <string.h>


b8 initialize_logging() {
  // TODO: create log file.
  return TRUE;
}

void shutdown_logging() {
  // TODO: cleanup logging/write queued entries.
}

void log_output(log_level level, const char *message, ...) {
  // Массив строк, который соответствует разным уровням логирования:
  //[FATAL]: — для фатальных ошибок.
  //[ERROR]: — для обычных ошибок.
  //[WARN]: — для предупреждений.
  //[INFO]: — для информационных сообщений.
  //[DEBUG]: — для сообщений уровня отладки.
  //[TRACE]: — для трассировочных сообщений.
  const char *level_strings[6] = {"[FATAL]: ", "[ERROR]: ", "[WARN]:  ",
                                  "[INFO]:  ", "[DEBUG]: ", "[TRACE]: "};
  b8 is_error = level < LOG_LEVEL_WARN;

  // Буфер для хранения отформатированного сообщения лога. Он имеет размер 32
  // 000 байт (32 КБ), что накладывает ограничение на длину одного сообщения.
  const i32 msg_length = 32000;
  char out_message[msg_length];
  // Функция memset гарантирует, что все байты массива out_message будут
  // установлены в значение 0. Это важно для того, чтобы избежать случайных
  // значений в памяти, если массив используется позже, например, для строкового
  // вывода.
  memset(out_message, 0, sizeof(out_message));
  // ПРИМЕЧАНИЕ: Странно, но заголовки MS перекрывают тип GCC/Clang va_list с
  // помощью «typedef char* va_list» в некоторых кейсах, и в результате здесь
  // возникает странная ошибка. Пока что обходной путь — просто использовать
  // __builtin_va_list, что именно тот тип ожидает va_start GCC/Clang.
  __builtin_va_list arg_ptr;
  // инициализирует список аргументов для дальнейшего использования в vsnprintf.
  va_start(arg_ptr, message);
  // формирует строку сообщения, используя формат и переменные, переданные в лог
  // (через message и переменные аргументы ...).
  vsnprintf(out_message, msg_length, message, arg_ptr);
  // завершение использования переменного списка аргументов
  va_end(arg_ptr);
  // окончательный массив с сообщением
  char out_message2[msg_length];
  // Формирует окончательное сообщение лога, добавляя к отформатированному
  // сообщению префикс, который соответствует уровню логирования. Например,
  // [INFO]: или [ERROR]:.
  sprintf(out_message2, "%s%s\n", level_strings[level], out_message);
  // Сообщение выводится на консоль с помощью printf. Это предполагает, что в
  // будущем можно будет добавить поддержку других типов вывода, например, в
  // файл или на платформенные средства вывода
  // Вывод с учитыванием платформы
    if (is_error) {
        platform_console_write_error(out_message2, level);
    } else {
        platform_console_write(out_message2, level);
    }
}

// expression — строковое представление выражения, которое вызвало сбой.
// message — дополнительное сообщение о причине сбоя.
// file — имя файла, где произошло нарушение.
// line — строка, на которой произошёл сбой.
void report_assertion_failure(const char *expression, const char *message,
                              const char *file, i32 line) {
  // Сообщение об ошибке будет выведено в лог с уровнем LOG_LEVEL_FATAL, что
  // означает фатальную ошибку.
  log_output(LOG_LEVEL_FATAL,
             "Assertion Failure: %s, message: '%s', in file: %s, line: %d\n",
             expression, message, file, line);
}
