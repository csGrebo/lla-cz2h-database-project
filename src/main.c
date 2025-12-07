#include <stdbool.h>
#include   <stdio.h>
#include  <getopt.h>
#include  <stdlib.h>

#include  "common.h"
#include    "file.h"
#include   "parse.h"

void print_usage(char *argv[]) {
    printf("Usage: %s -n -f <database file>\n", argv[0]);
    printf("\t -n  - create new database file\n");
    printf("\t -f  - (required) path to database file\n");
    printf("\t -a  - Add a new employee via CSV Entry (name,address,hours)\n");
    printf("\t -l  - List known employees\n");
}

int main(int argc, char *argv[]) {
    int c = 0;
    bool newfile = false;
    char *filepath = NULL;
    char *addstr = NULL;
    bool list = false;
    char *delstr = NULL;
    char *updstr = NULL;

    int dbfd = -1;
    struct dbheader_t *header = NULL;
    struct employee_t *employees = NULL;

    while ((c = getopt(argc, argv, "hlnf:a:d:u:")) != -1) {
        switch (c) {
            case 'h':
                print_usage(argv);
                return 0;
            case 'f':
                filepath = optarg;
                break;
            case 'n':
                newfile = true;
                break;
            case 'a':
                addstr = optarg;
                break;
            case 'l':
                list = true;
                break;
            case 'd':
                delstr = optarg;
                break;
            case 'u':
                updstr = optarg;
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
        if (create_db_header(&header) == STATUS_ERROR) {
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

    if (read_employees(dbfd, header, &employees) != STATUS_SUCCESS) {
        printf("Failed to read employees");
        return -1;
    }

    if (addstr) {
#if 0
        add_employee(header, &employees, addstr);
#else
        header->count++;
        employees = realloc(employees, header->count*(sizeof(struct employee_t)));
        add_employee(header, employees, addstr);
#endif
    }

    if (delstr) {
        remove_employee(header, &employees, delstr);
    }

    if (updstr) {
        update_employee(header, &employees, updstr);
    }

    if (list) {
        list_employees(header, employees);
    }

    output_file(dbfd, header, employees);

    free(header);
    free(employees);

    return 0;
}
