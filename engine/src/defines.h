#pragma once

// Беззнаковые целые
typedef unsigned char u8;       // 8 бит
typedef unsigned short u16;     // 16 бит
typedef unsigned int u32;       // 32 бита
typedef unsigned long long u64; // 64 бита

// Знаковые целые
typedef signed char i8;       // 8 бит
typedef signed short i16;     // 16 бит
typedef signed int i32;       // 32 бита
typedef signed long long i64; // 64 бита

// Числа с плавающей точкой
typedef float f32;  // 32 бита
typedef double f64; // 64 бита

// Логические типы
typedef int b32; // логическое значение (32 бита)
typedef char b8; // логическое значение (8 бит)

// STATIC_ASSERT — статические проверки
#if defined(__clang__) || defined(__gcc__)
#define STATIC_ASSERT _Static_assert
#else
#define STATIC_ASSERT static_assert
#endif

// Проверка размеров типов
// Если компилятор или платформа ведёт себя неожиданно — компиляция упадёт
// сразу, а не создаст скрытые баги.
STATIC_ASSERT(sizeof(u8) == 1, "Expected u8 to be 1 byte.");
STATIC_ASSERT(sizeof(u16) == 2, "Expected u16 to be 2 bytes.");
STATIC_ASSERT(sizeof(u32) == 4, "Expected u32 to be 4 bytes.");
STATIC_ASSERT(sizeof(u64) == 8, "Expected u64 to be 8 bytes.");

STATIC_ASSERT(sizeof(i8) == 1, "Expected i8 to be 1 byte.");
STATIC_ASSERT(sizeof(i16) == 2, "Expected i16 to be 2 bytes.");
STATIC_ASSERT(sizeof(i32) == 4, "Expected i32 to be 4 bytes.");
STATIC_ASSERT(sizeof(i64) == 8, "Expected i64 to be 8 bytes.");

STATIC_ASSERT(sizeof(f32) == 4, "Expected f32 to be 4 bytes.");
STATIC_ASSERT(sizeof(f64) == 8, "Expected f64 to be 8 bytes.");

#define TRUE 1
#define FALSE 0

/* Определение платформы */
// Определяет Windows
// Запрещает 32-битную сборку → движок строго 64-bit
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define KPLATFORM_WINDOWS 1
#ifndef _WIN64
#error "64-bit is required on Windows!"
#endif

// Linux / Android
#elif defined(__linux__) || defined(__gnu_linux__)
#define KPLATFORM_LINUX 1

#if defined(__ANDROID__)
#define KPLATFORM_ANDROID 1
#endif

// Unix / POSIX
#elif defined(__unix__)
#define KPLATFORM_UNIX 1

#elif defined(_POSIX_VERSION)
#define KPLATFORM_POSIX 1

// Apple (macOS / iOS)
#elif __APPLE__
#define KPLATFORM_APPLE 1
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR
#define KPLATFORM_IOS 1
#define KPLATFORM_IOS_SIMULATOR 1
#elif TARGET_OS_IPHONE
#define KPLATFORM_IOS 1
#elif TARGET_OS_MAC
#else
#error "Unknown Apple platform"
#endif

#else
#error "Unknown platform!"
#endif

#if defined(KPLATFORM_WINDOWS)

#if defined(KEXPORT)
#if defined(_MSC_VER)
#define KAPI __declspec(dllexport)
#else
#define KAPI __attribute__((visibility("default")))
#endif
#else
#if defined(_MSC_VER)
#define KAPI __declspec(dllimport)
#else
#define KAPI
#endif
#endif

#else
#define KAPI

#endif
