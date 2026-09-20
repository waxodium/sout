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
