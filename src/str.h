int strlen(const char *str)
{
    int len = 0;
    while (str[len] != '\0')
        len++;
    return len;
}

int strncmp(char *str1, char *str2, int n)
{
    for (int i = 0; i < n; i++) {
        if (str1[i] < str2[i])
            return -1;
        else if (str1[i] > str2[i])
            return 1;
    }
    return 0;
} 
char* strcpy(char *dest, const char *src)
{
    int len = strlen(src);
    for (int i = 0; i <= len; i++)
        dest[i] = src[i];
    return dest;
}

char* strcat(char *dest, const char *src)
{
    int len = strlen(src);
    strcpy(&dest[len], src);
    return dest;
}

