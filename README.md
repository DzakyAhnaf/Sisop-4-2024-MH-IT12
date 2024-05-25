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

### Deskripsi Soal
Adfi merupakan seorang CEO agency creative bernama Ini Karya Kita. Ia sedang melakukan inovasi pada manajemen project photography Ini Karya Kita. Salah satu ide yang dia kembangkan adalah tentang pengelolaan foto project dalam sistem arsip Ini Karya Kita. Dalam membangun sistem ini, Adfi tidak bisa melakukannya sendirian, dia perlu bantuan mahasiswa Departemen Teknologi Informasi angkatan 2023 untuk membahas konsep baru yang akan mengubah project fotografinya lebih menarik untuk dilihat. Adfi telah menyiapkan portofolio hasil project fotonya yang bisa didownload dan diakses di www.inikaryakita.id . Silahkan eksplorasi web Ini Karya Kita dan temukan halaman untuk bisa mendownload projectnya. Setelah kalian download terdapat folder gallery dan bahaya.
Pada folder “gallery”:
Membuat folder dengan prefix "wm." Dalam folder ini, setiap gambar yang dipindahkan ke dalamnya akan diberikan watermark bertuliskan inikaryakita.id. 
			Ex: "mv ikk.jpeg wm-foto/" 
   
Output: 

Before: (tidak ada watermark bertuliskan inikaryakita.id)

After: (terdapat watermark tulisan inikaryakita.id)


Pada folder "bahaya," terdapat file bernama "script.sh." Adfi menyadari pentingnya menjaga keamanan dan integritas data dalam folder ini. 
Mereka harus mengubah permission pada file "script.sh" agar bisa dijalankan, karena jika dijalankan maka dapat menghapus semua dan isi dari  "gallery"
Adfi dan timnya juga ingin menambahkan fitur baru dengan membuat file dengan prefix "test" yang ketika disimpan akan mengalami pembalikan (reverse) isi dari file tersebut.  



### Kode Penyelesaian

