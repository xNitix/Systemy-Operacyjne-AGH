#include <stdio.h>

// fseek ustawia wskaźnik pozycji w pliku
// fseek(int filedes, off_t offset, int whence)
// filedes - deskryptor pliku
// offset - przesuniecie w bajtach
// whence - skad ma liczyc przesuniecie
// jesli wartoscia whence jest SEEK_SET, to nowy wskaźnik pozycji w pliku jest oddalony o offset bajtów od początku pliku
// jesli wartoscia whence jest SEEK_CUR, to nowy wskaźnik pozycji w pliku jest oddalony o offset bajtów od bieżącej pozycji
// jesli wartoscia whence jest SEEK_END, to nowy wskaźnik pozycji w pliku jest oddalony o offset bajtów od rozmiaru pliku

// ftell zwraca aktualna pozycje w pliku

//fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
//ptr - wskaźnik do bufora, w którym zostaną zapisane dane
//size - rozmiar w bajtach każdego elementu do przeczytania
//nmemb - liczba elementów do przeczytania
//stream - wskaźnik do strumienia pliku

//fwrite(void *ptr, size_t size, size_t nmemb, FILE *stream)
//ptr - wskaźnik do bufora, z którego zostaną zapisane dane
//size - rozmiar w bajtach każdego elementu do zapisania
//nmemb - liczba elementów do zapisania
//stream - wskaźnik do strumienia pliku

// fopen(const char *path, const char *mode)
// path - sciezka do pliku
// mode - tryb otwarcia pliku
// r - czytanie
// w - zapis
// a - dopisywanie
// r+ - czytanie i zapis
// w+ - czytanie i zapis, plik jest tworzony jesli nie istnieje, a jesli istnieje to jest czyszczony
// a+ - czytanie i zapis, plik jest tworzony jesli nie istnieje, a jesli istnieje to jest dopisywany
// b - tryb binarny


void reverse_copy(FILE *giver, FILE *receiver) {
    fseek(giver, 0, SEEK_END); 
    // ustawilem fseek na koniec pliku, wiec fteel zwraca mi dlugosc pliku
    long totalBytesCount = ftell(giver);

    // buffer na jeden bajt
    char byte;
    for(long i = totalBytesCount-1; i >= 0; i--){
        fseek(giver, i, SEEK_SET); 
        fread(&byte, sizeof(char), 1, giver);
        fwrite(&byte, sizeof(char), 1, receiver); 
    }
}


void reverse_copy_1024buffer(FILE *giver, FILE *receiver) {
    char buffer[1024];
    long totalBytesCount;

    fseek(giver, 0, SEEK_END);
    totalBytesCount = ftell(giver);
    long bytesLeft = totalBytesCount;

    long blockSize;
    while (bytesLeft >= 0) {

        if (bytesLeft >= 1024) {
            blockSize = 1024;
        } else {
            blockSize = bytesLeft;
        }

        fseek(giver, -blockSize, SEEK_CUR);
        fread(buffer, 1, blockSize, giver);
        // powrot wskaznika po przeczytaniu bloku
        fseek(giver, -blockSize, SEEK_CUR);

        for (long i = 0; i < (blockSize / 2); i++) {
            char temp = buffer[i];
            buffer[i] = buffer[blockSize - i - 1];
            buffer[blockSize - i - 1] = temp;
        }

        fwrite(buffer, 1, blockSize, receiver);

        bytesLeft -= blockSize;
    }
}

int main(int argc, const char *argv[]) {

    FILE *giver = fopen(argv[1], "rb");

    if(!giver) {
        printf("File %s\n opening error", argv[1]);
        return 1;
    }

    FILE *receiver = fopen(argv[2], "wb");

    if(!receiver) {
        printf("File %s\n opening error", argv[2]);
        return 1;
    }

    FILE *receiver2 = fopen(argv[3], "wb");

    if(!receiver) {
        printf("File %s\n opening error", argv[2]);
        return 1;
    }

    reverse_copy(giver, receiver);
    reverse_copy_1024buffer(giver, receiver2);

    fclose(giver);
    fclose(receiver);
    fclose(receiver2);

    return 0;
}