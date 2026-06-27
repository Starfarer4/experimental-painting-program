#if !defined(UI_ANIMATION_H)
/*  =======================================================================
    File: ui_animation.h
    Date: May 1st 2024 10:54 PM
    Creator: Quinn Van De Keere
    =======================================================================*/
#define UI_ANIMATION_H

typedef enum ui_animation_rate
{
    UI_ANIMATION_RATE_FAST,
    UI_ANIMATION_RATE_SLOW,
    UI_ANIMATION_RATE_SLUG,
    UI_ANIMATION_RATE_SLOTH
}ui_animation_rate;

typedef struct ui_animated ui_animated;
struct ui_animated
{
    ui_animated *Prev;
    ui_animated *Next;
    
    ui_key Key;
    F32 T;
    ui_animation_rate Rate;

    U64 LastTouchedIndex;
};

typedef struct ui_animated_hash_slot ui_animated_hash_slot;
struct ui_animated_hash_slot
{
    ui_animated *First;
    ui_animated *Last;
};

typedef struct ui_animation_state ui_animation_state;
struct ui_animation_state
{
    ui_animated_hash_slot *AnimatedTable;
    U64 AnimatedTableSize;

    ui_animated *FirstFreeAnimated;
    ui_animated *LastFreeAnimated;

    F32 AnimationDelta;
    
    U64 AnimateIndex;
    
    arena *Arena;
};

void UIBeginAnimation(F32 AnimationDelta);
void UIEndAnimation();
ui_animation_state *UIAnimationStateAlloc();
void UIAnimationStateRelease(ui_animation_state *State);
void UISelectAnimationState(ui_animation_state *State);
ui_animated *UIGetAnimatedFromKey(ui_key Key);
ui_key UIAnimated(ui_animation_rate Rate, B8 Active, ui_key Key);
ui_key UIAnimatedFromString(ui_animation_rate Rate, B8 Active, string8 String);
ui_key UIAnimatedFromStringFmt(ui_animation_rate Rate, B8 Active, string8 String, ...);
ui_key UIAnimatedKeyFromStringFmt(string8 String, ...);
ui_key UIAnimatedFromString(ui_animation_rate Rate, B8 Active, string8 String);

#endif
