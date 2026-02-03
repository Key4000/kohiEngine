/*
Этот код описывает систему утверждений (assertions) для проверки корректности
работы программы на этапе выполнения. Он включает механизмы для проверки условий
(asserts) и обработки ошибок с использованием дебаггеров и сообщений об ошибках.
*/

#pragma once

#include <defines.h>

// Если этот макрос закомментировать, то все проверки утверждений будут
// отключены, и код в блоках #ifdef KASSERTIONS_ENABLED не будет выполняться.
// Это удобно для отключения утверждений в производственных сборках, где
// производительность важнее, чем детальная диагностика.
#define KASSERTIONS_ENABLED

//Макрос debugBreak используется для принудительного вызова точки останова
//(breakpoint) в отладчике при ошибке. Это поможет разработчику остановить
//выполнение программы на момент, когда условие утверждения не выполнено.
#ifdef KASSERTIONS_ENABLED
#if _MSC_VER
//На Windows (MSVC) используется встроенная функция __debugbreak(), которая
//вызывает точку останова, если отладчик подключен.
#include <intrin.h>
#define debugBreak() __debugbreak()
#else
//На Linux и других UNIX-подобных системах используется __builtin_trap(),
//которая заставляет программу завершиться или войти в отладчик.
#define debugBreak() __builtin_trap()
#endif

//Эта функция будет вызываться, когда одно из утверждений не будет выполнено.
//Она принимает: expression: саму строку с выражением, которое проверяется
// (например, a == b). message: дополнительное сообщение об ошибке. file: имя
// файла, в котором произошла ошибка. line: строка, на которой произошла ошибка.
KAPI void report_assertion_failure(const char *expression, const char *message,
                                   const char *file, i32 line);

//Проверяет условие expr. Если условие не выполнено (т.е. выражение ложно),
//вызывается функция report_assertion_failure, и выполнение программы
//останавливается с помощью debugBreak().
#define KASSERT(expr)                                                          \
  {                                                                            \
    if (expr) {                                                                \
    } else {                                                                   \
      report_assertion_failure(#expr, "", __FILE__, __LINE__);                 \
      debugBreak();                                                            \
    }                                                                          \
  }

//Это вариант макроса с дополнительным сообщением message, которое будет
//передано в функцию report_assertion_failure в случае ошибки. Таким образом,
//разработчик может дать дополнительное объяснение, что именно пошло не так.
#define KASSERT_MSG(expr, message)                                             \
  {                                                                            \
    if (expr) {                                                                \
    } else {                                                                   \
      report_assertion_failure(#expr, message, __FILE__, __LINE__);            \
      debugBreak();                                                            \
    }                                                                          \
  }

//Только для отладки
#ifdef _DEBUG
//В режиме отладки (_DEBUG определён) макрос будет работать как обычный KASSERT
//— проверяя выражение и вызывая точку останова.
#define KASSERT_DEBUG(expr)                                                    \
  {                                                                            \
    if (expr) {                                                                \
    } else {                                                                   \
      report_assertion_failure(#expr, "", __FILE__, __LINE__);                 \
      debugBreak();                                                            \
    }                                                                          \
  }
#else
#define KASSERT_DEBUG(expr) // Does nothing at all
#endif
#else

//Если макрос KASSERTIONS_ENABLED будет закомментирован, то все макросы
//утверждений (KASSERT, KASSERT_MSG, KASSERT_DEBUG) не будут ничего делать (то
//есть станут пустыми).
#define KASSERT(expr) // Does nothing at all
#define KASSERT_MSG(expr, message) // Does nothing at all
#define KASSERT_DEBUG(expr) // Does nothing at all
#endif
