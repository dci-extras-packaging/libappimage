// system includes
#include <fcntl.h>

// library includes
#include <archive.h>
#include <archive_entry.h>

// local includes
#include "type2.h"
#include "type1.h"


void appimage_type1_open(appimage_handler* handler) {
    if (is_handler_valid(handler) && !handler->is_open) {
#ifdef STANDALONE
        fprintf(stderr, "Opening %s as Type 1 AppImage\n", handler->path);
#endif
        struct archive* a;
        a = archive_read_new();
        archive_read_support_format_iso9660(a);
        if (archive_read_open_filename(a, handler->path, 10240) != ARCHIVE_OK) {
            fprintf(stderr, "%s", archive_error_string(a));
            handler->cache = NULL;
            handler->is_open = false;
        } else {
            handler->cache = a;
            handler->is_open = true;
        }
    }
}

void appimage_type1_close(appimage_handler* handler) {
    if (is_handler_valid(handler) && handler->is_open) {
#ifdef STANDALONE
        fprintf(stderr, "Closing %s\n", handler->path);
#endif
        struct archive* a = handler->cache;
        archive_read_close(a);
        archive_read_free(a);

        handler->cache = NULL;
        handler->is_open = false;
    }
}

void type1_traverse(appimage_handler* handler, traverse_cb command, void* command_data) {
    appimage_type1_open(handler);

    if (!command) {
#ifdef STANDALONE
        fprintf(stderr, "No traverse command set.\n");
#endif
        return;
    }

    if (handler->is_open) {
        struct archive* a = handler->cache;
        struct archive_entry* entry;
        int r;

        for (;;) {
            r = archive_read_next_header(a, &entry);
            if (r == ARCHIVE_EOF) {
                break;
            }
            if (r != ARCHIVE_OK) {
                fprintf(stderr, "%s\n", archive_error_string(a));
                break;
            }

            /* Skip all but regular files; FIXME: Also handle symlinks correctly */
            if (archive_entry_filetype(entry) != AE_IFREG) {
                continue;
            }

            command(handler, entry, command_data);
        }
    }

    appimage_type1_close(handler);
}

// TODO: remove forward declaration
gchar* replace_str(const gchar* src, const gchar* find, const gchar* replace);

char* type1_get_file_name(appimage_handler* handler, void* data) {
    (void) handler;

    struct archive_entry* entry = (struct archive_entry*) data;

    char* filename = replace_str(archive_entry_pathname(entry), "./", "");
    return filename;
}

void type1_extract_file(appimage_handler* handler, void* data, const char* target) {
    (void) data;

    struct archive* a = handler->cache;
    mk_base_dir(target);

    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    int f = open(target, O_WRONLY | O_CREAT | O_TRUNC, mode);

    if (f == -1){
#ifdef STANDALONE
        fprintf(stderr, "open error: %s\n", target);
#endif
        return;
    }

    archive_read_data_into_fd(a, f);
    close(f);
}

bool type1_read_file_into_buf(struct appimage_handler* handler, void* data, char** buffer, unsigned long* buf_size) {
    (void) data;

    struct archive* a = handler->cache;

    struct archive_entry* entry = data;

    int64_t file_size = archive_entry_size(entry);

    char* new_buffer = (char*) malloc(sizeof(char) * file_size);

    if (new_buffer == NULL) {
#ifdef STANDALONE
        fprintf(stderr, "failed to allocate enough memory for buffer (required: %ul bytes)\n", file_size);
#endif
        return false;
    }

    if (archive_read_data(a, new_buffer, (size_t) file_size) < 0) {
#ifdef STANDALONE
        fprintf(stderr, "failed to read data into buffer: %s\n", archive_error_string(a));
#endif
        free(new_buffer);
        return false;
    }

    *buffer = new_buffer;
    *buf_size = (unsigned long) file_size;
    return true;
}

char* type1_get_file_link(struct appimage_handler* handler, void* entry_ptr) {
    struct archive_entry* entry = entry_ptr;

    const char* link_path = archive_entry_symlink(entry) ?: archive_entry_hardlink(entry);

    if (link_path) {
        char* filename = replace_str(link_path, "./", "");
        return filename;
    }

    return NULL;
}

appimage_handler appimage_type_1_create_handler() {
    appimage_handler h;
    h.traverse = type1_traverse;
    h.get_file_name = type1_get_file_name;
    h.extract_file = type1_extract_file;
    h.get_file_link = type1_get_file_link;
    h.read_file_into_new_buffer = type1_read_file_into_buf;
    h.type = 1;

    return h;
}

bool match_type_1_magic_bytes(const char* buffer) {
    return buffer[0] == 0x41 && buffer[1] == 0x49 && buffer[2] == 0x01;
}

bool is_iso_9660_file(const char* path) {
    /* Implementation of the signature matches expressed at https://www.garykessler.net/library/file_sigs.html
     * Signature: 43 44 30 30 31 	  	= "CD001"
     * ISO 	  	ISO-9660 CD Disc Image
     * This signature usually occurs at byte offset 32769 (0x8001),
     * 34817 (0x8801), or 36865 (0x9001).
     * More information can be found at MacTech or at ECMA.
     */

    bool res = false;
    FILE* f = fopen(path, "rt");
    if (f != NULL) {
        char buffer[5] = {0};

        int positions[] = {32769, 34817, 36865};
        const char signature[] = "CD001";
        for (int i = 0; i < 3 && !res; i++) {
            int fseekRes = fseek(f, positions[i], SEEK_SET);
            if (!fseekRes) {
                fread(buffer, 1, 5, f);
                int strCmpRes = memcmp(signature, buffer, 5);
                if (!strCmpRes)
                    res = true;
            }
            memset(buffer, 0, 5);
        }

        fclose(f);
    }
    return res;
}