```c
#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

static int xmp_getattr(const char *path, struct stat *stbuf) {
    int res;

    res = lstat(path, stbuf);
    if (res == -1)
        return -errno;

    return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info *fi) {
    DIR *dp;
    struct dirent *de;

    (void) offset;
    (void) fi;

    dp = opendir(path);
    if (dp == NULL)
        return -errno;

    while ((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        if (filler(buf, de->d_name, &st, 0))
            break;
    }

    closedir(dp);
    return 0;
}

// Function to get image dimensions using ImageMagick identify
void get_image_dimensions(const char *image_path, int *width, int *height) {
    char command[512];
    snprintf(command, sizeof(command), "identify -format \"%%w %%h\" \"%s\"", image_path);

    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        perror("Failed to run identify command");
        exit(1);
    }

    fscanf(fp, "%d %d", width, height);
    pclose(fp);
}

// Function to add watermark using ImageMagick convert
void add_watermark(const char *image_path) {
    int width, height;
    get_image_dimensions(image_path, &width, &height);

    int pointsize = (width < height ? width : height) / 20; // Watermark text size based on image dimensions

    char command[1024];
    snprintf(command, sizeof(command), 
             "convert \"%s\" -gravity south -fill white -pointsize %d -annotate +0+10 'inikaryakita.id' \"%s\"", 
             image_path, pointsize, image_path);
    system(command);
}

// Function to handle adding watermark in the gallery folder
static void process_gallery() {
    DIR *dir;
    struct dirent *ent;
    char src_path[512], dest_path[512];

    // Create wm folder if it doesn't exist
    mkdir("gallery/wm", 0777);

    if ((dir = opendir("gallery")) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            // Check file extension
            if (strstr(ent->d_name, ".jpeg") || strstr(ent->d_name, ".jpg") || strstr(ent->d_name, ".png") ||
                strstr(ent->d_name, ".JPEG") || strstr(ent->d_name, ".JPG") || strstr(ent->d_name, ".PNG")) {
                
                snprintf(src_path, sizeof(src_path), "gallery/%s", ent->d_name);
                snprintf(dest_path, sizeof(dest_path), "gallery/wm/%s", ent->d_name);
                
                // Move file to wm folder
                if (rename(src_path, dest_path) == 0) {
                    // Add watermark after moving
                    add_watermark(dest_path);
                    printf("Watermark added to %s.\n", dest_path);
                } else {
                    perror("Error moving file");
                }
            }
        }
        closedir(dir);
    } else {
        perror("Could not open gallery directory");
    }
}

// Function to change permissions of script.sh
static void change_permission() {
    FILE *script_file = fopen("bahaya/script.sh", "r");
    if (script_file) {
        char line[256];
        while (fgets(line, sizeof(line), script_file)) {
            if (strstr(line, "rm -rf ../gallery")) {
                system("chmod +x bahaya/script.sh");
                break;
            }
        }
        fclose(script_file);
    } else {
        perror("Error opening script file");
    }
}

// Function to reverse the content of a file
static void reverse_file(const char *input_path, const char *output_path) {
    FILE *input = fopen(input_path, "r");
    FILE *output = fopen(output_path, "w");
    if (input && output) {
        fseek(input, 0, SEEK_END);
        long size = ftell(input);
        fseek(input, 0, SEEK_SET);
        char *content = (char *)malloc(size + 1);
        fread(content, 1, size, input);
        for (long i = size - 1; i >= 0; i--) {
            fputc(content[i], output);
        }
        free(content);
        fclose(input);
        fclose(output);
        printf("File content of %s has been reversed and saved as %s.\n", input_path, output_path);
    } else {
        perror("Error opening file");
    }
}

// Function to handle reversing files with the prefix "test" in bahaya folder
static void process_bahaya() {
    DIR *dir;
    struct dirent *ent;
    char src_path[512], dest_path[512];

    if ((dir = opendir("bahaya")) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (strncmp(ent->d_name, "test", 4) == 0) {
                snprintf(src_path, sizeof(src_path), "bahaya/%s", ent->d_name);
                snprintf(dest_path, sizeof(dest_path), "bahaya/reversed_%s", ent->d_name);
                reverse_file(src_path, dest_path);
            }
        }
        closedir(dir);
    } else {
        perror("Could not open bahaya directory");
    }
}

// Function to run script.sh
static void run_script() {
    system("./bahaya/script.sh");
}

// Function to delete directories
static void delete_directories() {
    char response[10];
    printf("Are you sure you want to delete the 'gallery' and 'bahaya' folders? (yes/no): ");
    scanf("%s", response);
    if (strcmp(response, "yes") == 0) {
        if (system("rm -rf gallery bahaya") != 0) {
            printf("Failed to delete folders\n");
        } else {
            printf("Folders deleted\n");
        }
    } else {
        printf("Deletion canceled\n");
    }
}

int main(int argc, char *argv[]) {
    process_gallery();

    change_permission();

    process_bahaya();

    run_script();

    delete_directories();

    struct fuse_operations xmp_oper = {
        .getattr = xmp_getattr,
        .readdir = xmp_readdir,
    };

    return fuse_main(argc, argv, &xmp_oper, NULL);
}

```
---

### Penjelesan

---

1. Fungsi ```xmp_getattr```:
	  ```c
	static int xmp_getattr(const char *path, struct stat *stbuf) {
    int res;

    res = lstat(path, stbuf);
    if (res == -1)
        return -errno;

    return 0;
	}

	
	```
	Fungsi ```xmp_getattr``` digunakan untuk mendapatkan atribut dari sebuah berkas atau direktori yang ditentukan oleh path. Memanggil fungsi lstat untuk mengisi struktur stat (stbuf) dengan informasi tentang berkas atau direktori yang ditentukan path. Jika lstat gagal, fungsi mengembalikan nilai negatif dari errno.

2. Fungsi ```xmp_readdir```:
	  ```c
	static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    DIR *dp;
    struct dirent *de;

    (void) offset;
    (void) fi;

    dp = opendir(path);
    if (dp == NULL)
        return -errno;

    while ((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        if (filler(buf, de->d_name, &st, 0))
            break;
    }

    closedir(dp);
    return 0;
	}

	```
	Fungsi ```xmp_readdir``` digunakan untuk membaca konten dari direktori yang ditentukan oleh path. Dengan membuka dan membaca entri dalam direktori satu per satu lalu mengisi buffer dengan nama entri direktori dan atributnya.

