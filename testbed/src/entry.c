/*
   Запуск программы:
   entry.c (create_game) → main() → application_create() → application_run()
        ↓                         ↓                        ↓
   Определяет игру        Входная точка           Движок выполняет игру

   Почему отдельный файл?
   · Разделение ответственности: entry.c знает об игре, движок - нет
   · Гибкость: Можно создать разные entry.c для разных игр
   · Инверсия управления: Движок вызывает игру через callback'и
*/

#include "game.h"

#include <entry.h>

// TODO: Remove this
#include <platform/platform.h>

//функция создания игры
b8 create_game(game* out_game) {
 // конфигурация игры для запуска(испытательный полигон - testbed)
 out_game->app_config.start_pos_x = 100;
 out_game->app_config.start_pos_y = 100;
 out_game->app_config.start_width = 1280;
 out_game->app_config.start_height = 720;
 out_game->app_config.name = "Kohi Engine Testbed";
 out_game->update = game_update;
 out_game->render = game_render;
 out_game->initialize = game_initialize;
 out_game->on_resize = game_on_resize;

 //состояние игры 
 out_game->state = platform_allocate(sizeof(game_state), FALSE);

 return TRUE;
}