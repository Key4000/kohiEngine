/* 
   файл игры
*/
#include "game.h"

#include <core/logger.h>

//иницализация 
b8 game_initialize(game* game_inst) {
    KDEBUG("game_initialize() called!");
    return TRUE;
}

//обновление игры
b8 game_update(game* game_inst, f32 delta_time) {
    return TRUE;
}

//отрисовка
b8 game_render(game* game_inst, f32 delta_time) {
    return TRUE;
}

//изменение размера окна игры
void game_on_resize(game* game_inst, u32 width, u32 height) {
}