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
```yaml
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

### Penjelasan Kode
1. Include library yang diperlukan dan mendefinisikan konstanta
```c
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
```
Pada bagian ini, kode memasukkan library yang diperlukan untuk program FUSE dan fungsi-fungsi sistem lainnya seperti operasi file, direktori, dan lain-lain.
Konstanta ```PART_SIZE``` mendefinisikan ukuran maksimum setiap pecahan file, dalam kasus ini 10240 byte/ 10KB. ```TMP_DIR``` adalah direktori tempat file gabungan disimpan. ```source_path``` adalah jalur direktori sumber relics, dalam kasus ini ```/home/jeky/sisop/praktikum/MODUL_4/soal_3/relics```

2. Mendeklarasi semua function
```c
void add_pecahan(const char *path);
void remove_pecahan();
static int relic_getattr(const char *path, struct stat *stbuf);
static int relic_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
static int relic_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
static int relic_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
static int relic_unlink(const char *path);
static int relic_chmod(const char *path, mode_t mode);
static int relic_create(const char *path, mode_t mode, struct fuse_file_info *fi);
```
Bagian ini mendeklarasikan fungsi-fungsi yang akan digunakan dalam program FUSE. Fungsi-fungsi ini adalah operasi dasar yang harus disediakan untuk sistem file FUSE.

3. Definisi struktur ```TempFile```
```c
typedef struct TempFile {
    char path[1024];
    struct TempFile *next;
} TempFile;

