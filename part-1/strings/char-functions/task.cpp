bool isalpha(unsigned char c) {
    return ((c >= 'A' && c <= 'Z') || (c>='a' && c<='z'));
}

unsigned char tolower(unsigned char c) {
    if (c >= 'A' && c <= 'Z'){
        return (c+32);
    } else {
        return c;
    }
}
