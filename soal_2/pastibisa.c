#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>

const char *base_dir = "/Users/tarisa/smt-2/sisop/shift4/2/relics";
const char *log_file = "/Users/tarisa/smt-2/sisop/shift4/2/logs-fuse.log";
const char *password = "sisopez";

void decode_base64(const char *input, char *output);
void *decode_rot13(const char *input, char *output);
char *decode_hex(const char *data);
char *reverse(const char *data);
void createLog(const char *status, const char *tag, const char *info);
int checkPass();
static int xmp_getattr(const char *path, struct stat *stbuf);
static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);

static struct fuse_operations xmp_oper = {
   .getattr = xmp_getattr,
   .readdir = xmp_readdir,
   .read = xmp_read,
};

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <mountpoint>\n", argv[0]);
        return 1;
    }
    return fuse_main(argc, argv, &xmp_oper);
}

static int xmp_getattr(const char *path, struct stat *stbuf) {
    char fpath[512];
    snprintf(fpath, sizeof(fpath), "%s%s", base_dir, path);
    int res = lstat(fpath, stbuf);
    if (res == -1) {
        return -errno;
    }
    return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    char fpath[512];
    snprintf(fpath, sizeof(fpath), "%s%s", base_dir, path);

    DIR *dp = opendir(fpath);
    if (dp == NULL) {
        createLog("ERROR", "opendir", strerror(errno));
        return -errno;
    }

    struct dirent *de;
    int res = 0;
    if (strncmp(path, "/rahasia",8) == 0) {
        if (!checkPass()) {
            createLog("FAILED", "access", "Berhasil akses ^^ ");
            closedir(dp);
            return -EACCES; // Access denied 
        }
    }

    while ((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        res = filler(buf, de->d_name, &st, 0);
        if (res != 0) {
            createLog("ERROR", "filler", "Buffer full or error");
            break;
        }
    }

    closedir(dp);
    return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    char fpath[512];
    snprintf(fpath, sizeof(fpath), "%s%s", base_dir, path);

    if (strncmp(path, "/rahasia", 8) == 0) {
        if (!checkPass()) {
            createLog("FAILED", "access", "Gagal akses, password salah");
            return -EACCES; 
        } else {
            createLog("SUCCESS", "access", "Berhasil akses ^^ ");
        }
    }

    int fd = open(fpath, O_RDONLY);
    if (fd == -1) {
        return -errno;
    }

    char decoded[1024];
    int res = pread(fd, decoded, sizeof(decoded) - 1, offset);
    if (res == -1) {
        res = -errno;
        createLog("FAILED", "readFile", strerror(errno));
    } else {
        decoded[res] = '\0';

        if (strstr(path, "/pesan/") != NULL) {
            if (strstr(path, "base64") != NULL) {
                decode_base64(decoded, buf);
                createLog("SUCCESS", "decodeFile", "Base64");
            } else if (strstr(path, "rot13") != NULL) {
                decode_rot13(decoded, buf);
                createLog("SUCCESS", "decodeFile", "ROT13");
            } else if (strstr(path, "hex") != NULL) {
                char *decoded_hex = decode_hex(decoded);
                strncpy(buf, decoded_hex, size);
                free(decoded_hex);
                createLog("SUCCESS", "decodeFile", "Hex");
            } else if (strstr(path, "rev") != NULL) {
                char *decoded_rev = reverse(decoded);
                strncpy(buf, decoded_rev, size);
                free(decoded_rev);
                createLog("SUCCESS", "decodeFile", "Reverse");
            } else {
                strncpy(buf, decoded, size);
                createLog("SUCCESS", "readFile", path);
            }
        } else {
            strncpy(buf, decoded, size);
        }
    }

    close(fd);
    return res;
}

void decode_base64(const char *input, char *output) {
    static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int i, j = 0;
    unsigned char k;
    int len = strlen(input);

    for (i = 0; i < len; i += 4) {
        k = strchr(base64_chars, input[i]) - base64_chars;
        k = (k << 2) | ((strchr(base64_chars, input[i + 1]) - base64_chars) >> 4);
        output[j++] = k;

        if (input[i + 2] != '=') {
            k = ((strchr(base64_chars, input[i + 1]) - base64_chars) << 4) | ((strchr(base64_chars, input[i + 2]) - base64_chars) >> 2);
            output[j++] = k;
        }

        if (input[i + 3] != '=') {
            k = ((strchr(base64_chars, input[i + 2]) - base64_chars) << 6) | (strchr(base64_chars, input[i + 3]) - base64_chars);
            output[j++] = k;
        }
    }

    output[j] = '\0';
}

void *decode_rot13(const char *data, char *decoded) {
    int i;
    int len = strlen(data);
    for (i = 0; i < len; i++) {
        if (isalpha(data[i])) {
            if (islower(data[i])) {
                decoded[i] = ((data[i] - 'a' + 13) % 26) + 'a';
            } else {
                decoded[i] = ((data[i] - 'A' + 13) % 26) + 'A';
            }
        } else {
            decoded[i] = data[i];
        }
    }
    decoded[len] = '\0';
    return decoded;
}

char *decode_hex(const char *data) {
    size_t len = strlen(data) / 2;
    char *decoded = malloc(len + 1);
    for (size_t i = 0; i < len; i++) {
        sscanf(data + 2 * i, "%2hhx", &decoded[i]);
    }
    decoded[len] = '\0';
    return decoded;
}

char *reverse(const char *data) {
    size_t len = strlen(data);
    char *reversed = malloc(len + 1);
    for (size_t i = 0; i < len; i++) {
        reversed[i] = data[len - 1 - i];
    }
    reversed[len] = '\0';
    return reversed;
}

void createLog(const char *status, const char *tag, const char *info) {
    FILE *log_fp = fopen(log_file, "a");
    if (log_fp) {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        fprintf(log_fp, "[%s]::%02d/%02d/%04d-%02d:%02d:%02d::[%s]::[%s]\n",
                status,
                t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
                t->tm_hour, t->tm_min, t->tm_sec,
                tag, info);
        fclose(log_fp);
    }
}
int checkPass() {
    char input[256];
    printf("Enter password: ");
    scanf("%s", input);
    if (strcmp(input, password) == 0) {
        createLog("SUCCESS", "access", "Password betull!!");
        return 1;
    } else {
        createLog("FAILED", "access", "Salah nih passwordnya");
        return 0;
    }
}
