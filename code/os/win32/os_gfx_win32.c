/*  =======================================================================
    File: os_gfx_win32.c
    Date: March 5th 2024 11:13 PM
    Creator: Quinn Van De Keere
    =======================================================================*/

#define PACKETDATA PK_X | PK_Y | PK_BUTTONS | PK_NORMAL_PRESSURE
#define PACKETMODE 0

#include <os/win32/wintab/wintab.h>
#include <os/win32/wintab/pktdef.h>
#include <os/win32/wintab/msgpack.h>

typedef BOOL win32_set_process_dpi_awareness_context_type(void *Value);
typedef UINT win32_set_thread_cursor_creation_scaling(UINT CursorDPI);
typedef UINT win32_get_dpi_for_window_type(HWND Handle);
#define WIN32_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((void *)-4)

typedef UINT win32_wintab_infoa(UINT, UINT, LPVOID);
typedef HCTX win32_wintab_opena(HWND, LPLOGCONTEXTA, BOOL);
typedef BOOL win32_wintab_packet(HCTX, UINT, LPVOID);

typedef struct win32_window win32_window;
struct win32_window
{
    win32_window *Next;
    win32_window *Prev;
    HWND Handle;
    F32 DPI;
    void (*Repaint)(os_handle RepaintWindow);
    win32_window *Parent;

    // Tablet state data
    HCTX WTContext;
    U32 WTMaxPressure;
    U32 WTRangeX;
    U32 WTRangeY;
};

typedef struct win32_graphics_state win32_graphics_state;
struct win32_graphics_state
{
    HINSTANCE Instance;
    
    win32_window *FirstFreeWindow;
    win32_window *LastFreeWindow;
    
    win32_window *FirstActiveWindow;
    win32_window *LastActiveWindow;
    
    arena *WindowsArena;

    os_event_list *EventList;
    arena *EventArena;

    B8 Resizing;
    HCURSOR HCursor;

    win32_get_dpi_for_window_type *GetDPIForWindowFunc;

    B8 WinTabSupported;
    win32_wintab_infoa *WTInfoA;
    win32_wintab_opena *WTOpenA;
    win32_wintab_packet *WTPacket;
    os_tablet_state TabletState;
    
    F32 DefaultRefreshRate;

    arena *Arena;
};

global_variable win32_graphics_state *Win32GraphicsState = 0;

// Win32 specific functions

os_key Win32OSKeyFromVKey(WPARAM VKey);
WPARAM Win32VKeyFromOSKey(os_key Key);

os_event *Win32PushEvent(os_event_kind Kind, win32_window *Window)
{
    Assert(Win32GraphicsState->EventList);
    
    os_event *Result = PushStruct(Win32GraphicsState->EventArena, os_event);
    DLLPushBack(Win32GraphicsState->EventList->First, Win32GraphicsState->EventList->Last, Result);
    ++Win32GraphicsState->EventList->Count;

    Result->Kind = Kind;
    Result->Window = (os_handle)Window;
    
    return Result;
}

win32_window *Win32AllocateWindow()
{
    win32_window *Result = 0;
    
    if(Win32GraphicsState->FirstFreeWindow)
    {
        Result = Win32GraphicsState->FirstFreeWindow;
        DLLRemove(Win32GraphicsState->FirstFreeWindow, Win32GraphicsState->LastFreeWindow, Result);
        MemoryZero(Result, sizeof(win32_window));
    }
    else
        Result = PushStruct(Win32GraphicsState->WindowsArena, win32_window);
    DLLPushFront(Win32GraphicsState->FirstActiveWindow, Win32GraphicsState->LastActiveWindow, Result);

    return Result;
}

void Win32FreeWindow(win32_window *Window)
{
    DLLRemove(Win32GraphicsState->FirstActiveWindow, Win32GraphicsState->LastActiveWindow, Window);
    DLLPushFront(Win32GraphicsState->FirstFreeWindow, Win32GraphicsState->LastFreeWindow, Window);
}

win32_window *Win32WindowFromHWND(HWND Handle)
{
    win32_window *Result = 0;
    for(win32_window *Window = Win32GraphicsState->FirstActiveWindow; Window; Window = Window->Next)
    {
        if(Window->Handle == Handle)
        {
            Result = Window;
            break;
        }
    }
    return Result;
}

