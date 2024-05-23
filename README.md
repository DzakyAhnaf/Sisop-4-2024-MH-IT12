# Laporan Resmi Praktikum Sistem Operasi 2024 Modul-4
## Anggota Kelompok IT 12 :

- Muhammad Dzaky Ahnaf (5027231039)
- Adlya Isriena Aftarisya (5027231066)
- Nisrina Atiqah Dwiputri Ridzki (5027231075)

## Daftar Isi

- [Soal 1](#soal-1)
- [Soal 2](#soal-2)
- [Soal 3](#soal-3)

## Soal 1

Dikerjakan oleh Nisrina Atiqah Dwiputri Ridzki (5027231075)

## Soal 2

Dikerjakan oleh Adlya Isriena Aftarisya (5027231066)

## Soal 3

Dikerjakan oleh Muhammad Dzaky Ahnaf (5027231039)

### Deskripsi Soal
Pada soal ini, kita diharapkan membuat sebuah file system meggunakan FUSE untuk menggabungkan relik-relik yang berupa file png yang terpecah, namun karena setiap pecahan relik itu masih memiliki nilai tersendiri, kita akan membuat sebuah file system yang mana saat kita mengakses file system tersebut kita dapat melihat semua relik dalam keadaan utuh, sementara relik yang asli tidak berubah sama sekali.

Ketentuan:
- Buatlah sebuah direktori dengan ketentuan seperti pada tree berikut
```
.
├── [nama_bebas]
├── relics
│   ├── relic_1.png.000
│   ├── relic_1.png.001
│   ├── dst dst…
│   └── relic_9.png.010
└── report
```
- Direktori [nama_bebas] adalah direktori FUSE dengan direktori asalnya adalah direktori relics. Ketentuan Direktori [nama_bebas] adalah sebagai berikut :
Ketika dilakukan listing, isi dari direktori [nama_bebas] adalah semua relic dari relics yang telah tergabung.

    - Ketika dilakukan listing, isi dari direktori [nama_bebas] adalah semua relic dari relics yang telah tergabung.

![image](https://github.com/DzakyAhnaf/Sisop-4-2024-MH-IT12/assets/110287409/b21323b6-5401-48ef-ab4f-7ba299048d43)

  - Ketika dilakukan copy (dari direktori [nama_bebas] ke tujuan manapun), file yang disalin adalah file dari direktori relics yang sudah tergabung.

![image](https://github.com/DzakyAhnaf/Sisop-4-2024-MH-IT12/assets/110287409/430d5b83-0d1e-48e8-bd15-c6d15f5b8e56)

  - Ketika ada file dibuat, maka pada direktori asal (direktori relics) file tersebut akan dipecah menjadi sejumlah pecahan dengan ukuran maksimum tiap pecahan adalah 10kb.

![image](https://github.com/DzakyAhnaf/Sisop-4-2024-MH-IT12/assets/110287409/561decad-03e8-425e-ac9b-8818d0331501)

  - File yang dipecah akan memiliki nama [namafile].000 dan seterusnya sesuai dengan jumlah pecahannya.Ketika dilakukan penghapusan, maka semua pecahannya juga ikut terhapus.

- Direktori report adalah direktori yang akan dibagikan menggunakan Samba File Server. Setelah kalian berhasil membuat direktori [nama_bebas], jalankan FUSE dan salin semua isi direktori [nama_bebas] pada direktori report.

![image](https://github.com/DzakyAhnaf/Sisop-4-2024-MH-IT12/assets/110287409/0dac2dbc-f006-4d48-b292-235b124d1bc4)

- Catatan:
    - pada contoh terdapat 20 relic, namun pada zip file hanya akan ada 10 relic
    - [nama_bebas] berarti namanya dibebaskan
    - pada soal 3c, cukup salin secara manual. File Server hanya untuk membuktikan bahwa semua file pada direktori [nama_bebas] dapat dibuka dengan baik.
    - sumber folder relics : https://drive.google.com/file/d/1BJkaBvGaxqiwPWvXRdYNXzxxmIYQ8FKf/view?usp=sharing


 ### Kode Penyelesaian
```archeology.c
#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libgen.h>

#define PART_SIZE 10240
#define TMP_DIR "/tmp/combined"

const char *source_path = "/home/jeky/sisop/praktikum/MODUL_4/soal_3/relics"; 

void add_pecahan(const char *path);
void remove_pecahan();
static int relic_getattr(const char *path, struct stat *stbuf);
static int relic_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
static int relic_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
static int relic_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
static int relic_unlink(const char *path);
static int relic_chmod(const char *path, mode_t mode);
static int relic_create(const char *path, mode_t mode, struct fuse_file_info *fi);

typedef struct TempFile {
    char path[1024];
    struct TempFile *next;
} TempFile;

TempFile *temp_files_head = NULL;

static struct fuse_operations relic_oper = {
    .getattr = relic_getattr,
    .readdir = relic_readdir,
    .read = relic_read,
    .write = relic_write,
    .unlink = relic_unlink,
    .chmod = relic_chmod,
    .create = relic_create,
};

int main(int argc, char *argv[]) {
    mkdir(TMP_DIR, 0755);
    int ret = fuse_main(argc, argv, &relic_oper, NULL);
    remove_pecahan();
    return ret;
}


void add_pecahan(const char *path) {
    TempFile *new_file = (TempFile *)malloc(sizeof(TempFile));
    if (!new_file) return;
    strcpy(new_file->path, path);
    new_file->next = temp_files_head;
    temp_files_head = new_file;
}

void remove_pecahan() {
    TempFile *current = temp_files_head;
    while (current) {
        unlink(current->path);
        TempFile *next = current->next;
        free(current);
        current = next;
    }
    temp_files_head = NULL;
}

// get attribute file
static int relic_getattr(const char *path, struct stat *stbuf) {
    memset(stbuf, 0, sizeof(struct stat));
    char fpath[1024];

    snprintf(fpath, sizeof(fpath), "%s%s", source_path, path);
    if (stat(fpath, stbuf) == -1) {
        snprintf(fpath, sizeof(fpath), "%s%s", TMP_DIR, path);
        if (stat(fpath, stbuf) == -1) {
            return -ENOENT;
        }
    }
    return 0;
}

// baca direktori buat ls
static int relic_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    (void)offset;
    (void)fi;
    
    if (strcmp(path, "/") != 0)
        return -ENOENT;

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    DIR *dp = opendir(source_path);
    if (!dp)
        return -errno;

    struct dirent *de;
    while ((de = readdir(dp)) != NULL) {
        if (de->d_type == DT_DIR)
            continue;

        char *ext = strrchr(de->d_name, '.');
        if (ext && strcmp(ext, ".000") == 0) {
            char base_name[256];
            strncpy(base_name, de->d_name, ext - de->d_name);
            base_name[ext - de->d_name] = '\0';

            char combined_path[1024];
            snprintf(combined_path, sizeof(combined_path), "%s%s/%s", TMP_DIR, path, base_name);

            mkdir(TMP_DIR, 0755);

            FILE *combined_file = fopen(combined_path, "wb");
            if (!combined_file) {
                closedir(dp);
                return -errno;
            }

            int part_idx = 0;
            while (1) {
                char part_path[1024];
                snprintf(part_path, sizeof(part_path), "%s/%s.%03d", source_path, base_name, part_idx);

                FILE *part_file = fopen(part_path, "rb");
                if (!part_file)
                    break;

                char buffer[PART_SIZE];
                size_t bytes_read;
                while ((bytes_read = fread(buffer, 1, PART_SIZE, part_file)) > 0) {
                    fwrite(buffer, 1, bytes_read, combined_file);
                }
                fclose(part_file);
                part_idx++;
            }
            fclose(combined_file);

            struct stat st;
            stat(combined_path, &st);
            filler(buf, base_name, &st, 0);

            add_pecahan(combined_path);
        }
    }
    closedir(dp);
    return 0;
}

// buka dan baca file (cat)
static int relic_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    (void)offset;
    
    char fpath[1024];
    snprintf(fpath, sizeof(fpath), "%s%s", TMP_DIR, path);

    int fd = open(fpath, O_RDONLY);
    if (fd == -1)
        return -errno;

    int res = pread(fd, buf, size, offset);
    if (res == -1)
        res = -errno;

    close(fd);
    return res;
}

// tulis isi file (cp, nano)
static int relic_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    char combined_path[1024];
    snprintf(combined_path, sizeof(combined_path), "%s%s", TMP_DIR, path);

    int fd = open(combined_path, O_WRONLY | O_CREAT, 0644);
    if (fd == -1)
        return -errno;

    int res = pwrite(fd, buf, size, offset);
    if (res == -1) {
        close(fd);
        return -errno;
    }
    close(fd);

    FILE *combined_file = fopen(combined_path, "rb");
    if (!combined_file)
        return -errno;

    int part_idx = 0;
    char part_path[1024];
    size_t bytes_read;
    char buffer[PART_SIZE];

    while ((bytes_read = fread(buffer, 1, PART_SIZE, combined_file)) > 0) {
        snprintf(part_path, sizeof(part_path), "%s%s.%03d", source_path, path, part_idx);
        FILE *part_file = fopen(part_path, "wb");
        if (!part_file) {
            fclose(combined_file);
            return -errno;
        }

        fwrite(buffer, 1, bytes_read, part_file);
        fclose(part_file);
        part_idx++;
    }

    fclose(combined_file);
    return res;
}

static int relic_unlink(const char *path) {
    char fpath[1024];
    int res = 0;

    for (int i = 0;; i++) {
        snprintf(fpath, sizeof(fpath), "%s%s.%03d", source_path, path, i);
        res = unlink(fpath);
        if (res == -1 && errno == ENOENT)
            break;
        else if (res == -1)
            return -errno;
    }

    return 0;
}

// buat ubah permission
static int relic_chmod(const char *path, mode_t mode) {
    char fpath[1024];
    snprintf(fpath, sizeof(fpath), "%s%s", TMP_DIR, path);
    
    int res = chmod(fpath, mode);
    if (res == -1)
        return -errno;
    
    return 0;
}

// membuat file atau folder baru (touch, mkdir)
static int relic_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    char combined_path[1024];
    snprintf(combined_path, sizeof(combined_path), "%s%s", TMP_DIR, path);

    int fd = open(combined_path, fi->flags, mode);
    if (fd == -1)
        return -errno;

    close(fd);
    return 0;
}
```
