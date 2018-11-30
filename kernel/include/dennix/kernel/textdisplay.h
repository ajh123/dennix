/* Copyright (c) 2018 Dennis Wölfing
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* kernel/include/dennix/kernel/textdisplay.h
 * Abstract text display class.
 */

#ifndef KERNEL_TEXTDISPLAY_H
#define KERNEL_TEXTDISPLAY_H

#include <dennix/kernel/kernel.h>

struct CharPos {
    unsigned int x;
    unsigned int y;

    bool operator==(const CharPos& p) { return p.x == x && p.y == y; }
};

class TextDisplay {
public:
    unsigned int height() { return _height; }
    unsigned int width() { return _width; }
    virtual void clear(CharPos from, CharPos to, uint8_t color) = 0;
    virtual void putCharacter(CharPos position, wchar_t c, uint8_t color) = 0;
    virtual void scroll(unsigned int lines, uint8_t color, bool up = true) = 0;
    virtual void setCursorPos(CharPos position) = 0;
    virtual ~TextDisplay() {}
protected:
    unsigned int _height;
    unsigned int _width;
};

uint8_t unicodeToCp437(wchar_t wc);

#endif