3. Fungsi ```get_image_dimensions```:
	```c
	void get_image_dimensions(const char *image_path, int *width, int *height) {
    char command[512];
    snprintf(command, sizeof(command), "identify -format \"%%w %%h\" \"%s\"", image_path);

    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        perror("Failed to run identify command");
        exit(1);
    }

    fscanf(fp, "%d %d", width, height);
    pclose(fp);
	}

	```
	
	Fungsi ```get_image_dimensions``` digunakan untuk mendapatkan dimensi (lebar dan tinggi) dari gambar yang ditentukan oleh image_path. Karena dimensi setiap foto yang ada berbeda beda.

4. Fungsi ```add_watermark```:
   ```c
    void add_watermark(const char *image_path) {
    int width, height;
    get_image_dimensions(image_path, &width, &height);

    int pointsize = (width < height ? width : height) / 20; // Watermark text size based on image dimensions

    char command[1024];
    snprintf(command, sizeof(command), 
             "convert \"%s\" -gravity south -fill white -pointsize %d -annotate +0+10 'inikaryakita.id' \"%s\"", 
             image_path, pointsize, image_path);
    system(command);
	}

	```

    Fungsi ```add_watermark``` digunakan untuk menambahkan watermark pada gambar yang ditentukan oleh image_path dengan format bentuk dan posisi meletakkan watermarknya setelah mendapatkan dimensi gambarnya.
    
5. Fungsi ```process_gallery```:
   ```c
   	static void process_gallery() {
    DIR *dir;
    struct dirent *ent;
    char src_path[512], dest_path[512];

    // Create wm folder if it doesn't exist
    mkdir("gallery/wm", 0777);

    if ((dir = opendir("gallery")) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            // Check file extension
            if (strstr(ent->d_name, ".jpeg") || strstr(ent->d_name, ".jpg") || strstr(ent->d_name, ".png") ||
                strstr(ent->d_name, ".JPEG") || strstr(ent->d_name, ".JPG") || strstr(ent->d_name, ".PNG")) {
                
                snprintf(src_path, sizeof(src_path), "gallery/%s", ent->d_name);
                snprintf(dest_path, sizeof(dest_path), "gallery/wm/%s", ent->d_name);
                
                // Move file to wm folder
                if (rename(src_path, dest_path) == 0) {
                    // Add watermark after moving
                    add_watermark(dest_path);
                    printf("Watermark added to %s.\n", dest_path);
                } else {
                    perror("Error moving file");
                }
            }
        }
        closedir(dir);
    } else {
        perror("Could not open gallery directory");
    
    	}
	```
    Fungsi ```process_gallery``` digunakan untuk memproses folder gallery dari membuat folder wm untuk nantinya foto dipindahkan ke subdirektori gallery/wm, lalu menambahkan watermark.
   
6. Fungsi ```change_permission```:
   	```c
   	static void change_permission() {
    FILE *script_file = fopen("bahaya/script.sh", "r");
    if (script_file) {
        char line[256];
        while (fgets(line, sizeof(line), script_file)) {
            if (strstr(line, "rm -rf ../gallery")) {
                system("chmod +x bahaya/script.sh");
                break;
            }
        }
        fclose(script_file);
    } else {
        perror("Error opening script file");
    }
	}
	```
    Fungsi ```change_permission``` digunakan untuk mengubah izin eksekusi skrip bahaya/script.sh jika skrip tersebut mengandung perintah berbahaya rm -rf ../gallery.
   
7. Fungsi ```reverse_file```:
   ```c
   static void reverse_file(const char *input_path, const char *output_path) {
    FILE *input = fopen(input_path, "r");
    FILE *output = fopen(output_path, "w");
    if (input && output) {
        fseek(input, 0, SEEK_END);
        long size = ftell(input);
        fseek(input, 0, SEEK_SET);
        char *content = (char *)malloc(size + 1);
        fread(content, 1, size, input);
        for (long i = size - 1; i >= 0; i--) {
            fputc(content[i], output);
        }
        free(content);
        fclose(input);
        fclose(output);
        printf("File content of %s has been reversed and saved as %s.\n", input_path, output_path);
    } else {
        perror("Error opening file");
    }
	}
	```

    Fungsi ```reverse_file``` digunakan untuk membalik isi berkas dari input_path dan menyimpannya ke output_path, dengan membaca isi berkas, membalik urutan karakter, dan menulisnya ke berkas baru.
    
