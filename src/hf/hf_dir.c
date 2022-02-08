#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

#include <hf/hf_dir.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

hf_dir_entry* hf_dir_entry_create(const char* path) {
    size_t len = strlen(path);
    if(len == 0u) {
        return NULL;
    }
    char last_char = path[len - 1];
    size_t cpy_len = (last_char == '\\' || last_char == '/') ? len - 1 : len;
    char* real_path = malloc(sizeof(char) * (cpy_len + 1));
    strncpy(real_path, path, cpy_len);
    real_path[cpy_len] = '\0';
    
#if defined(_MSC_VER)
    WIN32_FIND_DATA data;
    HANDLE hFind = FindFirstFile(real_path, &data);
    if (hFind != INVALID_HANDLE_VALUE) {
        hf_dir_entry* new_entry = malloc(sizeof(hf_dir_entry));
        new_entry->path = malloc(sizeof(char) *  HF_DIR_PATH_SIZE);
        GetFullPathName(real_path, HF_DIR_PATH_SIZE, new_entry->path, NULL);

        FindClose(hFind);
        free(real_path);
        return new_entry;
    }
#else//use dirent if available
    struct stat file_stat;
    if(stat(path, &file_stat) != -1) {
        hf_dir_entry* new_entry = malloc(sizeof(hf_dir_entry));
        new_entry->path = malloc(sizeof(char) *  HF_DIR_PATH_SIZE);
        strcpy(new_entry->path, real_path);
        new_entry->type = S_ISDIR(file_stat.st_mode) ? hf_dir_entry_folder : hf_dir_entry_file;

        free(real_path);
        return new_entry;
    }
#endif
    free(real_path);
    return NULL;
}

void hf_dir_entry_destroy(hf_dir_entry* entry) {
    if(entry) {
        free(entry->path);
    }
    free(entry);
}

hf_dir_entry** hf_dir_entries_create(const char* path, size_t* count) {
    size_t len = strlen(path);
    if(len == 0u) {
        return NULL;
    }
    char last_char = path[len - 1];
    size_t cpy_len = (last_char == '\\' || last_char == '/') ? len - 1 : len;
    char* real_path = malloc(sizeof(char) * (cpy_len + 3));//enough space for '\\', '*' & '\0'
    strncpy(real_path, path, cpy_len);
    real_path[cpy_len] = '/';
    real_path[cpy_len + 1] = '*';
    real_path[cpy_len + 2] = '\0';

#if defined(_MSC_VER)
    WIN32_FIND_DATA data;
    HANDLE hFind = FindFirstFile(real_path, &data);
    if (hFind != INVALID_HANDLE_VALUE) {
        size_t file_count = 0u;
        do {
            file_count++;
        } while (FindNextFile(hFind, &data));
        *count = file_count;

        hf_dir_entry** out_entries = malloc(sizeof(hf_dir_entry*) * file_count);
        hFind = FindFirstFile(real_path, &data);
        size_t index = 0;
        do {
            char* file_path = malloc(sizeof(char) *  HF_DIR_PATH_SIZE);
            strncpy(file_path, real_path, cpy_len + 1);
            char* name_ptr = file_path + (cpy_len + 1);
            strcpy(name_ptr, data.cFileName);
            out_entries[index++] = hf_dir_entry_create(file_path);

            free(file_path);
        } while (FindNextFile(hFind, &data));
        FindClose(hFind);

        free(real_path);
        return out_entries;
    }
#else
    (void)count;
    DIR* dir = opendir(path);
    if(dir) {
        struct dirent* entry;
        size_t file_count = 0u;
        while ((entry = readdir (dir)) != NULL) {
            file_count++;
        }
        *count = file_count;

        hf_dir_entry** out_entries = malloc(sizeof(hf_dir_entry*) * file_count);
        rewinddir(dir);
        size_t index = 0;
        while ((entry = readdir (dir)) != NULL) {
            char* file_path = malloc(sizeof(char) *  HF_DIR_PATH_SIZE);
            strncpy(file_path, real_path, cpy_len + 1);
            char* name_ptr = file_path + (cpy_len + 1);
            strcpy(name_ptr, entry->d_name);
            out_entries[index++] = hf_dir_entry_create(file_path);

            free(file_path);
        }
        closedir(dir);
        free(real_path);
        return out_entries;
    }
#endif
    free(real_path);
    return NULL;
}

void hf_dir_entries_destroy(hf_dir_entry** entries, size_t count) {
    for(size_t i = 0u; i < count; i++) {
        hf_dir_entry_destroy(entries[i]);
    }
    free(entries);
}