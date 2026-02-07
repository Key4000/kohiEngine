/*
 Приложение движка - реализация

*/

#include "application.h"
#include "game_types.h"

#include "logger.h"

#include "platform/platform.h"

//хранит глобальное состояние приложения
//управляет игровым циклом
//работает с платформенным слоем
typedef struct application_state {
 // Указатель на объект игры
 game* game_inst;

 // Флаги состояния приложения
 b8 is_running; // Приложение запущено
 b8 is_suspended; // Приложение приостановлено/свёрнуто

 // Состояние платформы (ОС/окна/графики)
 platform_state platform;

 // Размеры окна/экрана
 i16 width; // Ширина
 i16 height; // Высота

 // Временная метка для расчёта дельты времени
 f64 last_time;
} application_state;

//static - имеют внутреннее связывание(видны только в этом файле/еденице трансляции)
//флаг инициализации
static b8 initialized = FALSE;
//Единственный экземпляр состояния приложения в программе
//Статический, значит существует на протяжении всей работы программы
//Глобально доступен в пределах файла (но не извне)
static application_state app_state;

//Инициализация движка
b8 application_create(game* game_inst) {
 //защита от повторной инициализации
 if (initialized) {
  KERROR("application_create called more than once.");
  return FALSE;
 }

 //Сохраняет переданный указатель на игровой объект в глобальном состоянии
 app_state.game_inst = game_inst;

 //Инициализация системы логирования:
 initialize_logging();

 // TODO: Remove this
 KFATAL("A test message: %f", 3.14f);
 KERROR("A test message: %f", 3.14f);
 KWARN("A test message: %f", 3.14f);
 KINFO("A test message: %f", 3.14f);
 KDEBUG("A test message: %f", 3.14f);
 KTRACE("A test message: %f", 3.14f);

 //Установка базовых флагов
 app_state.is_running = TRUE;
 app_state.is_suspended = FALSE;

 //Инициализация платформы, через game_inst
 if (!platform_startup(
  &app_state.platform,
  game_inst->app_config.name,
  game_inst->app_config.start_pos_x,
  game_inst->app_config.start_pos_y,
  game_inst->app_config.start_width,
  game_inst->app_config.start_height)) {
  return FALSE;
 }

 //Инициализация игры
 /*
 Движок вызывает функцию инициализации игры через указатель
 Передаёт ей указатель на структуру game (с возможностью доступа к state)
 Игра в своей реализации game_initialize() должна:
   · Инициализировать свои ресурсы (текстуры, модели, звуки)
   · Настроить начальное состояние
   · Вернуть TRUE при успехе
 */
 if (!app_state.game_inst->initialize(app_state.game_inst)) {
  KFATAL("Game failed to initialize.");
  return FALSE;
 }

 //вызов обработчика изменения размера
 app_state.game_inst->on_resize(app_state.game_inst, app_state.width, app_state.height);

 //устанавливаем защиту от повторного вызова
 initialized = TRUE;
 return TRUE;
}

//главный игровой цикл
b8 application_run() {

 while (app_state.is_running) {

  //обработка сообщений ОС
  if(!platform_pump_messages(&app_state.platform)) {
   app_state.is_running = FALSE;
  }

  //если не приостановлено
  if(!app_state.is_suspended) {
   //обновление игры
   if (!app_state.game_inst->update(app_state.game_inst, (f32)0)) {
    KFATAL("Game update failed, shutting down.");
    app_state.is_running = FALSE;
    break;
   }

   //отрисовка игры, рендер
   if (!app_state.game_inst->render(app_state.game_inst, (f32)0)) {
    KFATAL("Game render failed, shutting down.");
    app_state.is_running = FALSE;
    break;
   }
  }
 }

 //остановка движка и убираем за собой
 app_state.is_running = FALSE;
 platform_shutdown(&app_state.platform);

 return TRUE;
}