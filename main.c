#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct {
    char magic[2];
    int fileSize;
    int reserved;
    int dataOffset;
} Header;

typedef struct {
    int size;
    int width;
    int height;
    int xPixels;
    int yPixels;
    int userId;
    int date;
    char accessRights;
} ImageInfo;

struct stat fileStat;

void processFile(char *filePath);

int main(int argc, char *argv[]) {
    if (argc == 2) {
        DIR *dir = opendir(argv[1]);
        if (dir == NULL) {
            perror("Eroare la deschiderea directorului");
            exit(-1);
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            char filePath[PATH_MAX];
            snprintf(filePath, sizeof(filePath), "%s/%s", argv[1], entry->d_name);

            processFile(filePath);
        }

        closedir(dir);
    } else {
        printf("Utilizare: %s <director_intrare>\n", argv[0]);
    }

    return 0;
}

void processFile(char *filePath) {
    char buffer[1000];
    int bytesRead;
    int fileDescriptor;
    int newFileDescriptor;

    fileDescriptor = open(filePath, O_RDWR);

    if (fileDescriptor == -1) {
        perror("Eroare la deschiderea fisierului");
        return;
    }

    Header header;
    ImageInfo imageInfo;

    bytesRead = read(fileDescriptor, &header, sizeof(Header));

    if (bytesRead == -1) {
        perror("Eroare la citirea antetului");
        close(fileDescriptor);
        return;
    }

    bytesRead = read(fileDescriptor, &imageInfo, sizeof(ImageInfo));

    if (bytesRead == -1) {
        perror("Eroare la citirea informatiilor despre imagine");
        close(fileDescriptor);
        return;
    }

    newFileDescriptor = creat("statistica.txt", S_IRUSR | S_IWUSR);

    if (newFileDescriptor == -1) {
        perror("Eroare la crearea fisierului statistica.txt");
        close(fileDescriptor);
        return;
    } else {
        stat(filePath, &fileStat);
        printf("Dimensiune fisier: %d bytes\n", fileStat.st_size);
        printf("\n");

        if (S_ISREG(fileStat.st_mode) && strstr(filePath, ".bmp")) {
            sprintf(buffer, "%d", imageInfo.width);
            write(newFileDescriptor, buffer, strlen(buffer));
            sprintf(buffer, "%d", imageInfo.height);
            write(newFileDescriptor, buffer, strlen(buffer));
        }

        sprintf(buffer, "%d", imageInfo.xPixels);
        write(newFileDescriptor, buffer, strlen(buffer));
        sprintf(buffer, "%d", imageInfo.yPixels);
        write(newFileDescriptor, buffer, strlen(buffer));
        printf("\n");
        write(newFileDescriptor, filePath, strlen(filePath));

        printf("Informatii salvate in statistica.txt\n");
    }

    printf("Numele fisierului: %s\n", filePath);
    if (S_ISREG(fileStat.st_mode) && !strstr(filePath, ".bmp")) {
        printf("Latime: %d\n", imageInfo.width);
        printf("Inaltime: %d\n", imageInfo.height);
    }
    printf("Coordonate X ale pixelilor: %d\n", imageInfo.xPixels);
    printf("Coordonate Y ale pixelilor: %d\n", imageInfo.yPixels);
    printf("Data: %d\n", imageInfo.date);
    printf("Drepturi de acces: %d\n", imageInfo.accessRights);

    close(newFileDescriptor);
    close(fileDescriptor);
}
