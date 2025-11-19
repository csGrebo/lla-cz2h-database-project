#include <stdbool.h>
#include   <stdio.h>
#include  <getopt.h>

#include  "common.h"
#include    "file.h"
#include   "parse.h"

void print_usage(char *argv[]) {
    printf("Usage: %s -n -f <database file>\n", argv[0]);
    printf("\t -n  - create new database file\n");
    printf("\t -f  - (required) path to database file\n");
}

int main(int argc, char *argv[]) {
    int c = 0;
    bool newfile = false;
    char *filepath = NULL;

    int dbfd = -1;
    struct dbheader_t *header = NULL;

    while ((c = getopt(argc, argv, "nf:")) != -1) {
        switch (c) {
            case 'f':
                filepath = optarg;
                break;
            case 'n':
                newfile = true;
                break;
            default:
                printf("Unknown option -%c\n", c);
                return -1;
        }
    }

    if (filepath == NULL) {
        printf("Filepath is a required argument\n");
        print_usage(argv);
        return 0;
    }

    if (newfile) {
        dbfd = create_db_file(filepath);
        if (dbfd == STATUS_ERROR) {
            printf("Unable to create database file\n");
            return -1;
        }
        if (create_db_header(dbfd, &header) == STATUS_ERROR) {
            printf("Failed to create database header\n");
            return -1;
        }
    } else {
        dbfd = open_db_file(filepath);
        if (dbfd == STATUS_ERROR) {
            printf("Unable to create database file\n");
            return -1;
        }
        if (validate_db_header(dbfd, &header) == STATUS_ERROR) {
            printf("Failed to validate database header\n");
            return -1;
        }
    }

    output_file(dbfd, header);

    return 0;
}