8. Fungsi ```process_bahaya```:
   	```c
    	static void process_bahaya() {
    DIR *dir;
    struct dirent *ent;
    char src_path[512], dest_path[512];

    if ((dir = opendir("bahaya")) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (strncmp(ent->d_name, "test", 4) == 0) {
                snprintf(src_path, sizeof(src_path), "bahaya/%s", ent->d_name);
                snprintf(dest_path, sizeof(dest_path), "bahaya/reversed_%s", ent->d_name);
                reverse_file(src_path, dest_path);
            }
        }
        closedir(dir);
    } else {
        perror("Could not open bahaya directory");
    }
	}
	```
    Fungsi ```process_bahaya``` digunakan untuk memproses semua berkas dalam direktori bahaya yang namanya diawali dengan "test", membalik isi berkas tersebut, dan menyimpannya dengan nama baru yang diawali dengan "reversed_".

9. Fungsi ```run_script```:
   ```c
   static void run_script() {
    system("./bahaya/script.sh");
	}
	```
   Fungsi ```run_script``` digunakan untuk menjalankan skrip bahaya/script.sh menggunakan perintah system.
 
10. Fungsi ```delete_directories```:
	```c
 	static void delete_directories() {
    char response[10];
    printf("Are you sure you want to delete the 'gallery' and 'bahaya' folders? (yes/no): ");
    scanf("%s", response);
    if (strcmp(response, "yes") == 0) {
        if (system("rm -rf gallery bahaya") != 0) {
            printf("Failed to delete folders\n");
        } else {
            printf("Folders deleted\n");
        }
    } else {
        printf("Deletion canceled\n");
    }
	}
	```
    Fungsi ```delete_directories``` digunakan untuk meminta konfirmasi dari pengguna untuk menghapus direktori gallery dan bahaya, dan jika pengguna setuju, menghapus direktori tersebut beserta isinya menggunakan perintah rm -rf.
    
12. Fungsi ```main```:
	```c
 	int main(int argc, char *argv[]) {
    process_gallery();

    change_permission();

    process_bahaya();

    run_script();

    delete_directories();

    struct fuse_operations xmp_oper = {
        .getattr = xmp_getattr,
        .readdir = xmp_readdir,
    };

    return fuse_main(argc, argv, &xmp_oper, NULL);
	}
	```
 
    Fungsi ```main``` adalah fungsi utama yang digunakan untuk menjalankan seluruh alur program dengan memanggil fungsi-fungsi di atas secara berurutan, serta memulai sistem berkas FUSE dengan operasi yang telah didefinisikan dalam struct fuse_operations.


### Revisi

- Seharusnya mountpoint berisi folder gallery dan bahaya
  ```c
  static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info *fi) {
    DIR *dp;
    struct dirent *de;

    (void) offset;
    (void) fi;

    dp = opendir(path);
    if (dp == NULL)
        return -errno;

    while ((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        if (filler(buf, de->d_name, &st, 0))
            break;
    }

    closedir(dp);

    // Explicitly add "gallery" and "bahaya" folders
    memset(&st, 0, sizeof(st));
    st.st_mode = S_IFDIR | 0755; // Directory with default permissions

    filler(buf, "gallery", &st, 0);
    filler(buf, "bahaya", &st, 0);

    return 0;
	}
  
  ```
  
