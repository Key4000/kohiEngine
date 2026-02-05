#include "platform/platform.h"

// Windows platform layer.
#if KPLATFORM_WINDOWS

#include "core/logger.h"

#include <windows.h>
#include <windowsx.h>  // param input extraction
#include <stdlib.h>

typedef struct internal_state {
    HINSTANCE h_instance;  //дескриптор экземпляра приложения, который используется Windows API для идентификации процесса.
    HWND hwnd;  //дескриптор окна, который используется для обращения к окну в системе 
} internal_state;


static f64 clock_frequency; // Частота таймера
static LARGE_INTEGER start_time;  // Время старта

//Это основной обработчик сообщений от системы. Он обрабатывает различные события, такие как нажатие клавиш, движение
LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param);

//Эта функция инициализирует приложение
//создает класс окна, создает окно, инициализация таймера 
b8 platform_startup(
    platform_state *plat_state,
    const char *application_name,
    i32 x,
    i32 y,
    i32 width,
    i32 height) 
{
    //Выделение памяти для состояния приложения
    plat_state->internal_state = malloc(sizeof(internal_state));
    //создаем указатель на выделенную память 
    internal_state *state = (internal_state *)plat_state->internal_state;
    //Получение хэндла текущего процесса
    //Функция GetModuleHandleA возвращает хэндл текущего модуля (программы). Это необходимо для создания окон и использования других ресурсов. 
    state->h_instance = GetModuleHandleA(0);

    /* - Настройка и регистрация класса окна - */
    //загружаем иконку(принимает параметры: дескриптор текущего исполняемого файла, код стандартной системной иконки(синяя буква "А", на белом фоне))
    HICON icon = LoadIcon(state->h_instance, IDI_APPLICATION);
    //структура, которая описывает класс окна 
    WNDCLASSA wc;
    //обнуляем структуру, теперь все поля wc, становятся равными 0 или NULL
    memset(&wc, 0, sizeof(wc));
    //обрабатывать двойные клики 
    wc.style = CS_DBLCLKS;
    //обработчик сообщений 
    wc.lpfnWndProc = win32_process_message;
    //кол-во доп байт для класса 
    wc.cbClsExtra = 0;
    //кол-во доп байт для каждого окна 
    wc.cbWndExtra = 0;
    //дескриптор текущего приложения
    wc.hInstance = state->h_instance
    //устанавливаем иконку
    wc.hIcon = icon;
    //настройка курсора(принимает  NULL = системный, стандартный системный курсор)
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    //отключаем заливку фона, для полного контроля над рендерингом 
    wc.hbrBackground = NULL;       
    
    //внутренний идентификатор(имя) окна, для работы с ним 
    wc.lpszClassName = "kohi_window_class";
   //обработка ошибок регистрации класса окна 
    if (!RegisterClassA(&wc)) {
        //появится поверх всех окон 
        MessageBoxA(0, "Window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK);
        return FALSE;
    }

    /*      создаем окно     */
    u32 client_x = x;
    u32 client_y = y;
    u32 client_width = width;
    u32 client_height = height;

    u32 window_x = client_x;
    u32 window_y = client_y;
    u32 window_width = client_width;
    u32 window_height = client_height;

    
    //заголовок, системное меню(иконка справа), рамка
    u32 window_style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
    //окно в панели задач windows
    u32 window_ex_style = WS_EX_APPWINDOW;
   
    window_style |= WS_MAXIMIZEBOX;  // Кнопка "развернуть"
    window_style |= WS_MINIMIZEBOX;  // Кнопка "свернуть"  
    window_style |= WS_THICKFRAME;   // Возможность изменять размер

    //вычисляем размеры рамок 
    RECT border_rect = {0, 0, 0, 0};
    AdjustWindowRectEx(&border_rect, window_style, 0, window_ex_style);

    // в этом случае граница отрицательная 
    window_x += border_rect.left;
    window_y += border_rect.top;

    // растёт по размеру границы ОС
    window_width += border_rect.right - border_rect.left;
    window_height += border_rect.bottom - border_rect.top;

    //создаем окно 
    HWND handle = CreateWindowExA(
        window_ex_style, "kohi_window_class", application_name,
        window_style, window_x, window_y, window_width, window_height,
        0, 0, state->h_instance, 0);

    //обработка ошибок создания окна 
    if (handle == 0) {
        MessageBoxA(NULL, "Window creation failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        KFATAL("Window creation failed!");
        return FALSE;
    } else {
        state->hwnd = handle;
    }

    /* - Код для управления отображением и активацией окна -  */
    //должно ли стать активным окно 
    b32 should_activate = 1;  // TODO: если окно не должно принимать ввод должно быть false 
    //если окно активировано, то показать и активировать, если нет, то показать, но не активировать
    i32 show_window_command_flags = should_activate ? SW_SHOW : SW_SHOWNOACTIVATE;
    // Если изначально свернуто, использовать SW_MINIMIZE или SW_SHOWMINNOACTIVE
    // Если изначально развернуто, использовать SW_SHOWMAXIMIZED или SW_MAXIMIZE
    
    //контролирует видимость окна и его состояние(принимает дескриптор окна и флаги "как показать окно")
    ShowWindow(state->hwnd, show_window_command_flags);

    /* - инициализация таймера - */
    
    //int 64 бита которое будет хранить частоту таймера 
    LARGE_INTEGER frequency;
    
    //заполняем структуру реальной частотой 
    QueryPerformanceFrequency(&frequency);
    
    //вычисляем длительность одного тика 
    clock_frequency = 1.0 / (f64)frequency.QuadPart;
    
    //получаем текущее значение счетчика при запуске
    QueryPerformanceCounter(&start_time);

    return TRUE;
}


//Функция завершения платформенного слоя
//корректное уничтожение окон и других ресурсов windows
//освобождение памяти и других ресурсов выделенных в platform_startup 
//завершение работы в обратном порядкн инициализации
void platform_shutdown(platform_state *plat_state) {
    //простое привидение типа, мы знаем что там лежит internal_state
    internal_state *state = (internal_state *)plat_state->internal_state;
    //если дескриптор окна не NULL 
    if (state->hwnd) {
        //удаляем окно 
        DestroyWindow(state->hwnd);
        //обнуляем дескриптор 
        state->hwnd = 0;
    }
}

//цикл обработки сообщений windows
b8 platform_pump_messages(platform_state *plat_state) {
    //структура MSG содержит всю информацию о сообщении 
    MSG message;
    //
    
    /*
    BOOL PeekMessageA(
    LPMSG lpMsg,        // Куда положить сообщение
    HWND  hWnd,         // Для какого окна (NULL = все окна потока)
    UINT  wMsgFilterMin,// Минимальный ID сообщения для фильтрации
    UINT  wMsgFilterMax,// Максимальный ID сообщения для фильтрации
    UINT  wRemoveMsg    // Что делать с сообщением после чтения
); 
    1. &message - адрес структуры MSG, куда будет помещено сообщение
    2. NULL - получать сообщения для всех окон текущего потока
    3. 0, 0 - не фильтровать по типу сообщений
    4. PM_REMOVE - удалить сообщение из очереди после чтения
    */
    while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
      
        // Преобразует сообщения клавиатуры
        // WM_KEYDOWN/WM_KEYUP → WM_CHAR 
        TranslateMessage(&message);
        //передает сообщение в вашу оконную процедуру(win32_procces_message)
        DispatchMessageA(&message);
    }
    
    return TRUE;
}

/* - абстракция(обёртки) - */

//выделение памяти
void *platform_allocate(u64 size, b8 aligned) {
    return malloc(size);
}

//освобождение памяти 
void platform_free(void *block, b8 aligned) {
    free(block);
}

//обнуление памяти 
void *platform_zero_memory(void *block, u64 size) {
    return memset(block, 0, size);
}

//копирование данных
void *platform_copy_memory(void *dest, const void *source, u64 size) {
    return memcpy(dest, source, size);
}

//заполнение значением памяти 
void *platform_set_memory(void *dest, i32 value, u64 size) {
    return memset(dest, value, size);
}

//функция логирования в консоль 
void platform_console_write(const char *message, u8 colour) {
    //получаем дескриптор стандартного вывода
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    //уровни логирования 
    // FATAL,ERROR,WARN,INFO,DEBUG,TRACE
    static u8 levels[6] = {64, 4, 6, 2, 1, 8};
    
    //устанавливает цвет фона и текста для последующего вывода
    SetConsoleTextAttribute(console_handle, levels[colour]);
    
    //отправляет строку в отладчик
    OutputDebugStringA(message);
   
    //длина сообщения 
    u64 length = strlen(message);
   
    //Long Pointer to DWORD = DWORD* (32 unsigned int)
    LPDWORD number_written = 0;
   
    /*
    BOOL WriteConsoleA(
    HANDLE  hConsoleOutput,      // Дескриптор консоли
    const VOID* lpBuffer,        // Данные для записи
    DWORD   nNumberOfCharsToWrite, // Сколько символов записать
    LPDWORD lpNumberOfCharsWritten, // Сюда запишет сколько записалось
    LPVOID  lpReserved           // Зарезервировано (должно быть NULL)
); 
    
    */
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message, (DWORD)length, number_written, 0);
}


