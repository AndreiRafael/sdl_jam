#ifndef HF_DIR_H
#define HF_DIR_H

#ifndef HF_DIR_PATH_SIZE
#define HF_DIR_PATH_SIZE 260
#endif

typedef enum hf_dir_entry_type_t {
    hf_dir_entry_unknown,
    hf_dir_entry_folder,
    hf_dir_entry_file,
} hf_dir_entry_type;

typedef struct hf_dir_entry_t {
    hf_dir_entry_type type;
    char* path;
} hf_dir_entry;

hf_dir_entry* hf_dir_entry_create(const char* path);
void hf_dir_entry_destroy(hf_dir_entry* entry);
// create an array of directory_entry given a path that must be a folder
hf_dir_entry** hf_dir_entries_create(const char* path, size_t* count);
void hf_dir_entries_destroy(hf_dir_entry** entries, size_t count);

#endif