TempFile *temp_files_head = NULL;
```
Struktur ```TempFile``` digunakan untuk menyimpan informasi tentang file gabungan sementara yang dibuat di direktori ```TMP_DIR```. Setiap node dalam struktur ini menyimpan jalur lengkap ke file gabungan sementara dan pointer ke node berikutnya dalam daftar linked list.
```temp_files_head``` adalah pointer ke node pertama dalam daftar linked list ini.

4. Definisi operasi FUSE
```c
static struct fuse_operations relic_oper = {
    .getattr = relic_getattr,
    .readdir = relic_readdir,
    .read = relic_read,
    .write = relic_write,
    .unlink = relic_unlink,
    .chmod = relic_chmod,
    .create = relic_create,
};
```
Bagian ini mendefinisikan struktur operasi FUSE yang berisi pointer ke fungsi-fungsi yang akan digunakan untuk mengimplementasikan operasi sistem file.

5. Fungsi ```main```
```c
int main(int argc, char *argv[]) {
    mkdir(TMP_DIR, 0755);
    int ret = fuse_main(argc, argv, &relic_oper, NULL);
    remove_pecahan();
    return ret;
}
```
Fungsi ```main``` adalah titik masuk program. Pertama, ia membuat direktori ```TMP_DIR``` jika belum ada. Kemudian, ia memanggil fungsi ```fuse_main``` yang akan menjalankan FUSE dengan operasi yang telah didefinisikan sebelumnya. Ketika program selesai, fungsi ```remove_pecahan``` dipanggil untuk menghapus semua file gabungan sementara.

6. Fungsi ```add_pecahan```
```c
void add_pecahan(const char *path) {
    TempFile *new_file = (TempFile *)malloc(sizeof(TempFile));
    if (!new_file) return;
    strcpy(new_file->path, path);
    new_file->next = temp_files_head;
    temp_files_head = new_file;
}
```
Fungsi ini digunakan untuk menambahkan jalur file gabungan sementara ke daftar linked list ```temp_files_head```. Sebuah node baru dialokasikan dengan ```malloc```, jalur file disalin ke dalamnya, dan node baru ditambahkan ke awal daftar linked list.

7. Fungsi ```remove_pecahan```
```c
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
```
Fungsi ini digunakan untuk menghapus semua file gabungan sementara yang disimpan dalam daftar linked list ```temp_files_head```. Fungsi ini berjalan melalui daftar linked list, menghapus setiap file menggunakan ```unlink```, dan membebaskan memori yang dialokasikan untuk setiap node.

8. Fungsi ```relic_getattr```
```c
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
```
Fungsi ini digunakan untuk mendapatkan atribut file atau direktori. Pertama, fungsi mencoba mendapatkan atribut dari direktori sumber ```source_path```. Jika gagal, kemudian mencoba mendapatkan atribut dari direktori sementara ```TMP_DIR```. Jika keduanya gagal, fungsi mengembalikan kode error ```ENOENT``` (tidak ditemukan).

9. Fungsi ```relic_readdir```
```c
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
```
Bagian ini adalah inti dari fungsi ```relic_readdir```. Fungsi ini mengambil setiap file yang namanya diawali dengan "```.000```" dari direktori sumber ```source_path```. Untuk setiap file seperti itu, fungsi ini:

1. Mengekstrak nama dasar file (tanpa ekstensi "```.000```")
2. Membuat jalur file gabungan sementara di direktori ```TMP_DIR```
3. Membuka file gabungan sementara untuk ditulis
4. Mengumpulkan semua pecahan file dengan ekstensi bertingkat (```.000```, ```.001```, dst.) dari direktori sumber
5. Menggabungkan semua pecahan file ke file gabungan sementara
6. Menutup file gabungan sementara
7. Mendapatkan atribut file gabungan sementara
8. Memanggil ```filler``` dengan nama dasar file dan atribut file gabungan
9. Menambahkan jalur file gabungan sementara ke daftar linked list ```temp_files_head```

Jadi, setiap kali fungsi ```readdir``` dipanggil, fungsi ini akan menghasilkan daftar file yang sudah digabungkan dari pecahan-pecahan file di direktori sumber.

```c
}
closedir(dp);
return 0;
```
Terakhir, fungsi menutup direktori sumber dan mengembalikan ```0``` (sukses).

10. Fungsi ```relic_read```
```c
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
```
Fungsi ini digunakan untuk membaca isi file. Fungsi membuka file gabungan sementara di direktori ```TMP_DIR```, membaca isinya menggunakan ```pread```, dan mengembalikan data yang dibaca ke buffer ```buf```. Jika terjadi kesalahan, fungsi mengembalikan kode error yang sesuai.

11. Fungsi ```relic_write```
```c
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
```
Fungsi ini digunakan untuk menulis ke file. Pertama, fungsi membuka atau membuat file gabungan sementara di direktori ```TMP_DIR``` dan menulis data ke file tersebut menggunakan ```pwrite```. Kemudian, fungsi membaca kembali isi file gabungan sementara dan memecahnya menjadi beberapa pecahan file dengan ukuran maksimum ```PART_SIZE```, yang disimpan di direktori sumber ```source_path``` dengan ekstensi bertingkat (```.000```, .```001```, dst.).

12. Fungsi ```relic_unlink```
```c
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
```
Fungsi ini digunakan untuk menghapus file. Fungsi mencoba menghapus semua pecahan file yang terkait dengan nama file yang diberikan di direktori sumber ```source_path```. Fungsi akan menghapus semua pecahan file dengan ekstensi bertingkat (```.000```, ```.001```, dst.) hingga tidak ada lagi pecahan file yang ditemukan (kode error ```ENOENT```).

13. Fungsi ```relic_chmod```
```c
static int relic_chmod(const char *path, mode_t mode) {
    char fpath[1024];
    snprintf(fpath, sizeof(fpath), "%s%s", TMP_DIR, path);
    
    int res = chmod(fpath, mode);
    if (res == -1)
        return -errno;
    
    return 0;
}
```
Fungsi ini digunakan untuk mengubah izin akses file atau direktori. Fungsi mengubah izin akses file gabungan sementara di direktori ```TMP_DIR``` dengan menggunakan fungsi ```chmod```.

14. Fungsi ```relic_create```
```c
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

