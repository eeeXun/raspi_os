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
