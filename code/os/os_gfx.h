#if !defined(OS_GFX_H)
/*  =======================================================================
    File: os_gfx.h
    Date: March 6th 2024 12:57 AM
    Creator: Quinn Van De Keere
    =======================================================================*/
#define OS_GFX_H

#include "generated/os_gfx.meta.h"

typedef enum os_event_kind
{
    OS_EVENT_KIND_WINDOW_DESTROY,
    OS_EVENT_KIND_PRESS,
    OS_EVENT_KIND_RELEASE,
    OS_EVENT_KIND_WINDOW_LOSE_FOCUS,
    OS_EVENT_KIND_SCROLL,
    OS_EVENT_KIND_TABLET,
}os_event_kind;

typedef enum os_key
{
    OS_KEY_NULL,

    OS_KEY_ESC,
    
    OS_KEY_F1,
    OS_KEY_F2,
    OS_KEY_F3,
    OS_KEY_F4,
    OS_KEY_F5,
    OS_KEY_F6,
    OS_KEY_F7,
    OS_KEY_F8,
    OS_KEY_F9,
    OS_KEY_F10,
    OS_KEY_F11,
    OS_KEY_F12,
    OS_KEY_F13,
    OS_KEY_F14,
    OS_KEY_F15,
    OS_KEY_F16,
    OS_KEY_F17,
    OS_KEY_F18,
    OS_KEY_F19,
    OS_KEY_F20,
    OS_KEY_F21,
    OS_KEY_F22,
    OS_KEY_F23,
    OS_KEY_F24,

    OS_KEY_TICK,

    OS_KEY_0,
    OS_KEY_1,
    OS_KEY_2,
    OS_KEY_3,
    OS_KEY_4,
    OS_KEY_5,
    OS_KEY_6,
    OS_KEY_7,
    OS_KEY_8,
    OS_KEY_9,

    OS_KEY_MINUS,
    OS_KEY_EQUAL,
    OS_KEY_BACKSPACE,
    OS_KEY_TAB,

    OS_KEY_A,
    OS_KEY_B,
    OS_KEY_C,
    OS_KEY_D,
    OS_KEY_E,
    OS_KEY_F,
    OS_KEY_G,
    OS_KEY_H,
    OS_KEY_I,
    OS_KEY_J,
    OS_KEY_K,
    OS_KEY_L,
    OS_KEY_M,
    OS_KEY_N,
    OS_KEY_O,
    OS_KEY_P,
    OS_KEY_Q,
    OS_KEY_R,
    OS_KEY_S,
    OS_KEY_T,
    OS_KEY_U,
    OS_KEY_V,
    OS_KEY_W,
    OS_KEY_X,
    OS_KEY_Y,
    OS_KEY_Z,

    OS_KEY_LEFT_BRACKET,
    OS_KEY_RIGHT_BRACKET,
    OS_KEY_BACKSLASH,
    OS_KEY_CAPS_LOCK,
    OS_KEY_SEMICOLON,
    OS_KEY_QUOTE,
    OS_KEY_RETURN,
    OS_KEY_SHIFT,
    OS_KEY_COMMA,
    OS_KEY_PERIOD,
    OS_KEY_SLASH,
    OS_KEY_CTRL,
    OS_KEY_ALT,
    OS_KEY_SPACE,
    OS_KEY_MENU,
    OS_KEY_SCROLL_LOCK,
    OS_KEY_NUM_LOCK,
    OS_KEY_PAUSE,
    OS_KEY_INSERT,
    OS_KEY_HOME,
    OS_KEY_PAGE_UP,
    OS_KEY_PAGE_DOWN,
    OS_KEY_UP,
    OS_KEY_LEFT,
    OS_KEY_DOWN,
    OS_KEY_RIGHT,
    OS_KEY_DELETE,
    OS_KEY_END,
    
    OS_KEY_LEFT_MOUSE_BUTTON,
    OS_KEY_MIDDLE_MOUSE_BUTTON,
    OS_KEY_RIGHT_MOUSE_BUTTON,

    OS_KEY_COUNT
}os_key;