Fungsi ```relic_create``` digunakan untuk membuat file baru. Fungsi ini membuat file gabungan sementara di direktori ```TMP_DIR``` dengan menggunakan fungsi ```open```. Jika berhasil, file dibuka dengan mode dan flag yang sesuai, kemudian ditutup kembali. Jika terjadi kesalahan saat membuka file, fungsi akan mengembalikan kode error yang sesuai.

Dengan demikian, penjelasan untuk setiap fungsi dalam kode program FUSE tersebut telah selesai.
Secara garis besar, program ini bekerja dengan cara:

1. Membuat direktori ```TMP_DIR``` untuk menyimpan file gabungan sementara.
2. Mendefinisikan operasi FUSE yang diperlukan, seperti ```getattr```, ```readdir```, ```read```, ```write```, ```unlink```, chmod, dan create.
3. Ketika melakukan listing direktori (```readdir```), program akan membuat file gabungan sementara di ```TMP_DIR``` dengan menggabungkan semua pecahan file dari direktori sumber ```source_path```. Informasi file gabungan sementara disimpan dalam daftar linked list ```temp_files_head```.
4. Operasi lain seperti ```read```, ```write```, dan ```create``` akan menggunakan file gabungan sementara di ```TMP_DIR```.
5. Saat menulis file baru (```write```), program akan memecah file menjadi beberapa pecahan dengan ukuran maksimum ```PART_SIZE``` dan menyimpannya di direktori sumber ```source_path``` dengan ekstensi bertingkat (```.000```, ```.001```, dst.).
6. Saat menghapus file (```unlink```), program akan menghapus semua pecahan file yang terkait di direktori sumber ```source_path```.
7. Ketika program selesai (```remove_pecahan```), semua file gabungan sementara di ```TMP_DIR``` akan dihapus.

Dengan menggunakan FUSE, program ini dapat menyajikan direktori sumber ```source_path``` seolah-olah berisi file-file utuh, meskipun sebenarnya file-file tersebut terpecah-pecah di bawahnya.

### Dokumentasi Pengerjaan

1. Mengcompile lalu menjalankan program ```archeology.c``` dan memastikan folder ```bebas/``` telah ter-mount

![image](https://github.com/DzakyAhnaf/Sisop-4-2024-MH-IT12/assets/110287409/28265ef2-be18-43f0-a0ec-5d924d32919a)

2. Melakukan listing, isi dari direktori [nama_bebas] adalah semua relic dari relics yang telah tergabung.

![image](https://github.com/DzakyAhnaf/Sisop-4-2024-MH-IT12/assets/110287409/2e48809f-76d1-49c9-9fe9-aa414c260ca7)

3. Melakukan copy (dari direktori [nama_bebas] ke tujuan manapun), file yang disalin adalah file dari direktori relics yang sudah tergabung.

![image](https://github.com/DzakyAhnaf/Sisop-4-2024-MH-IT12/assets/110287409/87008b7d-2eb5-41ce-98d3-365be69b1089)

4. Membuat suatu file (disini dicontohkan dengan meng-copy file ```ara.jpeg```) di ```./bebas```. Lalu, pada direktori asal (direktori ```relics/```) file tersebut akan dipecah menjadi sejumlah pecahan dengan ukuran maksimum tiap pecahan adalah 10kb.

![Screenshot from 2024-05-24 00-38-26](https://github.com/DzakyAhnaf/Sisop-4-2024-MH-IT12/assets/110287409/2e144df5-dba5-47ad-83ae-32d74999b77a)    
![Screenshot from 2024-05-24 00-38-34](https://github.com/DzakyAhnaf/Sisop-4-2024-MH-IT12/assets/110287409/297f46bf-df4e-49e9-b822-ed24f31a52a1)

5. Melakukan penghapusan file ```ara.jpeg``` di folder ```bebas/```. Semua pecahannya yang ada di ```relics/``` juga ikut terhapus.

![image](https://github.com/DzakyAhnaf/Sisop-4-2024-MH-IT12/assets/110287409/4e538d61-03b1-46d9-b40d-856e1db7c59a)

6. samba