LRESULT Win32WindowProc(HWND Handle, UINT Message, WPARAM wParam, LPARAM lParam)
{
    LRESULT Result = 0;

    if(Win32GraphicsState->EventArena)
    {
        B8 Release = 0;
        switch(Message)
        {
            case WM_SIZE:
            case WM_PAINT:
            {
                win32_window *Window = Win32WindowFromHWND(Handle);
                if(Window->Repaint)
                {
                    PAINTSTRUCT Paint = {0};
                    BeginPaint(Window->Handle, &Paint);
                    Window->Repaint((os_handle)Window);
                    EndPaint(Window->Handle, &Paint);
                }
                else
                    Result = DefWindowProcW(Handle, Message, wParam, lParam);
            }break;

            case WM_LBUTTONUP:
            case WM_MBUTTONUP:
            case WM_RBUTTONUP:
            {
                Release = 1;
            }
            case WM_LBUTTONDOWN:
            case WM_MBUTTONDOWN:
            case WM_RBUTTONDOWN:
            {
                win32_window *Window = Win32WindowFromHWND(Handle);
                os_event *Event = Win32PushEvent(Release ? OS_EVENT_KIND_RELEASE : OS_EVENT_KIND_PRESS, Window);
            
                switch(Message)
                {
                    case WM_LBUTTONUP: case WM_LBUTTONDOWN:
                    {
                        Event->Key = OS_KEY_LEFT_MOUSE_BUTTON;
                    }break;
                    case WM_MBUTTONUP: case WM_MBUTTONDOWN:
                    {
                        Event->Key = OS_KEY_MIDDLE_MOUSE_BUTTON;
                    }break;
                    case WM_RBUTTONUP: case WM_RBUTTONDOWN:
                    {
                        Event->Key = OS_KEY_RIGHT_MOUSE_BUTTON;
                    }break;
                }
            
                Event->Position.X = (F32)LOWORD(lParam);
                Event->Position.Y = (F32)HIWORD(lParam);
                if(Release)
                    ReleaseCapture();
                else
                    SetCapture(Handle);
            }break;

            case WM_SYSKEYDOWN: case WM_SYSKEYUP:
            {
                if(wParam != VK_MENU && (wParam < VK_F1 || VK_F24 < wParam || wParam == VK_F4))
                    Result = DefWindowProcW(Handle, Message, wParam, lParam);
            }
            case WM_KEYDOWN: case WM_KEYUP:
            {
                B32 WasDown = (lParam & (1<<30));
                B32 IsDown = !(lParam & (1<<31));

                B32 IsRepeat = 0;
                if(!IsDown)
                    Release = 1;
                else if(WasDown)
                    IsRepeat = 1;

                B32 RightSided = 0;
                if((lParam & (1<<24)) &&
                   (wParam == VK_CONTROL || wParam == VK_RCONTROL ||
                    wParam == VK_MENU || wParam == VK_RMENU ||
                    wParam == VK_SHIFT || wParam == VK_RSHIFT))
                {
                    RightSided = 1;
                }

                win32_window *Window = Win32WindowFromHWND(Handle);
                os_event *Event = Win32PushEvent(Release ? OS_EVENT_KIND_RELEASE : OS_EVENT_KIND_PRESS, Window);
                Event->Key = Win32OSKeyFromVKey(wParam);
                Event->RepeatCount = lParam & (1<<15);
                Event->IsRepeat = IsRepeat;
                Event->RightSided = RightSided;
                if(Event->Key == OS_KEY_ALT && Event->Flags & OS_EVENT_FLAG_ALT) Event->Flags &= ~OS_EVENT_FLAG_ALT;
                if(Event->Key == OS_KEY_CTRL && Event->Flags & OS_EVENT_FLAG_CTRL) Event->Flags &= ~OS_EVENT_FLAG_CTRL;
                if(Event->Key == OS_KEY_SHIFT && Event->Flags & OS_EVENT_FLAG_SHIFT) Event->Flags &= ~OS_EVENT_FLAG_SHIFT;
            }break;

            case WM_MOUSEWHEEL:
            {
                win32_window *Window = Win32WindowFromHWND(Handle);
                I16 WheelDelta = HIWORD(wParam);
                os_event *Event = Win32PushEvent(OS_EVENT_KIND_SCROLL, Window);
                POINT Point = {0};
                Point.x = (I32)(I16)LOWORD(lParam);
                Point.y = (I32)(I16)HIWORD(lParam);
                ScreenToClient(Window->Handle, &Point);
                Event->Position.X = (F32)Point.x;
                Event->Position.Y = (F32)Point.y;
                Event->Delta = Vec2F32(0.0f, -(F32)WheelDelta);
            }break;

            case WM_MOUSEHWHEEL:
            {
                win32_window *Window = Win32WindowFromHWND(Handle);
                I16 WheelDelta = HIWORD(wParam);
                os_event *Event = Win32PushEvent(OS_EVENT_KIND_SCROLL, Window);
                POINT Point = {0};
                Point.x = (I32)(I16)LOWORD(lParam);
                Point.y = (I32)(I16)HIWORD(lParam);
                ScreenToClient(Window->Handle, &Point);
                Event->Position.X = (F32)Point.x;
                Event->Position.Y = (F32)Point.y;
                Event->Delta = Vec2F32(-(F32)WheelDelta, 0.0f);
            }break;

            case WT_PACKET:
            {
                win32_window *Window = Win32WindowFromHWND(Handle);
                PACKET Packet = {0};
                if((HCTX)lParam == Window->WTContext && Win32GraphicsState->WTPacket(Window->WTContext, (UINT)wParam, &Packet))
                {
                    POINT Point = {0};
                    Point.x = Packet.pkX;
                    Point.y = Packet.pkY;
                    ScreenToClient(Handle, &Point);

                    os_event *Event = Win32PushEvent(OS_EVENT_KIND_SCROLL, Window);
                    Event->Position.X = (F32)Point.x;
                    Event->Position.Y = (F32)Point.y;
                    Event->Pressure = (F32)Packet.pkNormalPressure / (F32)Window->WTMaxPressure;

                    Win32GraphicsState->TabletState.Position = Event->Position;
                    Win32GraphicsState->TabletState.Pressure = Event->Pressure;
                }
                else
                    Result = DefWindowProcW(Handle, Message, wParam, lParam);
            }break;
            
            case WM_ENTERSIZEMOVE:
            {
                Win32GraphicsState->Resizing = 1;
            }break;

            case WM_EXITSIZEMOVE:
            {
                Win32GraphicsState->Resizing = 0;
            }break;

            case WM_SETCURSOR:
            {
                win32_window *Window = Win32WindowFromHWND(Handle);
                os_handle WindowHandle = (os_handle)Window;
                if(!Win32GraphicsState->Resizing && Contains2F32(OSClientRectFromWindow(WindowHandle), OSMouseFromWindow(WindowHandle)))
                    SetCursor(Win32GraphicsState->HCursor);
                else
                    Result = DefWindowProcW(Handle, Message, wParam, lParam);
            }break;
            
            case WM_DESTROY:
            {
                win32_window *Window = Win32WindowFromHWND(Handle);
                Win32PushEvent(OS_EVENT_KIND_WINDOW_DESTROY, Window);
            }break;
        
            case WM_KILLFOCUS:
            {
                win32_window *Window = Win32WindowFromHWND(Handle);
                Win32PushEvent(OS_EVENT_KIND_WINDOW_LOSE_FOCUS, Window);
                ReleaseCapture();
            }break;

            case WM_DPICHANGED:
            {
                win32_window *Window = Win32WindowFromHWND(Handle);
                F32 NewDPI = (F32)(wParam & 0xffff);
                Window->DPI = NewDPI;
            }break;
            
            default:
            {
                Result = DefWindowProcW(Handle, Message, wParam, lParam);
            }break;
        }
    }
    else
    {
        Result = DefWindowProcW(Handle, Message, wParam, lParam);
    }

    return Result;
}

