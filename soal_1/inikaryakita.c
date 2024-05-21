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
