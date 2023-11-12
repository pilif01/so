#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

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

int main(int argc, char *argv[]) {
    char buffer[1000];
    int fileDescriptor;
    int bytesRead;
    int newFileDescriptor;

    if (argc == 2) {
        fileDescriptor = open(argv[1], O_RDWR);

        if (fileDescriptor == -1) {
            perror("Eroare la deschiderea fisierului");
            exit(-1);
        } else {
            Header header;
            ImageInfo imageInfo;

            bytesRead = read(fileDescriptor, &header, sizeof(Header));

            if (bytesRead == -1) {
                perror("Eroare la citirea antetului");
                exit(-1);
            }

            bytesRead = read(fileDescriptor, &imageInfo, sizeof(ImageInfo));

            if (bytesRead == -1) {
                perror("Eroare la citirea informatiilor despre imagine");
                exit(-1);
            }

            newFileDescriptor = creat("statistica.txt", S_IRUSR | S_IWUSR);

            if (newFileDescriptor == -1) {
                perror("Eroare la crearea fisierului statistica.txt");
                exit(-1);
            } else {
                stat(argv[1], &fileStat);
                printf("Dimensiune fisier: %d bytes\n", fileStat.st_size);
                printf("\n");

                sprintf(buffer, "%d", imageInfo.width);
                write(newFileDescriptor, buffer, strlen(buffer));
                sprintf(buffer, "%d", imageInfo.height);
                write(newFileDescriptor, buffer, strlen(buffer));
                sprintf(buffer, "%d", imageInfo.xPixels);
                write(newFileDescriptor, buffer, strlen(buffer));
                sprintf(buffer, "%d", imageInfo.yPixels);
                write(newFileDescriptor, buffer, strlen(buffer));
                printf("\n");
                write(newFileDescriptor, argv[1], strlen(argv[1]));

                printf("Informatii salvate in statistica.txt\n");
            }

            printf("Numele fisierului: %s\n", argv[1]);
            printf("Latime: %d\n", imageInfo.width);
            printf("Inaltime: %d\n", imageInfo.height);
            printf("Coordonate X ale pixelilor: %d\n", imageInfo.xPixels);
            printf("Coordonate Y ale pixelilor: %d\n", imageInfo.yPixels);
            printf("Data: %d\n", imageInfo.date);
            printf("Drepturi de acces: %d\n", imageInfo.accessRights);
        }
    } else {
        printf("Utilizare: %s imagine.bmp\n", argv[0]);
    }

    return 0;
}

