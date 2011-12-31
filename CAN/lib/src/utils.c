int itoh(int n, char * h1, char * h2)
{
    if (n < 0 || n > 255) {
        return -1;
    }

    int d1 = n >> 4;
	int d2 = n % 16;

    if (d1 < 10)
        *h1 = d1 + '0';
    else
        *h1 = d1 - 10 + 'A';

    if (d2 < 10)
        *h2 = d2 + '0';
    else
        *h2 = d2 - 10 + 'A';

    return 0;
}

int htoi(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
	} else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
	} else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
	} else
        return -1;
}
