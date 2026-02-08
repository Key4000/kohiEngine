/*

   Основная точка входа в приложение.
 
*/

#pragma once

#include "core/application.h"
#include "core/logger.h"
#include "game_types.h"
#include "core/kmemory.h"

//Внешне определенная функция для создания игры.
extern b8 create_game(game* out_game);


int main(void) {
    initialize_memory();

    // Создание экземпляра игры
    game game_inst;
    if (!create_game(&game_inst)) {
        KFATAL("Не удалось создать игру!");
        return -1;
    }

    // Валидация указателей на функции
    
   if (!game_inst.initialize) KERROR("initialize is NULL");
   if (!game_inst.update) KERROR("update is NULL");
   if (!game_inst.render) KERROR("render is NULL");
   if (!game_inst.on_resize) KERROR("on_resize is NULL");
   if (!game_inst.initialize || !game_inst.update || 
       !game_inst.render || !game_inst.on_resize) {
       KFATAL("");
       return -2;
   }

    // Инициализация
    if (!application_create(&game_inst)) {
        KINFO("Не удалось создать приложение!.");
        return 1;
    }

    // Начинаем цикл игры.
    if(!application_run()) {
        KINFO("Приложение не завершило работу корректно.");
        return 2;
    }
    shutdown_memory();

    return 0;
}