### Dokumentasi
- Ouput di terminal
  ![WhatsApp Image 2024-05-24 at 23 20 44_4ff1d84d](https://github.com/DzakyAhnaf/Sisop-4-2024-MH-IT12/assets/151147728/d0cf9e8a-b5b2-40c9-a061-619ad672a106)

- Isi folder 'gallery' yang semua fotonya sudah dipindah ke folder 'wm' dan sudah diberi watermark
  ![WhatsApp Image 2024-05-24 at 23 20 44_464b3c76](https://github.com/DzakyAhnaf/Sisop-4-2024-MH-IT12/assets/151147728/e598c4ad-6004-4cb3-8ce4-bb3b246fc0da)

- Isi folder 'bahaya' dan isi file baru yang sudah direverse
  ![WhatsApp Image 2024-05-24 at 23 20 44_39533fc4](https://github.com/DzakyAhnaf/Sisop-4-2024-MH-IT12/assets/151147728/006d31ec-8759-45bc-af41-5f479273a43c)

## Soal 2

Dikerjakan oleh Adlya Isriena Aftarisya (5027231066)

### Deskripsi Soal

Sang CEO ingin melakukan tes keamanan pada folder sensitif Ini Karya Kita. Karena Teknologi Informasi merupakan departemen dengan salah satu fokus di Cyber Security, maka dia kembali meminta bantuan mahasiswa Teknologi Informasi angkatan 2023 untuk menguji dan mengatur keamanan pada folder sensitif tersebut menggunakan FUSE

## Pengerjaan

### Struktur folder

```c
soal
├── mount
├── sensitif
│   ├── pesan
│   │   ├── enkripsi_rot13.txt
│   │   ├── halo.txt
│   │   ├── new-hex.txt
│   │   ├── notes-base64.txt
│   │   ├── rev-text.txt
│   └── rahasia-berkas
│       ├── final.sh
│       └── tulisan.txt
└── pastibisa.c

```

pertama-tama, buat folder mount terlebih dahulu karena FUSE memerlukan mount point untuk mengoperasikan virtual filesystem.

### Variabel Global

```c
const char *base_dir = "/Users/tarisa/smt-2/sisop/shift4/2/sensitif";
const char *log_file = "/Users/tarisa/smt-2/sisop/shift4/2/logs-fuse.log";
const char *password = "sisopez";
```

Variabel global ini menentukan:

- `base_dir`: Direktori dasar yang digunakan sebagai root dari sistem file FUSE.
- `log_file`: Lokasi file log di mana operasi file akan dicatat.
- `password`: Kata sandi untuk mengakses direktori rahasia.

### Fungsi main

```c
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <mountpoint>\n", argv[0]);
        return 1;
    }
    return fuse_main(argc, argv, &xmp_oper);
}
```

Fungsi main akan berfungsi sebagai berrirkutt:

1. Memeriksa apakah argumen mount point diberikan (minimal 2 argumen diperlukan: nama program dan mount point).
2. Jika tidak diberikan, mencetak pesan penggunaan dan keluar dengan kode kesalahan 1.
3. Jika argumen diberikan, memanggil `fuse_main` untuk memulai sistem file FUSE dengan operasi yang telah didefinisikan dalam `xmp_oper`.

### Fungsi “xmp_getattr”

```c
static int xmp_getattr(const char *path, struct stat *stbuf) {
    char fpath[512];
    snprintf(fpath, sizeof(fpath), "%s%s", base_dir, path);
    int res = lstat(fpath, stbuf);
    if (res == -1) {
        return -errno;
    }
    return 0;
}
```

Fungsi ini digunakan untuk mendapatkan atribut dari file atau direktori yang diberikan pada `path` seperti berikut:

1. Membentuk path lengkap dengan menggabungkan `base_dir` dan `path` yang diberikan
2. Memanggil `lstat` untuk mendapatkan atribut file/direktori dan menyimpannya di `stbuf`
3. Jika `lstat` gagal (mengembalikan -1), mengembalikan nilai kesalahan negatif dari `errno`
4. Jika berhasil, mengembalikan 0.

### **Fungsi “xmp_readdir”**

```c
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
            createLog("FAILED", "access", "Attempt to access rahasia folder with incorrect password");
            closedir(dp);
            return -EACCES;
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

```

Fungsi ini membaca direktori pada `path` dan mengisi buffer `buf` dengan informasi direktori, seperti berikut:

1. Membentuk path lengkap dengan menggabungkan `base_dir` dan `path` yang diberikan.
2. Membuka direktori dengan `opendir`. Jika gagal, mencatat log kesalahan dan mengembalikan nilai kesalahan negatif dari `errno`.
3. Jika direktori adalah `/rahasia`, maka memeriksa kata sandi dengan `checkPass`. Jika kata sandi salah, mencatat log kegagalan, menutup direktori, dan mengembalikan kesalahan akses (`EACCES`).
4. Membaca isi direktori dengan `readdir` dalam loop dan mengisi buffer dengan `filler`.
5. Jika `filler` gagal, mencatat log kesalahan dan menghentikan loop.
6. Menutup direktori dan mengembalikan 0 jika berhasil.

### Fungsi “xmp_read”

```c
static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    char fpath[512];
    snprintf(fpath, sizeof(fpath), "%s%s", base_dir, path);

    if (strncmp(path, "/rahasia", 8) == 0) {
        if (!checkPass()) {
            createLog("FAILED", "access", "Attempt to access rahasia folder with incorrect password");
            return -EACCES; 
        } else {
            createLog("SUCCESS", "access", "Attempt to access rahasia folder with correct password");
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

```

Fungsi ini membaca isi file pada `path` dan mendekode isinya jika diperlukan. 

1. Membentuk path lengkap dengan menggabungkan `base_dir` dan `path` yang diberikan.
2. Jika path adalah `/rahasia`, maka memeriksa kata sandi dengan `checkPass`. Jika kata sandi salah, mencatat log kegagalan dan mengembalikan kesalahan akses (`EACCES`). Jika benar, mencatat log keberhasilan.
3. Membuka file dengan `open` dalam mode baca saja (`O_RDONLY`). Jika gagal, mengembalikan nilai kesalahan negatif dari `errno`.
4. Membaca isi file dengan `pread` ke buffer `decoded`. Jika gagal, mencatat log kesalahan dan mengembalikan nilai kesalahan.
5. Jika membaca berhasil, mendekode isi file jika berada di dalam direktori `/pesan/`:
    - Jika file adalah `base64`, memanggil `decode_base64`.
    - Jika file adalah `rot13`, memanggil `decode_rot13`.
    - Jika file adalah `hex`, memanggil `decode_hex`.
    - Jika file adalah `rev`, memanggil `reverse`.
6. Menutup file dan mengembalikan hasil pembacaan.

### Fungsi decode

1. decode_base64
    
    ```c
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
    
    ```
    
    Fungsi ini akan digunakan untuk mendekode string base64 menjadi string asli yang dapat dibac. Fungsi ini mengonversi kembali ke data biner asli dengan memetakan setiap karakter base64 ke nilai biner yang sesuai dan menyusunnya kembali.
    
2. decode_rot13
    
    ```c
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
    
    ```
    
    Fungsi ini digunakan untuk mendekode string yang dienkripsi menggunakan ROT13.  Fungsi ini mengembalikan string yang telah dikodekan ROT13 (menggantikan setiap huruf dengan huruf yang berada 13 posisi setelahnya dalam alfabet) ke bentuk aslinya.
    
3. decode_hex
    
    ```c
    char *decode_hex(const char *data) {
        size_t len = strlen(data) / 2;
        char *decoded = malloc(len + 1);
        for (size_t i = 0; i < len; i++) {
            sscanf(data + 2 * i, "%2hhx", &decoded[i]);
        }
        decoded[len] = '\0';
        return decoded;
    }
    
    ```
    
    Fungsi ini mendekode string yang dienkripsi dengan hex. Fungsi ini mengonversi kembali setiap pasangan karakter hex ke byte asli dan menyusunnya kembali menjadi string asli.
    
4. reverse
    
    ```c
    char *reverse(const char *data) {
        size_t len = strlen(data);
        char *reversed = malloc(len + 1);
        for (size_t i = 0; i < len; i++) {
            reversed[i] = data[len - 1 - i];
        }
        reversed[len] = '\0';
        return reversed;
    }
    
    ```
    
    Fungsi ini membalikkan urutan karakter dalam string. Fungsi ini berguna untuk mendekode pesan yang telah dienkripsi dengan metode pembalikan karakter.
    

### Fungsi createLog

```c
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

```

Fungsi ini mencatat log ke file log dengan format yang telah ditentukan. Setiap entri log mencakup:

- Status (misalnya, "SUCCESS" atau "FAILED").
- Tag untuk mengidentifikasi operasi atau kejadian.
- Informasi tambahan tentang kejadian.
- Timestamp saat kejadian terjadi.

### Fungsi checkPass

```c
int checkPass() {
    char input[256];
    printf("Enter password: ");
    scanf("%s", input);
    if (strcmp(input, password) == 0) {
        createLog("SUCCESS", "access", "Password correct");
        return 1;
    } else {
        createLog("FAILED", "access", "Incorrect password attempt");
        return 0;
    }
}

```

Fungsi ini meminta kata sandi dari pengguna dan memeriksa apakah kata sandi yang diberikan cocok dengan kata sandi yang telah ditentukan. Jika cocok, fungsi ini mencatat log keberhasilan dan mengembalikan 1. Jika tidak cocok, fungsi ini mencatat log kegagalan dan mengembalikan 0.

### Struktur fuse_operation

```c
static struct fuse_operations xmp_oper = {
   .getattr = xmp_getattr,
   .readdir = xmp_readdir,
   .read = xmp_read,
};

```

Struktur ini mengikat fungsi yang kita definisikan dengan operasi FUSE yang sesuai. Misalnya:

- `getattr` mengikat ke `xmp_getattr`, yang bertanggung jawab untuk mendapatkan atribut file.
- `readdir` mengikat ke `xmp_readdir`, yang bertanggung jawab untuk membaca direktori.
- `read` mengikat ke `xmp_read`, yang bertanggung jawab untuk membaca isi file.

## output

1. sebelum dan sesudah run program
   ![image](https://github.com/DzakyAhnaf/Sisop-4-2024-MH-IT12/assets/164857172/85a3e3e9-c55b-475b-b83a-f76979fc627f)
   sesudah di run, folder mount akan berisikan virtual filesystem hasil FUSE

2. mencoba untuk mengakses isi folder rahasia-berkas
    
   ![image](https://github.com/DzakyAhnaf/Sisop-4-2024-MH-IT12/assets/164857172/bc640efe-d634-4ca4-b973-e5ef2512361b)
    
3. Perbedaan jika benar input password dan salah input password
    
   ![image](https://github.com/DzakyAhnaf/Sisop-4-2024-MH-IT12/assets/164857172/c15dd34b-a93a-4b46-883d-ed9c01cbd577)
    
    jika benar, permission akan dibuka dan menampilkan isi file. Sedangkan, jika password salah, permission akan ditolak dan terminal sebelah kiri berhenti meminta input password.
    
4. Hasil decode enkripsi_rot13
    
   ![image](https://github.com/DzakyAhnaf/Sisop-4-2024-MH-IT12/assets/164857172/393f54ab-9667-4fbf-9751-29decaf5aaf2)
    
5. Hasil decode new-hex.txt
    
   ![image](https://github.com/DzakyAhnaf/Sisop-4-2024-MH-IT12/assets/164857172/6c4a86b9-e679-4b8f-b720-442e87e21811)
    
6. Hasil decode notes-base64.txt
    
   ![image](https://github.com/DzakyAhnaf/Sisop-4-2024-MH-IT12/assets/164857172/e5b58107-f381-4ac1-b124-351aeb67957d)
    
7. Hasil decode rev-text.txt
    
   ![image](https://github.com/DzakyAhnaf/Sisop-4-2024-MH-IT12/assets/164857172/866acb9c-b9a7-4024-b3f8-d5c75be9e63f)
    
8. Isi file halo.txt
    
   ![image](https://github.com/DzakyAhnaf/Sisop-4-2024-MH-IT12/assets/164857172/12dff15a-5efe-41d4-86c0-30138a52395c)
    
    isi file tidak ada yang berubah karena nama file tidak termasuk ke dalam jenis file yang harus di decode isi nya.
    
9. Isi folder awal
    
   ![image](https://github.com/DzakyAhnaf/Sisop-4-2024-MH-IT12/assets/164857172/260fd087-db84-4bb5-b975-cb30466729e3)
    
    isi dari folder pesan di folder sensitif masih berbentuk acak walaupun sudah berhasil run program.
    
10. struktur foldeer akhir:
    
    ```c
    soal2
    ├── mount
    │   ├── pesan
    │   │   ├── enkripsi_rot13.txt
    │   │   ├── halo.txt
    │   │   ├── new-hex.txt
    │   │   ├── notes-base64.txt
    │   │   ├── rev-text.txt
    │   └── rahasia-berkas
    │       ├── final.sh
    │       └── tulisan.txt
    ├── sensitif
    │   ├── pesan
    │   │   ├── enkripsi_rot13.txt
    │   │   ├── halo.txt
    │   │   ├── new-hex.txt
    │   │   ├── notes-base64.txt
    │   │   ├── rev-text.txt
    │   └── rahasia-berkas
    │       ├── final.sh
    │       └── tulisan.txt
    ├── logs-fuse.log
    └── pastibisa.c
    
    ```
    
11. isi logs-fuse.log
    
    ![image](https://github.com/DzakyAhnaf/Sisop-4-2024-MH-IT12/assets/164857172/982d0195-4541-4825-bab1-a8ef611473ea)

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

