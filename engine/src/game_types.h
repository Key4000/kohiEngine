/*
 * Определяет контракт, который должна реализовать 
 * любая игра на этом движке.
 */

#pragma once

#include "core/application.h"


 
 typedef struct game {
    application_config app_config;  // Конфигурация приложения
    b8 (*initialize)(struct game*); // Указатель на функцию инициализации
    b8 (*update)(struct game*, f32); // Указатель на функцию обновления
    b8 (*render)(struct game*, f32); // Указатель на функцию рендеринга
    void (*on_resize)(struct game*, u32, u32); // Обработчик изменения размера
    void* state; // Указатель на состояние игры (кастомные данные)
} game;
