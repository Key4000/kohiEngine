/* 
   файл игры
*/
#pragma once

#include <defines.h>
#include <game_types.h>

//состояние игры
typedef struct game_state {
    f32 delta_time;
} game_state;

//иницализация 
b8 game_initialize(game* game_inst);

//обновление игры
b8 game_update(game* game_inst, f32 delta_time);

//отрисовка
b8 game_render(game* game_inst, f32 delta_time);

//изменение размера окна игры
void game_on_resize(game* game_inst, u32 width, u32 height);