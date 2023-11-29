#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

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

void processFile(char *inputPath, char *outputDir, char c);

void convertToGray(char *filePath);

int main(int argc, char *argv[]) {
    if (argc == 4) {
        DIR *dir = opendir(argv[1]);
        if (dir == NULL) {
            perror("Eroare la deschiderea directorului");
            exit(-1);
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            char inputPath[PATH_MAX];
            snprintf(inputPath, sizeof(inputPath), "%s/%s", argv[1], entry->d_name);

            if (entry->d_type == DT_REG || entry->d_type == DT_LNK) {
                pid_t pid = fork();

                if (pid == -1) {
                    perror("Eroare la crearea procesului fiu");
                    exit(-1);
                } else if (pid == 0) {
                    // Proces fiu
                    processFile(inputPath, argv[2], argv[3][0]);
                    exit(0);
                }
            }
        }

        int totalCorrectSentences = 0;
        int status;
        pid_t wpid;

        while ((wpid = wait(&status)) > 0) {
            if (WIFEXITED(status)) {
                printf("S-a incheiat procesul cu pid-ul %d si codul %d\n", wpid, WEXITSTATUS(status));
                totalCorrectSentences += WEXITSTATUS(status);
            }
        }

        printf("Au fost identificate in total %d propozitii corecte care contin caracterul %c\n", totalCorrectSentences, argv[3][0]);

        closedir(dir);
    } else {
        printf("Utilizare: %s <director_intrare> <director_iesire> <c>\n", argv[0]);
    }

    return 0;
}