//функция для вывода ошибок в stderr
void platform_console_write_error(const char *message, u8 colour) {
    //получаем дескриптор стандартного вывода ошибок
    HANDLE console_handle = GetStdHandle(STD_ERROR_HANDLE);
    // FATAL,ERROR,WARN,INFO,DEBUG,TRACE
    //уровни логирования
    static u8 levels[6] = {64, 4, 6, 2, 1, 8};
    //устанавливаем цвет фона и текста для последующего вывода 
    SetConsoleTextAttribute(console_handle, levels[colour]);
    
    //отправляет строку в отладчик 
    OutputDebugStringA(message);
    
    
    //длина сообщения
    u64 length = strlen(message);
    
    //Long Pointer to DWORD = DWORD* (32 unsigned int)
    LPDWORD number_written = 0;
    
        /*
    BOOL WriteConsoleA(
    HANDLE  hConsoleOutput,      // Дескриптор консоли
    const VOID* lpBuffer,        // Данные для записи
    DWORD   nNumberOfCharsToWrite, // Сколько символов записать
    LPDWORD lpNumberOfCharsWritten, // Сюда запишет сколько записалось
    LPVOID  lpReserved           // Зарезервировано (должно быть NULL)
); 
    
    */ 
    WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), message, (DWORD)length, number_written, 0);
}