typedef enum os_event_flags
{
    OS_EVENT_FLAG_CTRL = (1<<0),
    OS_EVENT_FLAG_SHIFT = (1<<1),
    OS_EVENT_FLAG_ALT = (1<<2)
}os_event_flags;

typedef struct os_event os_event;
struct os_event
{
    os_event *Next;
    os_event *Prev;

    os_event_kind Kind;
    os_handle Window;
    os_event_flags Flags;

    vec2_f32 Position;
    vec2_f32 Delta;
    
    os_key Key;
    B32 IsRepeat;
    B32 RightSided;
    B32 Character;
    B32 RepeatCount;

    F32 Pressure;
};

typedef struct os_event_list os_event_list;
struct os_event_list
{
    os_event *First;
    os_event *Last;
    U64 Count;
};

typedef enum os_cursor
{
    OS_CURSOR_POINTER,
    OS_CURSOR_IBAR,
    OS_CURSOR_LEFT_RIGHT,
    OS_CURSOR_UP_DOWN,
    OS_CURSOR_DOWN_RIGHT,
    OS_CURSOR_UP_RIGHT,
    OS_CURSOR_UP_DOWN_LEFT_RIGHT,
    OS_CURSOR_HAND_POINT,
    OS_CURSOR_HAND_GRAB,
    OS_CURSOR_HAND_ROTATE,
    OS_CURSOR_ZOOM_IN,
    OS_CURSOR_DISABLED,
    OS_CURSOR_COUNT
}os_cursor;

typedef enum os_message_box_type
{
    OS_MESSAGE_BOX_TYPE_INFO,
    OS_MESSAGE_BOX_TYPE_ERROR,
    OS_MESSAGE_BOX_TYPE_WARNING
}os_message_box_type;

typedef enum os_cursor_image_type
{
    OS_CURSOR_IMAGE_TYPE_MONO,
    OS_CURSOR_IMAGE_TYPE_PNG,
    OS_CURSOR_IMAGE_TYPE_COLOR,
}os_cursor_image_type;

typedef struct os_cursor_image os_cursor_image;
struct os_cursor_image
{
    os_cursor_image *Next;
    os_cursor_image *Prev;

    os_cursor_image_type Type;
    
    U8 Width;
    U8 Height;
    U8 PaletteColors;
    U16 HotspotX;
    U16 HotspotY;
    U32 DataSize;
    U32 Offset;
    void *ImageData;
};

typedef struct os_cursor_format os_cursor_format;
struct os_cursor_format
{
    U16 NumImages;
    
    os_cursor_image *FirstImage;
    os_cursor_image *LastImage;
};

typedef struct os_tablet_state os_tablet_state;
struct os_tablet_state
{
    vec2_f32 Position;
    F32 Pressure;
};

os_cursor_format OSLoadCursorFromBytes(arena *Arena, U8 *Bytes, U64 Size);

void OSGraphicsStartUp();
os_handle OSOpenWindow(string8 Title, os_handle Parent, U32 Width, U32 Height, U32 X, U32 Y);
os_event_list OSGetEvents(arena *Arena);
void OSEatEvent(os_event_list *Events, os_event *Event);
void OSCloseWindow(os_handle WindowHandle);
void OSWindowEquipRepaint(os_handle WindowHandle, void (*RepaintHook)(os_handle RepaintWindow));
void OSWindowFirstPaint(os_handle WindowHandle);
rng2_f32 OSClientRectFromWindow(os_handle Handle);
F32 OSDPIFromWindow(os_handle Handle);
vec2_f32 OSMouseFromWindow(os_handle Handle);
vec2_f32 OSGetMouse();
string8 OSFileOpenDialog(arena *Arena, os_handle WindowHandle);
void OSSetCursor(os_cursor Cursor);
B8 OSWindowIsFocused(os_handle Handle);
F32 OSDefaultRefreshRate();
void OSMessageBox(string8 Caption, string8 Message, os_message_box_type Type, os_handle WindowHandle);
B8 OSGetKeyFlags(os_key Key, os_event_flags Flags);
B8 OSGetKey(os_key Key);
os_tablet_state OSGetTabletState();

#endif