void processFile(char *inputPath, char *outputDir) {
    char buffer[1000];
    int bytesRead;
    int fileDescriptor;
    int newFileDescriptor;

    fileDescriptor = open(inputPath, O_RDWR);

    if (fileDescriptor == -1) {
        perror("Eroare la deschiderea fisierului");
        exit(-1);
    }

    Header header;
    ImageInfo imageInfo;

    bytesRead = read(fileDescriptor, &header, sizeof(Header));

    if (bytesRead == -1) {
        perror("Eroare la citirea antetului");
        close(fileDescriptor);
        exit(-1);
    }

    bytesRead = read(fileDescriptor, &imageInfo, sizeof(ImageInfo));

    if (bytesRead == -1) {
        perror("Eroare la citirea informatiilor despre imagine");
        close(fileDescriptor);
        exit(-1);
    }

    newFileDescriptor = creat("statistica.txt", S_IRUSR | S_IWUSR);

    if (newFileDescriptor == -1) {
        perror("Eroare la crearea fisierului statistica.txt");
        close(fileDescriptor);
        exit(-1);
    } else {
        stat(inputPath, &fileStat);
        printf("Dimensiune fisier: %d bytes\n", fileStat.st_size);
        printf("\n");

        if (S_ISREG(fileStat.st_mode) && strstr(inputPath, ".bmp")) {
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
        write(newFileDescriptor, inputPath, strlen(inputPath));

        printf("Informatii salvate in statistica.txt\n");
    }

    printf("Numele fisierului: %s\n", inputPath);
    if (S_ISREG(fileStat.st_mode) && !strstr(inputPath, ".bmp")) {
        printf("Latime: %d\n", imageInfo.width);
        printf("Inaltime: %d\n", imageInfo.height);
    }
    printf("Coordonate X ale pixelilor: %d\n", imageInfo.xPixels);
    printf("Coordonate Y ale pixelilor: %d\n", imageInfo.yPixels);
    printf("Data: %d\n", imageInfo.date);
    printf("Drepturi de acces: %d\n", imageInfo.accessRights);

    close(newFileDescriptor);

    if (S_ISREG(fileStat.st_mode) && strstr(inputPath, ".bmp")) {
        pid_t grayPid = fork();

        if (grayPid == -1) {
            perror("Eroare la crearea procesului fiu pentru conversia la tonuri de gri");
            exit(-1);
        } else if (grayPid == 0) {
            // Proces fiu pentru conversia la tonuri de gri
            convertToGray(inputPath);
            exit(0);
        } else {
            // Proces parinte
            int status;
            waitpid(grayPid, &status, 0);
            printf("S-a incheiat procesul pentru conversia la tonuri de gri cu pid-ul %d si codul %d\n", grayPid, WEXITSTATUS(status));
        }
    }
        if (S_ISREG(fileStat.st_mode) && !strstr(inputPath, ".bmp")) {
        printf("Latime: %d\n", imageInfo.width);
        printf("Inaltime: %d\n", imageInfo.height);

        if (imageInfo.width > 0 && imageInfo.height > 0) {
            int pipefd[2];
            if (pipe(pipefd) == -1) {
                perror("Eroare la crearea pipe-ului");
                exit(-1);
            }

            pid_t textProcessPid = fork();

            if (textProcessPid == -1) {
                perror("Eroare la crearea procesului fiu pentru generarea continutului text");
                close(pipefd[0]);
                close(pipefd[1]);
                exit(-1);
            } else if (textProcessPid == 0) {
                // Proces fiu pentru generarea continutului text
                close(pipefd[0]); // Inchidem partea de citire a pipe-ului

                // Redirectionam iesirea standard catre pipe
                dup2(pipefd[1], STDOUT_FILENO);

                // Inchidem partea de scriere a pipe-ului
                close(pipefd[1]);

                // Apelam script-ul shell cu argumentul c
                execlp("/bin/bash", "bash", "script", &c, NULL);

                perror("Eroare la apelul script-ului shell");
                exit(-1);
            } else {
                // Proces parinte
                close(pipefd[1]); // Inchidem partea de scriere a pipe-ului

                // Citim continutul generat de procesul fiu
                FILE *pipeStream = fdopen(pipefd[0], "r");
                char buffer[100];
                int sentencesCount;

                if (pipeStream == NULL) {
                    perror("Eroare la deschiderea fluxului de date din pipe");
                    close(pipefd[0]);
                    exit(-1);
                }

                fscanf(pipeStream, "%d", &sentencesCount);
                fclose(pipeStream);

                // Afișăm mesajul specific
                printf("Procesul fiu a generat %d propozitii corecte\n", sentencesCount);

                // Trimitem numarul de propozitii corecte prin pipe la procesul parinte
                write(pipefd[1], &sentencesCount, sizeof(sentencesCount));

                // Inchidem partea de citire a pipe-ului
                close(pipefd[0]);

                // Asteptam ca procesul fiu sa se incheie
                int status;
                waitpid(textProcessPid, &status, 0);
                printf("S-a incheiat procesul pentru generarea continutului text cu pid-ul %d si codul %d\n", textProcessPid, WEXITSTATUS(status));
            }
        }
    }

    close(fileDescriptor);
}

void convertToGray(char *filePath) {
    int fileDescriptor = open(filePath, O_RDWR);

    if (fileDescriptor == -1) {
        perror("Eroare la deschiderea fisierului pentru conversie");
        exit(-1);
    }

    Header header;
    ImageInfo imageInfo;

    // Citim antetul și informațiile despre imagine
    int bytesRead = read(fileDescriptor, &header, sizeof(Header));

    if (bytesRead == -1) {
        perror("Eroare la citirea antetului pentru conversie");
        close(fileDescriptor);
        exit(-1);
    }

    bytesRead = read(fileDescriptor, &imageInfo, sizeof(ImageInfo));

    if (bytesRead == -1) {
        perror("Eroare la citirea informatiilor despre imagine pentru conversie");
        close(fileDescriptor);
        exit(-1);
    }

    // Verificăm dacă imaginea este de tip BMP
    if (strstr(filePath, ".bmp")) {
        // Calculăm dimensiunea totală a imaginii (numărul total de pixeli)
        int imageSize = imageInfo.width * imageInfo.height;

        // Ne deplasăm la începutul zonei raster (dataOffset specificat în antet)
        lseek(fileDescriptor, header.dataOffset, SEEK_SET);

        // Iterăm prin fiecare pixel și aplicăm formula de conversie la tonuri de gri
        for (int i = 0; i < imageSize; ++i) {
            unsigned char pixel[3];  // Fiecare pixel are 3 componente: R, G, B
            read(fileDescriptor, pixel, sizeof(pixel));

            // Formula de conversie la tonuri de gri
            unsigned char grayValue = (unsigned char)(0.299 * pixel[0] + 0.587 * pixel[1] + 0.114 * pixel[2]);

            // Suprascriem valorile pixelilor cu valoarea de gri calculată
            lseek(fileDescriptor, -sizeof(pixel), SEEK_CUR);
            write(fileDescriptor, &grayValue, sizeof(grayValue));
        }

        printf("Conversie la tonuri de gri pentru %s realizata cu succes\n", filePath);
    }

    close(fileDescriptor);
}