// Generic implementations

void OSGraphicsStartUp()
{
    arena *Arena = ArenaAlloc(Megabytes(4));
    Win32GraphicsState = PushStruct(Arena, win32_graphics_state);
    Win32GraphicsState->Arena = Arena;

    Win32GraphicsState->Instance = GetModuleHandle(0);
    
    Win32GraphicsState->WindowsArena = ArenaAlloc(Kilobytes(4));

    // Set DPI awareness
    {
        HMODULE Module = LoadLibraryA("user32.dll");
        if(Module)
        {
            win32_set_process_dpi_awareness_context_type *SetProcessDPIAwarenessContextFunc = 
                (win32_set_process_dpi_awareness_context_type *)GetProcAddress(Module, "SetProcessDpiAwarenessContext");
            Win32GraphicsState->GetDPIForWindowFunc = (win32_get_dpi_for_window_type *)GetProcAddress(Module, "GetDpiForWindow");
            win32_set_thread_cursor_creation_scaling *SetThreadCursorCreationScalingFunc =
                (win32_set_thread_cursor_creation_scaling *)GetProcAddress(Module, "SetThreadCursorCreationScaling");

            SetProcessDPIAwarenessContextFunc(WIN32_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
            SetThreadCursorCreationScalingFunc(96);
        }
        else
        {
            Warning("Cannot make use of advanced DPI functionality. Failed to load user32.dll\n");
        }
    }

    // register window class "graphics_window"
    {
        WNDCLASSEXW WindowClass = {sizeof(WNDCLASSEXW)};
        WindowClass.style = 0;
        WindowClass.lpfnWndProc = Win32WindowProc;
        WindowClass.cbClsExtra = 0;
        WindowClass.hInstance = Win32GraphicsState->Instance;
        WindowClass.hIcon = LoadIconA(0, IDI_APPLICATION);
        WindowClass.hCursor = LoadCursorA(0, IDC_ARROW);
        WindowClass.hbrBackground = 0;
        WindowClass.lpszMenuName = 0;
        WindowClass.lpszClassName = L"graphics_window";
        WindowClass.hIconSm = 0;
        
        RegisterClassExW(&WindowClass);
    }

    DEVMODEW DevModeW = {0};
    if(EnumDisplaySettingsW(0, ENUM_CURRENT_SETTINGS, &DevModeW))
        Win32GraphicsState->DefaultRefreshRate = (F32)DevModeW.dmDisplayFrequency;
    else
        Win32GraphicsState->DefaultRefreshRate = 60.0f;

    // Initialize WinTab API
    {
        HMODULE Module = LoadLibraryA("Wintab32.dll");
        if(Module)
        {
            Win32GraphicsState->WTInfoA = (win32_wintab_infoa *)GetProcAddress(Module, "WTInfoA");
            Win32GraphicsState->WTOpenA = (win32_wintab_opena *)GetProcAddress(Module, "WTOpenA");
            Win32GraphicsState->WTPacket = (win32_wintab_packet *)GetProcAddress(Module, "WTPacket");
            Win32GraphicsState->WinTabSupported = 1;
        }
        else
        {
            Warning("Unable to load Wintab32.dll. Tablet functionality is unsupported.\n");
        }
    }
}

typedef UINT (WINAPI * WTINFOA)(UINT, UINT, LPVOID);

F32 OSDefaultRefreshRate()
{
    return Win32GraphicsState->DefaultRefreshRate;
}

#define Win32CursorXList(X)                     \
    X(POINTER, IDC_ARROW)                       \
    X(IBAR, IDC_IBEAM)                          \
    X(LEFT_RIGHT, IDC_SIZEWE)                   \
    X(UP_DOWN, IDC_SIZENS)                      \
    X(DOWN_RIGHT, IDC_SIZENWSE)                 \
    X(UP_RIGHT, IDC_SIZENESW)                   \
    X(UP_DOWN_LEFT_RIGHT, IDC_SIZEALL)          \
    X(HAND_POINT, IDC_HAND)                     \
    X(DISABLED, IDC_NO)

#define CursorCase(E, R) case OS_CURSOR_##E:    \
    {                                           \
        local_persist HCURSOR CachedCursor = 0; \
        if(CachedCursor == 0)                   \
            CachedCursor = LoadCursor(0, R);    \
        HCursor = CachedCursor;                 \
    }break;

#define Win32CustomCursorXList(X)               \
    X(HAND_GRAB, HAND_GRAB_CURSOR)              \
    X(HAND_ROTATE, HAND_ROTATE_CURSOR)          \
    X(ZOOM_IN, ZOOM_IN_CURSOR)

#define CustomCursorCase(E, R) case OS_CURSOR_##E:                      \
    {                                                                   \
        local_persist HCURSOR CachedCursor = 0;                         \
        if(CachedCursor == 0)                                           \
            CachedCursor = LoadCursor(Win32GraphicsState->Instance, #R); \
        HCursor = CachedCursor;                                         \
    }break;

void OSSetCursor(os_cursor Cursor)
{
    B32 ValidCursor = 1;
    
    HCURSOR HCursor = 0;
    switch(Cursor)
    {        
        Win32CursorXList(CursorCase);
        Win32CustomCursorXList(CustomCursorCase);
        
        default: ValidCursor = 0; break;
    }

    if(ValidCursor && !Win32GraphicsState->Resizing)
    {
        if(HCursor != Win32GraphicsState->HCursor)
        {
            PostMessage(0, WM_SETCURSOR, 0, 0);
            POINT Point = {0};
            GetCursorPos(&Point);
            SetCursorPos(Point.x, Point.y);
        }
        Win32GraphicsState->HCursor = HCursor;
    }
}

#undef CustomCursorCase
#undef Win32CustomCursorXList
#undef CursorCase
#undef Win32CursorXList

os_event_list OSGetEvents(arena *Arena)
{
    os_event_list Events = {0};
    
    Win32GraphicsState->EventList = &Events;
    Win32GraphicsState->EventArena = Arena;
    
    MSG Message = {0};
    while(PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
    {
        DispatchMessage(&Message);
        TranslateMessage(&Message);
    }

    Win32GraphicsState->EventArena = 0;
    Win32GraphicsState->EventList = 0;

    return Events;
}

void OSEatEvent(os_event_list *Events, os_event *Event)
{
    DLLRemove(Events->First, Events->Last, Event);
    --Events->Count;
} 

os_handle OSOpenWindow(string8 Title, os_handle Parent, U32 Width, U32 Height, U32 X, U32 Y)
{
    temp_arena Scratch = GetScratch(0, 0);
    string16 Title16 = Str16From8(Scratch.Arena, Title);
    HINSTANCE Instance = Win32GraphicsState->Instance;
    HWND WindowHandle = CreateWindowExW(0, L"graphics_window", (WCHAR *)Title16.String, WS_OVERLAPPEDWINDOW, X, Y,
                                        Width, Height, 0, 0, Instance, 0);
    ReleaseScratch(Scratch);

    win32_window *Window = Win32AllocateWindow();
    Window->Handle = WindowHandle;
    if(Win32GraphicsState->GetDPIForWindowFunc)
        Window->DPI = (F32)Win32GraphicsState->GetDPIForWindowFunc(WindowHandle);
    else
        Window->DPI = 96.0f;

    if(Parent)
    {
        win32_window *ParentWindow = (win32_window *)Parent;
        SetWindowLongPtr(WindowHandle, GWLP_HWNDPARENT, (LONG_PTR)ParentWindow->Handle);
        Window->Parent = ParentWindow;
    }
    
    Window->Parent = (win32_window *)Parent;

    // Initialize tablet functionality for the window
    if(Win32GraphicsState->WinTabSupported)
    {
        if(Win32GraphicsState->WTInfoA(0, 0, 0))
        {   
            LOGCONTEXTA LogContext = {0};
            AXIS RangeX = {0};
            AXIS RangeY = {0};
            AXIS Pressure = {0};

            Win32GraphicsState->WTInfoA(WTI_DDCTXS, 0, &LogContext);
            Win32GraphicsState->WTInfoA(WTI_DEVICES, DVC_X, &RangeX);
            Win32GraphicsState->WTInfoA(WTI_DEVICES, DVC_Y, &RangeY);
            Win32GraphicsState->WTInfoA(WTI_DEVICES, DVC_NPRESSURE, &Pressure);
            
            LogContext.lcPktData = PACKETDATA;
            LogContext.lcOptions |= CXO_MESSAGES | CXO_SYSTEM;
            LogContext.lcPktMode = 0;
            LogContext.lcMoveMask = PACKETDATA;
            LogContext.lcBtnUpMask = LogContext.lcBtnDnMask;

            LogContext.lcOutOrgX = GetSystemMetrics(SM_XVIRTUALSCREEN);
            LogContext.lcOutOrgY = GetSystemMetrics(SM_YVIRTUALSCREEN);
            LogContext.lcOutExtX = GetSystemMetrics(SM_CXVIRTUALSCREEN);
            LogContext.lcOutExtY = -GetSystemMetrics(SM_CYVIRTUALSCREEN);

            Window->WTContext = Win32GraphicsState->WTOpenA(Window->Handle, &LogContext, TRUE);
            if(Window->WTContext)
            {
                Window->WTMaxPressure = Pressure.axMax;
                Window->WTRangeX = RangeX.axMax;
                Window->WTRangeY = RangeY.axMax;
            }
            else
            {
                Error("Couldn't open a WinTab context. Tablet functionality suspended.\n");
            }
        }
        else
        {
            Warning("Tablet functionality (WinTab) services are unsupported.\n");
        }
    }
    
    return (os_handle)Window;
}

void OSCloseWindow(os_handle WindowHandle)
{
    win32_window *Window = (win32_window *)WindowHandle;

    DestroyWindow(Window->Handle);
    
    Win32FreeWindow(Window);
}

void OSWindowEquipRepaint(os_handle WindowHandle, void (*RepaintHook)(os_handle RepaintWindow))
{
    win32_window *Window = (win32_window *)WindowHandle;
    Window->Repaint = RepaintHook;
}

void OSWindowFirstPaint(os_handle WindowHandle)
{
    win32_window *Window = (win32_window *)WindowHandle;
    ShowWindow(Window->Handle, SW_SHOW);
}

rng2_f32 OSClientRectFromWindow(os_handle Handle)
{
    rng2_f32 Result = {0};
    win32_window *Window = (win32_window *)Handle;
    if(Window)
    {
        RECT Rect = {0};
        GetClientRect(Window->Handle, &Rect);
        Result.Left = (F32)Rect.left;
        Result.Right = (F32)Rect.right;
        Result.Top = (F32)Rect.top;
        Result.Bottom = (F32)Rect.bottom;
    }
    return Result;
}

F32 OSDPIFromWindow(os_handle Handle)
{
    F32 Result = 0.0f;
    win32_window *Window = (win32_window *)Handle;
    if(Window)
        Result = Window->DPI;
    return Result;
}

B8 OSWindowIsFocused(os_handle Handle)
{
    win32_window *Window = (win32_window *)Handle;
    HWND ActiveHandle = GetActiveWindow();
    return ActiveHandle == Window->Handle;
}

void OSWindowFocus(os_handle Handle)
{
    win32_window *Window = (win32_window *)Handle;
    SetForegroundWindow(Window->Handle);
    SetFocus(Window->Handle);
}

vec2_f32 OSMouseFromWindow(os_handle Handle)
{
    vec2_f32 Result = {0};
    POINT Point;
    if(GetCursorPos(&Point))
    {
        win32_window *Window = (win32_window *)Handle;
        ScreenToClient(Window->Handle, &Point);
        Result.X = (F32)Point.x;
        Result.Y = (F32)Point.y;
    }
    return Result;
}

vec2_f32 OSGetMouse()
{
    vec2_f32 Result = {0};
    POINT Point;
    if(GetCursorPos(&Point))
    {
        Result.X = (F32)Point.x;
        Result.Y = (F32)Point.y;
    }
    return Result;
}

os_tablet_state OSGetTabletState()
{
    return Win32GraphicsState->TabletState;
}

B8 OSGetKey(os_key Key)
{
    SHORT State = GetAsyncKeyState((I32)Win32VKeyFromOSKey(Key));
    B8 Pressed = (State & (0x8000)) != 0;
    return Pressed;
}

B8 OSGetKeyFlags(os_key Key, os_event_flags Flags)
{
    B8 Pressed = OSGetKey(Key);
    B8 ActualFlags = 0;
    if(Pressed)
    {
        if(GetAsyncKeyState(VK_CONTROL) & 0x8000) ActualFlags |= OS_EVENT_FLAG_CTRL;
        if(GetAsyncKeyState(VK_SHIFT) & 0x8000) ActualFlags |= OS_EVENT_FLAG_SHIFT;
        if(GetAsyncKeyState(VK_MENU) & 0x8000) ActualFlags |= OS_EVENT_FLAG_ALT;
    }
    return Pressed && ActualFlags == Flags;
}

string8 OSFileOpenDialog(arena *Arena, os_handle WindowHandle)
{
    string8 Path = {0};
    
    IFileOpenDialog *Dialog = 0;
    IShellItem *Item = 0;
    U16 *FilePath = 0;

    win32_window *Window = (win32_window *)WindowHandle;
    
    CoInitializeEx(0, COINIT_APARTMENTTHREADED);
    HRESULT Error = CoCreateInstance(&CLSID_FileOpenDialog, 0, CLSCTX_ALL, &IID_IFileOpenDialog, (void **)&Dialog);
    COMDLG_FILTERSPEC Extensions[] = {{L"Open", L"*.*"}};

    if(Dialog)
    {
        if(SUCCEEDED(Error))
            Error = IFileOpenDialog_SetFileTypes(Dialog, 1, Extensions);
        if(SUCCEEDED(Error))
            Error = IFileOpenDialog_Show(Dialog, Window ? Window->Handle : 0);
        if(SUCCEEDED(Error))
            Error = IFileOpenDialog_GetResult(Dialog, &Item); // TODO: Use GetResults instead
        if(SUCCEEDED(Error))
            Error = IShellItem_GetDisplayName(Item, SIGDN_FILESYSPATH, &FilePath);
        if(SUCCEEDED(Error))
        {
            U64 Length = 0;
            while(FilePath[Length]) ++Length;
            Path = Str8From16(Arena, Str16(FilePath, Length));
        }
    }

    // Clean up
    if(Item) IShellItem_Release(Item);
    if(Dialog) IFileOpenDialog_Release(Dialog);

    CoUninitialize();

    return Path;
}

void OSMessageBox(string8 Caption, string8 Message, os_message_box_type Type, os_handle WindowHandle)
{
    win32_window *Window = (win32_window *)WindowHandle;
    
    U32 OSType = MB_OK;
    switch(Type)
    {
        case OS_MESSAGE_BOX_TYPE_INFO: OSType += MB_ICONINFORMATION; break;
        case OS_MESSAGE_BOX_TYPE_ERROR: OSType += MB_ICONERROR; break;
        case OS_MESSAGE_BOX_TYPE_WARNING: OSType += MB_ICONWARNING; break;
    }
    
    temp_arena Scratch = GetScratch(0, 0);
    string16 Caption16 = Str16From8(Scratch.Arena, Caption);
    string16 Message16 = Str16From8(Scratch.Arena, Message);
    MessageBoxW(Window ? Window->Handle : 0, Message16.String, Caption16.String, OSType);
    ReleaseScratch(Scratch);
}

WPARAM Win32VKeyFromOSKey(os_key Key)
{
    local_persist B32 First = 1;
    local_persist WPARAM KeyTable[OS_KEY_COUNT];
    if(First)
    {
        First = 0;
        MemoryZero(KeyTable, sizeof(KeyTable));

        KeyTable[OS_KEY_A] = 'A';
        KeyTable[OS_KEY_B] = 'B';
        KeyTable[OS_KEY_C] = 'C';
        KeyTable[OS_KEY_D] = 'D';
        KeyTable[OS_KEY_E] = 'E';
        KeyTable[OS_KEY_F] = 'F';
        KeyTable[OS_KEY_G] = 'G';
        KeyTable[OS_KEY_H] = 'H';
        KeyTable[OS_KEY_I] = 'I';
        KeyTable[OS_KEY_J] = 'J';
        KeyTable[OS_KEY_K] = 'K';
        KeyTable[OS_KEY_L] = 'L';
        KeyTable[OS_KEY_M] = 'M';
        KeyTable[OS_KEY_N] = 'N';
        KeyTable[OS_KEY_O] = 'O';
        KeyTable[OS_KEY_P] = 'P';
        KeyTable[OS_KEY_Q] = 'Q';
        KeyTable[OS_KEY_R] = 'R';
        KeyTable[OS_KEY_S] = 'S';
        KeyTable[OS_KEY_T] = 'T';
        KeyTable[OS_KEY_U] = 'U';
        KeyTable[OS_KEY_V] = 'V';
        KeyTable[OS_KEY_W] = 'W';
        KeyTable[OS_KEY_X] = 'X';
        KeyTable[OS_KEY_Y] = 'Y';
        KeyTable[OS_KEY_Z] = 'Z';

        for(U64 WinKey = '0', OSKey = OS_KEY_0; WinKey <= '9'; ++WinKey, ++OSKey)
            KeyTable[OSKey] = (WPARAM)WinKey;

        for(U64 WinKey = VK_F1, OSKey = OS_KEY_0; WinKey <= VK_F24; ++WinKey, ++OSKey)
            KeyTable[OSKey] = (WPARAM)WinKey;

        KeyTable[OS_KEY_SPACE] = VK_SPACE;
        KeyTable[OS_KEY_TICK] = VK_OEM_3;
        KeyTable[OS_KEY_MINUS] = VK_OEM_MINUS;
        KeyTable[OS_KEY_EQUAL] = VK_OEM_PLUS;
        KeyTable[OS_KEY_LEFT_BRACKET] = VK_OEM_4;
        KeyTable[OS_KEY_RIGHT_BRACKET] = VK_OEM_6;
        KeyTable[OS_KEY_SEMICOLON] = VK_OEM_1;
        KeyTable[OS_KEY_QUOTE] = VK_OEM_7;
        KeyTable[OS_KEY_COMMA] = VK_OEM_COMMA;
        KeyTable[OS_KEY_PERIOD] = VK_OEM_PERIOD;
        KeyTable[OS_KEY_SLASH] = VK_OEM_2;
        KeyTable[OS_KEY_BACKSLASH] = VK_OEM_5;
        KeyTable[OS_KEY_TAB] = VK_TAB;
        KeyTable[OS_KEY_PAUSE] = VK_PAUSE;
        KeyTable[OS_KEY_ESC] = VK_ESCAPE;
        KeyTable[OS_KEY_UP] = VK_UP;
        KeyTable[OS_KEY_DOWN] = VK_DOWN;
        KeyTable[OS_KEY_LEFT] = VK_LEFT;
        KeyTable[OS_KEY_RIGHT] = VK_RIGHT;
        KeyTable[OS_KEY_BACKSPACE] = VK_BACK;
        KeyTable[OS_KEY_RETURN] = VK_RETURN;
        KeyTable[OS_KEY_DELETE] = VK_DELETE;
        KeyTable[OS_KEY_INSERT] = VK_INSERT;
        KeyTable[OS_KEY_PAGE_UP] = VK_PRIOR;
        KeyTable[OS_KEY_PAGE_DOWN] = VK_NEXT;
        KeyTable[OS_KEY_HOME] = VK_HOME;
        KeyTable[OS_KEY_END] = VK_END;
        KeyTable[OS_KEY_CAPS_LOCK] = VK_CAPITAL;
        KeyTable[OS_KEY_NUM_LOCK] = VK_NUMLOCK;
        KeyTable[OS_KEY_SCROLL_LOCK] = VK_SCROLL;
        KeyTable[OS_KEY_MENU] = VK_APPS;
        KeyTable[OS_KEY_CTRL] = VK_CONTROL;
        KeyTable[OS_KEY_SHIFT] = VK_SHIFT;
        KeyTable[OS_KEY_ALT] = VK_MENU;

        KeyTable[OS_KEY_LEFT_MOUSE_BUTTON] = VK_LBUTTON;
        KeyTable[OS_KEY_MIDDLE_MOUSE_BUTTON] = VK_MBUTTON;
        KeyTable[OS_KEY_RIGHT_MOUSE_BUTTON] = VK_RBUTTON;
    }

    WPARAM Result = KeyTable[Key];
    return Result;
}

os_key Win32OSKeyFromVKey(WPARAM VKey)
{
    local_persist B32 First = 1;
    local_persist os_key KeyTable[256];
    if(First)
    {
        First = 0;
        MemoryZero(KeyTable, sizeof(KeyTable));

        KeyTable['A'] = OS_KEY_A;
        KeyTable['B'] = OS_KEY_B;
        KeyTable['C'] = OS_KEY_C;
        KeyTable['D'] = OS_KEY_D;
        KeyTable['E'] = OS_KEY_E;
        KeyTable['F'] = OS_KEY_F;
        KeyTable['G'] = OS_KEY_G;
        KeyTable['H'] = OS_KEY_H;
        KeyTable['I'] = OS_KEY_I;
        KeyTable['J'] = OS_KEY_J;
        KeyTable['K'] = OS_KEY_K;
        KeyTable['L'] = OS_KEY_L;
        KeyTable['M'] = OS_KEY_M;
        KeyTable['N'] = OS_KEY_N;
        KeyTable['O'] = OS_KEY_O;
        KeyTable['P'] = OS_KEY_P;
        KeyTable['Q'] = OS_KEY_Q;
        KeyTable['R'] = OS_KEY_R;
        KeyTable['S'] = OS_KEY_S;
        KeyTable['T'] = OS_KEY_T;
        KeyTable['U'] = OS_KEY_U;
        KeyTable['V'] = OS_KEY_V;
        KeyTable['W'] = OS_KEY_W;
        KeyTable['X'] = OS_KEY_X;
        KeyTable['Y'] = OS_KEY_Y;
        KeyTable['Z'] = OS_KEY_Z;

        for(U64 WinKey = '0', OSKey = OS_KEY_0; WinKey <= '9'; ++WinKey, ++OSKey)
            KeyTable[WinKey] = (os_key)OSKey;
        
        for(U64 WinKey = VK_F1, OSKey = OS_KEY_0; WinKey <= VK_F24; ++WinKey, ++OSKey)
            KeyTable[WinKey] = (os_key)OSKey;

        KeyTable[VK_SPACE] = OS_KEY_SPACE;
        KeyTable[VK_OEM_3] = OS_KEY_TICK;
        KeyTable[VK_OEM_MINUS] = OS_KEY_MINUS;
        KeyTable[VK_OEM_PLUS] = OS_KEY_EQUAL;
        KeyTable[VK_OEM_4] = OS_KEY_LEFT_BRACKET;
        KeyTable[VK_OEM_6] = OS_KEY_RIGHT_BRACKET;
        KeyTable[VK_OEM_1] = OS_KEY_SEMICOLON;
        KeyTable[VK_OEM_7] = OS_KEY_QUOTE;
        KeyTable[VK_OEM_COMMA] = OS_KEY_COMMA;
        KeyTable[VK_OEM_PERIOD] = OS_KEY_PERIOD;
        KeyTable[VK_OEM_2] = OS_KEY_SLASH;
        KeyTable[VK_OEM_5] = OS_KEY_BACKSLASH;
        KeyTable[VK_TAB] = OS_KEY_TAB;
        KeyTable[VK_PAUSE] = OS_KEY_PAUSE;
        KeyTable[VK_ESCAPE] = OS_KEY_ESC;
        KeyTable[VK_UP] = OS_KEY_UP;
        KeyTable[VK_DOWN] = OS_KEY_DOWN;
        KeyTable[VK_LEFT] = OS_KEY_LEFT;
        KeyTable[VK_RIGHT] = OS_KEY_RIGHT;
        KeyTable[VK_BACK] = OS_KEY_BACKSPACE;
        KeyTable[VK_RETURN] = OS_KEY_RETURN;
        KeyTable[VK_DELETE] = OS_KEY_DELETE;
        KeyTable[VK_INSERT] = OS_KEY_INSERT;
        KeyTable[VK_PRIOR] = OS_KEY_PAGE_UP;
        KeyTable[VK_NEXT] = OS_KEY_PAGE_DOWN;
        KeyTable[VK_HOME] = OS_KEY_HOME;
        KeyTable[VK_END] = OS_KEY_END;
        KeyTable[VK_CAPITAL] = OS_KEY_CAPS_LOCK;
        KeyTable[VK_NUMLOCK] = OS_KEY_NUM_LOCK;
        KeyTable[VK_SCROLL] = OS_KEY_SCROLL_LOCK;
        KeyTable[VK_APPS] = OS_KEY_MENU;
        KeyTable[VK_CONTROL] = OS_KEY_CTRL;
        KeyTable[VK_LCONTROL] = OS_KEY_CTRL;
        KeyTable[VK_RCONTROL] = OS_KEY_CTRL;
        KeyTable[VK_SHIFT] = OS_KEY_SHIFT;
        KeyTable[VK_LSHIFT] = OS_KEY_SHIFT;
        KeyTable[VK_RSHIFT] = OS_KEY_SHIFT;
        KeyTable[VK_MENU] = OS_KEY_ALT;
        KeyTable[VK_LMENU] = OS_KEY_ALT;
        KeyTable[VK_RMENU] = OS_KEY_ALT;

        KeyTable[VK_LBUTTON] = OS_KEY_LEFT_MOUSE_BUTTON;
        KeyTable[VK_MBUTTON] = OS_KEY_MIDDLE_MOUSE_BUTTON;
        KeyTable[VK_RBUTTON] = OS_KEY_RIGHT_MOUSE_BUTTON;
    }

    os_key Key = KeyTable[VKey & 0x000000ff];
    return Key;
}
