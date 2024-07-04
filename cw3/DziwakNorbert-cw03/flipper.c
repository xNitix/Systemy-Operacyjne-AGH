# include <stdio.h>
# include <dirent.h>
# include <string.h>

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


int main (int argc, const char *argv[]) {
    DIR *giverDir;

    struct dirent *input;

    giverDir = opendir(argv[1]);

    if (giverDir== NULL) {
        printf("Error opening directory\n");
        return 1;
    }

    while ((input = readdir(giverDir)) != NULL) {
        if (strcmp(input->d_name + strlen(input->d_name) - 4, ".txt") == 0) {
            char giverFilePath[1024];
            char receiverFilePath[1024];

            sprintf(giverFilePath, "%s/%s", argv[1], input->d_name);
            sprintf(receiverFilePath, "%s/%s", argv[2], input->d_name);

            FILE *giverFile = fopen(giverFilePath, "r");
            FILE *receiverFile = fopen(receiverFilePath, "w"); // jak nie ma pliku to go tworzy

            if(!giverFile) {
                printf("File %s\n opening error", giverFilePath);
                return 1;
            }

            if(!receiverFile) {
                printf("File %s\n opening error", receiverFilePath);
                return 1;
            }

            reverse_copy(giverFile, receiverFile);

            fclose(giverFile);
            fclose(receiverFile);
        }
    }

    closedir(giverDir);

    return 0;
}