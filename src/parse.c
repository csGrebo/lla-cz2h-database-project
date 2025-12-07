#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include "parse.h"
#include "common.h"

int create_db_header(struct dbheader_t **headerOut) {
    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    if (header == NULL) {
        printf("Calloc failed to create db header\n");
        return STATUS_ERROR;
    }
    header->version = 0x1;
    header->count = 0;
    header->magic = HEADER_MAGIC;
    header->filesize = sizeof(struct dbheader_t);
    *headerOut = header;
    return STATUS_SUCCESS;
}

int validate_db_header(int fd, struct dbheader_t **headerOut) {
    if (fd < 0) {
        printf("Recieved invalid File Descriptor\n");
        return STATUS_ERROR;
    }

    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    if (header == NULL) {
        printf("Calloc failed to create db header\n");
        return STATUS_ERROR;
    }

    if (read(fd, header, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t)) {
        perror("read");
        free(header);
        return STATUS_ERROR;
    }

    header->version = ntohs(header->version);
    header->count = ntohs(header->count);
    header->magic = ntohl(header->magic);
    header->filesize = ntohl(header->filesize);

    if (header->magic != HEADER_MAGIC) {
        printf("Improper header magic\n");
        free(header);
        return STATUS_ERROR;
    }

    if (header->version != 1) {
        printf("Improper header version\n");
        free(header);
        return STATUS_ERROR;
    }

    struct stat dbstat = {0};
    fstat(fd, &dbstat);
    if (header->filesize != dbstat.st_size) {
        printf("Corrupted database\n");
        free(header);
        return STATUS_ERROR;
    }
    *headerOut = header;
    return STATUS_SUCCESS;
}

int read_employees(int fd, struct dbheader_t *header, struct employee_t **employeesOut) {
    if (fd < 0) {
        printf("Recieved invalid File Descriptor\n");
        return STATUS_ERROR;
    }

    int count = header->count;

    struct employee_t *employees = calloc(count, sizeof(struct employee_t));
    if (employees == NULL) {
        printf("Calloc failed\n");
        return STATUS_ERROR;
    }

    read(fd, employees, count * sizeof(struct employee_t));

    int i = 0;
    for (; i < count; i++) {
        employees[i].hours = ntohl(employees[i].hours);
    }

    *employeesOut = employees;

    return STATUS_SUCCESS;
}

int output_file(int fd, struct dbheader_t *header, struct employee_t *employees) {
    if (fd < 0) {
        printf("Recieved invalid File Descriptor\n");
        return STATUS_ERROR;
    }

    int realcount = header->count;
    int realsize = sizeof(struct dbheader_t) + realcount*sizeof(struct employee_t);

    header->magic = htonl(header->magic);
    header->filesize = htonl(realsize);
    header->count = htons(header->count);
    header->version = htons(header->version);

    lseek(fd, 0, SEEK_SET);

    write(fd, header, sizeof(struct dbheader_t));


    int i = 0;
    for (; i < realcount; i++) {
        employees[i].hours = htonl(employees[i].hours);
        write(fd, &employees[i], sizeof(struct employee_t));
    }
    ftruncate(fd, realsize);

    return STATUS_SUCCESS;
}

int add_employee(struct dbheader_t *header, struct employee_t **employees, char *addstr) {
    if (NULL == header) return STATUS_ERROR;
    if (NULL == employees) return STATUS_ERROR;
    if (NULL == *employees) return STATUS_ERROR;
    if (NULL == addstr) return STATUS_ERROR;

    printf("%s\n", addstr);

    char *name = strtok(addstr, ",");
    char *addr = strtok(NULL, ",");
    char *hours = strtok(NULL, ",");

    printf("%s %s %s\n", name, addr, hours);

    struct employee_t *e = *employees;
    e = realloc(e, sizeof(struct employee_t)*header->count+1);
    if (e == NULL) {
        return STATUS_ERROR;
    }
    header->count++;

    strncpy(e[header->count-1].name, name, sizeof(e[header->count-1].name)-1);
    strncpy(e[header->count-1].address, addr, sizeof(e[header->count-1].address)-1);
    e[header->count-1].hours = atoi(hours);
    *employees = e;

    return STATUS_SUCCESS;
}

void list_employees(struct dbheader_t *header, struct employee_t *employees) {
    // if (header->count == 0) printf("No employees to list\n");
    int i = 0;
    for (; i < header->count; i++) {
        printf("Employee %d\n", i);
        printf("\tName: %s\n", employees[i].name);
        printf("\tAddress: %s\n", employees[i].address);
        printf("\tHours: %d\n", employees[i].hours);
        printf("\n");
    }
}

int remove_employee(struct dbheader_t *header, struct employee_t **employees, char *delstr) {
    struct employee_t *etmp = *employees;
    int cnt = header->count;
    int ncnt = 0;
    int i = 0;
    for (; i < cnt; i++) {
        if (strcmp(etmp[i].name, delstr) != 0) {
            ncnt++;
        }
    }
    struct employee_t *nemp = calloc(ncnt, sizeof(struct employee_t));
    if (nemp == NULL) {
        return STATUS_ERROR;
    }
    int destidx = 0;
    for (int k = 0; k < cnt; k++) {
        if (strcmp(etmp[k].name, delstr) != 0) {
            nemp[destidx] = etmp[k];
            destidx++;
        }
    }
    header->count = ncnt;
    *employees = nemp;
    free(etmp);
    etmp = NULL;
    return STATUS_SUCCESS;
}

int update_employee(struct dbheader_t *header, struct employee_t **employees, char *updstr) {
    if (NULL == updstr) return STATUS_ERROR;
    struct employee_t *etmp = *employees;

    char *name = strtok(updstr, "|");
    char *addr = strtok(NULL, "|");
    char *hours = strtok(NULL, "|");

    if (NULL == name) return STATUS_ERROR;
    int i = 0;
    for (; i < header->count; i++) {
        if (strcmp(etmp[i].name, name) == 0) {
            if (NULL != addr && strcmp("", addr) != 0) {
                if (strlen(addr) <= 255) strncpy(etmp[i].address, addr, strlen(addr));
            }
            if (NULL != hours && strcmp("", hours) != 0) {
                etmp[i].hours = atoi(hours);
            }
        }
    }
    *employees = etmp;
    return STATUS_SUCCESS;
}
