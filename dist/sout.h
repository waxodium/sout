/*
Copyright (C) 2026 waxodium <waxodium@proton.me>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdarg.h>
#include <limits.h>

#if defined(_WIN32)
    #include <windows.h>
#else
    #include <unistd.h>
#endif

typedef struct {
    char data[1024];
    int position;
} OutputBuffer;

typedef struct {
    int prefix;
    int zero_pad;
    int width;

    int precision;
    int has_precision;

    char specifier;
} FormatTable;

void write_chunk(const char *buf, int length) {
    if (length <= 0)
        return;

#if defined(_WIN32)
    DWORD written;
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), buf, length, &written, NULL);
#else
    write(1, buf, length);
#endif
}

void flush(OutputBuffer *buffer) {
    if (buffer->position > 0) {
        write_chunk(buffer->data, buffer->position);
        buffer->position = 0;
    }
}

void addchar(OutputBuffer *buffer, char c) {
    if (buffer->position >= 1023)
        flush(buffer);

    buffer->data[buffer->position++] = c;
}

FormatTable parse_format(const char **cursor, va_list *arguments) {
    FormatTable format = {0};
    (*cursor)++;

    while (**cursor == '#' || **cursor == '0') {
        if (**cursor == '#')
            format.prefix = 1;
        else
            format.zero_pad = 1;

        (*cursor)++;
    }

    while (**cursor >= '0' && **cursor <= '9') {
        format.width = format.width * 10 + (**cursor - '0');
        (*cursor)++;
    }

    if (**cursor == '.') {
        format.has_precision = 1;

        (*cursor)++;

        if (**cursor == '*') {
            format.precision = va_arg(*arguments, int);

            if (format.precision < 0) {
                format.has_precision = 0;
                format.precision = 0;
            }

            (*cursor)++;
        } else {
            while (**cursor >= '0' && **cursor <= '9') {
                format.precision = format.precision * 10 + (**cursor - '0');
                (*cursor)++;
            }
        }
    }

    format.specifier = **cursor;

    return format;
}

void soutf(const char *format, ...) {
    OutputBuffer stream = {
        .position = 0
    };

    va_list arguments;
    va_start(arguments, format);

    for (const char *cursor = format; *cursor != '\0'; cursor++) {

        if (stream.position >= 1023)
            flush(&stream);

        if (*cursor == '%' && *(cursor + 1) != '\0') {

            FormatTable spec = parse_format(&cursor, &arguments);

            switch (spec.specifier) {
                case 's': {
                    const char *text = va_arg(arguments, const char *);

                    if (text == NULL)
                        text = "(null)";

                    int length = 0;

                    if (spec.has_precision) {

                        while (text[length] != '\0' && length < spec.precision) {
                            length++;
                        }

                    } else {

                        while (text[length] != '\0')
                            length++;
                    }

                    for (int i = 0; i < length; i++)
                        addchar(&stream, text[i]);

                    break;
                }
                case 'd': {
                    int number = va_arg(arguments, int);

                    char digits[sizeof(unsigned int) * CHAR_BIT];

                    int p = 0;
                    unsigned int num;

                    if (number == 0) {

                        addchar(&stream, '0');

                    } else {
                        if (number < 0) {
                            addchar(&stream, '-');
                            num = (unsigned int)(-(number + 1)) + 1;

                        } else {
                            num = (unsigned int)number;
                        }

                        while (num > 0) {
                            digits[p++] = (num % 10) + '0';
                            num /= 10;
                        }

                        while (p > 0)
                            addchar(&stream, digits[--p]);
                    }

                    break;
                }

                case 'u': {
                    unsigned int number = va_arg(arguments, unsigned int);
                    char digits[sizeof(unsigned int) * CHAR_BIT];
                    int p = 0;
                    
                    if (number == 0) {
                        addchar(&stream, '0');
                    } else {

                        while (number > 0) {
                            digits[p++] = (number % 10) + '0';
                            number /= 10;
                        }

                        while (p > 0) addchar(&stream, digits[--p]);
                    }

                    break;
                }

                case 'x':
                case 'X': {
                    unsigned int number = va_arg(arguments, unsigned int);
                    unsigned int original = number;
                    char digits[sizeof(unsigned int) * 2];
                    int p = 0;

                    if (number == 0) {
                        digits[p++] = '0';
                    } else {
                        
                        while (number > 0) {
                            unsigned int digit = number % 16;

                            if (digit < 10) {
                                digits[p++] = '0' + digit;
                            } else if (spec.specifier == 'x') {

                                digits[p++] = 'a' + (digit - 10);

                            } else {

                                digits[p++] = 'A' + (digit - 10);
                            }

                            number /= 16;
                        }
                    }

                    int prefix = spec.prefix && original != 0 ? 2 : 0;
                    int padding = spec.width - p - prefix;

                    if (padding < 0) padding = 0;

                    if (spec.zero_pad) {

                        if (prefix) {
                            addchar(&stream, '0');
                            addchar(&stream, spec.specifier);
                        }

                        while (padding-- > 0)
                            addchar(&stream, '0');

                    } else {

                        while (padding-- > 0)
                            addchar(&stream, ' ');

                        if (prefix) {
                            addchar(&stream, '0');
                            addchar(&stream, spec.specifier);
                        }
                    }

                    while (p > 0)
                        addchar(&stream, digits[--p]);

                    break;
                }
 
                case 'c': {
                    addchar(&stream, (char)va_arg(arguments, int));
                    break;
                }

                case '%': {
                    addchar(&stream, '%');
                    break;
                }

                default: {
                    addchar(&stream, '%');
                    addchar(&stream, *cursor);
                    break;
                }
            }
        } else {

            addchar(&stream, *cursor);
        }
    }

    flush(&stream);

    va_end(arguments);
}


// &&&&&& Header &&&&&&

void write_chunk(const char *buf, int length);
void soutf(const char *format, ...);

#include <stddef.h>
#include <string.h>

int soutspan(const char *src, const char *start, const char *end, char *destination, size_t cap) {
    if (!src || !destination || cap == 0)
        return 0;

    const char *a = src;

    if (start && *start) {
        a = strstr(src, start);
        if (!a)
            return 0;

        a += strlen(start);
    }

    const char *b = a + strlen(a);

    if (end && *end) {
        b = strstr(a, end);
        if (!b)
            return 0;
    }

    size_t len = (size_t)(b - a);

    if (len + 1 > cap)
        return 0;

    memcpy(destination, a, len);
    destination[len] = '\0';

    return 1;
}

int soutview(const char *src, const char *start, const char *end, const char **out, size_t *length) {
    if (!src || !out || !length)
        return 0;

    const char *a = src;

    if (start && *start) {
        a = strstr(src, start);
        if (!a)
            return 0;

        a += strlen(start);
    }

    const char *b = a + strlen(a);

    if (end && *end) {
        b = strstr(a, end);
        if (!b)
            return 0;
    }

    *out = a;
    *length = (size_t)(b - a);

    return 1;
}

int soutviewc(const char *src, char start, char end, const char **out, size_t *length) {
    if (!src || !out || !length)
        return 0;

    const char *ahh = src;

    if (start != '\0') {
        ahh = strchr(src, start);
        if (!ahh)
            return 0;

        ahh++;
    }

    const char *bih = ahh + strlen(ahh);

    if (end != '\0') {
        bih = strchr(ahh, end);
        if (!bih)
            return 0;
    }

    *out = ahh;
    *length = (size_t)(bih - ahh);

    return 1;
}

int soutsplit(const char **src, char separator, const char **out, size_t *length) {
    if (!src || !*src || !out || !length)
        return 0;

    const char *anh = *src;

    if (*anh == '\0')
        return 0;

    const char *b = strchr(anh, separator);

    if (b) {
        *out = anh;
        *length = (size_t)(b - anh);
        *src = b + 1;
    } else {
        *out = anh;
        *length = strlen(anh);
        *src = anh + *length;
    }

    return 1;
}

int souttrim(const char **src, size_t *length) {
    if (!src || !*src || !length)
        return 0;

    const char *a = *src;
    const char *b = a + *length;

    while (a < b && (*a == ' ' || *a == '\t' || *a == '\n' || *a == '\r'))
        a++;

    while (b > a && (*(b - 1) == ' ' || *(b - 1) == '\t' || *(b - 1) == '\n' || *(b - 1) == '\r'))
        b--;

    *src = a;
    *length = (size_t)(b - a);

    return 1;
}

// %%%%%%%%% Header %%%%%%%%%
int soutspan(const char *src, const char *start, const char *end, char *destination, size_t cap);
int soutview(const char *src, const char *start, const char *end, const char **out, size_t *length);
int soutviewc(const char *src, char start, char end, const char **out, size_t *length);
int soutsplit(const char **src, char separator, const char **out, size_t *length);
int souttrim(const char **src, size_t *length);
