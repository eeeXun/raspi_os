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
