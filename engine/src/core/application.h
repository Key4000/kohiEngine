/*

  Приложение движка - хедер

*/

#pragma once

#include "defines.h"

struct game;

//Конфигурация движка
typedef struct application_config {
 // начальная позиция окна движка
 i16 start_pos_x;
 i16 start_pos_y;

 // стартовый размер окна движка
 i16 start_width;
 i16 start_height;
 //имя
 char* name;
} application_config;

//инициализация движка(передаем экземпляр игры, то есть движок запускает игру)
KAPI b8 application_create(struct game* game_inst);

//запустить движок
KAPI b8 application_run();