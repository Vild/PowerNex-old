#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#include <powernex/powernex.h>

enum MOD {
	CONTROL       = 0x01,
	ALT           = 0x02,
	ALTGR         = 0x04,
	LSHIFT        = 0x08,
	RSHIFT        = 0x10,
	CAPSLOCK      = 0x20,
	SCROLLLOCK    = 0x40,
	NUMLOCK       = 0x80,
	RELEASED_MASK = 0x80
};

// Bitmap for modifiers
typedef uint8_t kb_modStatus_t;

typedef struct kb_keyMap {
  uint8_t scancodes[128];
  uint8_t shiftScancodes[128];

  uint8_t controlMap[8];

  kb_modStatus_t modifiers;
} kb_keyMap_t;

void kb_init();
void kb_switchLayout(kb_keyMap_t * layout);

// Returns '\0' if no character is available.
char kb_getc();



#endif