//получение точного времени 
f64 platform_get_absolute_time() {
   
    //int 64 для хранения времени 
    LARGE_INTEGER now_time;
    //получает значение счетчика 
    QueryPerformanceCounter(&now_time);
  
    //количество тиков * сколько секунд на тик 
    return (f64)now_time.QuadPart * clock_frequency;
}

//поставить поток на паузу 
void platform_sleep(u64 ms) {
    Sleep(ms);
}

//оконная процедура(это callback которую windows вызывает на каждое сообщение для окна  
LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param) {
    switch (msg) {
        case WM_ERASEBKGND:
            // Notify the OS that erasing will be handled by the application to prevent flicker.
            return 1;
        case WM_CLOSE:
            // TODO: Fire an event for the application to quit.
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE: {
            // Get the updated size.
            // RECT r;
            // GetClientRect(hwnd, &r);
            // u32 width = r.right - r.left;
            // u32 height = r.bottom - r.top;

            // TODO: Fire an event for window resize.
        } break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            // Key pressed/released
            //b8 pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
            // TODO: input processing

        } break;
        case WM_MOUSEMOVE: {
            // Mouse move
            //i32 x_position = GET_X_LPARAM(l_param);
            //i32 y_position = GET_Y_LPARAM(l_param);
            // TODO: input processing.
        } break;
        case WM_MOUSEWHEEL: {
            // i32 z_delta = GET_WHEEL_DELTA_WPARAM(w_param);
            // if (z_delta != 0) {
            //     // Flatten the input to an OS-independent (-1, 1)
            //     z_delta = (z_delta < 0) ? -1 : 1;
            //     // TODO: input processing.
            // }
        } break;
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP: {
            //b8 pressed = msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN;
            // TODO: input processing.
        } break;
    }

    return DefWindowProcA(hwnd, msg, w_param, l_param);
}

#endif // KPLATFORM_WINDOWS
