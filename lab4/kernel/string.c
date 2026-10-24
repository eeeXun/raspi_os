int atoi(char* s)
{
    int ret = 0;
    int neg = 0;
    while (*s == ' ')
        s++;
    if (*s == '-') {
        neg = 1;
        s++;
    }
    while (*s >= '0' && *s <= '9') {
        ret *= 10;
        ret += (*s - '0');
        s++;
    }
    if (neg)
        ret = -ret;
    return ret;
}

int digit_value(char c, int base)
{
    int ret;
    if (c >= '0' && c <= '9')
        ret = c - '0';
    else if (c >= 'a' && c <= 'z')
        ret = c - 'a' + 10;
    else if (c >= 'A' && c <= 'Z')
        ret = c - 'A' + 10;
    else
        return -1;
    return ret < base ? ret : -1;
}

long long strtol(char* s)
{
    long long ret = 0;
    int neg = 0;
    int base = 10;
    while (*s == ' ')
        s++;
    if (*s == '-') {
        neg = 1;
        s++;
    }
    if (*s == '0') {
        s++;
        if ((*s == 'b') || (*s == 'B')) {
            base = 2;
            s++;
        } else if ((*s == 'x') || (*s == 'X')) {
            base = 16;
            s++;
        } else {
            base = 8;
        }
    }
    for (int digit; (digit = digit_value(*s, base)) >= 0; s++) {
        ret *= base;
        ret += digit;
    }
    if (neg)
        ret = -ret;
    return ret;
}

int strlen(char* s)
{
    int len = 0;
    while (*s) {
        len++;
        s++;
    }
    return len;
}

int strcmp(char* s1, char* s2)
{
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

int strncmp(char* s1, char* s2, int n)
{
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0)
        return 0;
    return (unsigned char)*s1 - (unsigned char)*s2;
}

int strncpy(char* dst, char* src, int n)
{
    if (n == 0)
        return *src ? -1 : 0;
    while (n > 1 && *src) {
        *dst++ = *src++;
        n--;
    }
    if (*src)
        return -1;
    *dst = '\0';
    return 0;
}
