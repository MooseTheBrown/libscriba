/*
 * Copyright (C) 2015 Mikhail Sapozhnikov
 *
 * This file is part of libscriba.
 *
 * libscriba is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libscriba is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libscriba. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "scriba.h"
#include "company.h"
#include "poc.h"
#include "project.h"
#include "event.h"
#include "serializer.h"
#include <linux/time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

// return values
#define OK              0
#define INVALID_ARGS    1
#define USAGE           2

// defaults
#define DEFAULT_NUM_COMPANIES   100
#define DEFAULT_POC_PER_COMPANY 10
#define DEFAULT_PROJ_PER_POC    5
#define DEFAULT_EVENT_PER_PROJ  10

// temporary database location
#define TEMP_DB     "temp_db"
#define OUTPUT_FILE "scriba_test_db"

struct
{
    int num_companies;
    int people_per_company;
    int proj_per_poc;
    int events_per_proj;
} params;

int parse_args(int argc, char **argv);

int parse_args(int argc, char **argv)
{
    if (argc == 1)
    {
        // no args, set defaults
        params.num_companies = DEFAULT_NUM_COMPANIES;
        params.people_per_company = DEFAULT_POC_PER_COMPANY;
        params.proj_per_poc = DEFAULT_PROJ_PER_POC;
        params.events_per_proj = DEFAULT_EVENT_PER_PROJ;
        return OK;
    }

    if (!strcmp("-h", argv[1]))
    {
        // print usage
        printf("available options:\n");
        printf("-c <num_companies>\tnumber of companies\n");
        printf("-P <num_people>\tnumber of people per company\n");
        printf("-p <num_projects>\tnumber of projects per person\n");
        printf("-e <num_events>\tnumber of events per project\n");
        return USAGE;
    }

    for (int i = 1; i < argc; i++)
    {
        if (!strcmp("-c", argv[i]))
        {
            if ((i + 1) == argc)
            {
                printf("Missing value for option -c\n");
                return INVALID_ARGS;
            }
            else
            {
                params.num_companies = atoi(argv[i + 1]);
                i++;
            }
        }
        else if (!strcmp("-P", argv[i]))
        {
            if ((i + 1) == argc)
            {
                printf("Missing value for option -P\n");
                return INVALID_ARGS;
            }
            else
            {
                params.people_per_company = atoi(argv[i + 1]);
                i++;
            }
        }
        else if (!strcmp("-p", argv[i]))
        {
            if ((i + 1) == argc)
            {
                printf("Missing value for option -p\n");
                return INVALID_ARGS;
            }
            else
            {
                params.proj_per_poc = atoi(argv[i + 1]);
                i++;
            }
        }
        else if (!strcmp("-e", argv[i]))
        {
            if ((i + 1) == argc)
            {
                printf("Missing value for option -e\n");
                return INVALID_ARGS;
            }
            else
            {
                params.events_per_proj = atoi(argv[i + 1]);
                i++;
            }
        }
        else
        {
            printf("Invalid argument %s\n", argv[i]);
            printf("Run %s -h for the list of valid arguments\n", argv[0]);
            return INVALID_ARGS;
        }
    }

    return OK;
}

int main(int argc, char **argv)
{
    struct timespec start_ts;
    struct timespec generation_ts;
    struct timespec export_ts;

    int err = parse_args(argc, argv);
    if (err == USAGE)
    {
        return OK;
    }
    if (err != OK)
    {
        return err;
    }

    printf("Initializing scriba...");
    fflush(stdout);
    // initialize database
    struct ScribaDB db;
    db.name = "scriba_sqlite";
    db.type = SCRIBA_DB_BUILTIN;
    db.location = NULL;

    struct ScribaDBParam param1;
    param1.key = "db_loc";
    param1.value = TEMP_DB;

    struct ScribaDBParam param2;
    param2.key = "db_sync";
    param2.value = "off";

    struct ScribaDBParamList paramList[2];
    paramList[0].param = &param1;
    paramList[0].next = &paramList[1];
    paramList[1].param = &param2;
    paramList[1].next = NULL;

    err = scriba_init(&db, paramList);
    if (err != SCRIBA_INIT_SUCCESS)
    {
        printf("failed\n");
        return err;
    }
    printf("done\n");
    fflush(stdout);

    // add entries
    printf("Generating data...");
    fflush(stdout);
    clock_gettime(CLOCK_MONOTONIC_RAW, &start_ts);
    long entries = 0;
    for (int a = 0; a < params.num_companies; a++)
    {
        scriba_id_t company_id;
        scriba_id_create(&company_id);
        char name[50];
        char jur_name[50];
        char addr[50];
        char phone[50];
        char email[50];
        char inn[] = "1234567890";

        snprintf(name, 50, "Company %d", a + 1);
        snprintf(jur_name, 50, "Company %d LLC", a + 1);
        snprintf(addr, 50, "%d street", a + 1);
        snprintf(phone, 50, "555-%d", a + 1);
        snprintf(email, 50, "company%d@test.com", a + 1);

        scriba_addCompanyWithID(company_id, name, jur_name, addr,
                                inn, phone, email);
        entries++;

        for (int b = 0; b < params.people_per_company; b++)
        {
            scriba_id_t poc_id;
            scriba_id_create(&poc_id);
            char firstname[50];
            char secondname[50];
            char lastname[50];
            char mobile[50];
            char phone[50];
            char email[50];
            char pos[50];

            snprintf(firstname, 50, "Person %d-%d", a + 1, b + 1);
            snprintf(secondname, 50, "Secondname %d-%d", a + 1, b + 1);
            snprintf(lastname, 50, "Lastname %d-%d", a + 1, b + 1);
            snprintf(mobile, 50, "985-%d-%d", a + 1, b + 1);
            snprintf(phone, 50, "85-%d-%d", a + 1, b + 1);
            snprintf(email, 50, "85-%d-%d", a + 1, b + 1);
            snprintf(pos, 50, "testposition%d%d", a + 1, b + 1);

            scriba_addPOCWithID(poc_id, firstname, secondname, lastname, mobile,
                                phone, email, pos, company_id);
            entries++;

            for (int c = 0; c < params.proj_per_poc; c++)
            {
                scriba_id_t proj_id;
                scriba_id_create(&proj_id);
                char title[50];
                char descr[50];
                enum ScribaProjectState state;
                enum ScribaCurrency currency;
                unsigned long cost;

                snprintf(title, 50, "Project %d-%d-%d", a + 1, b + 1, c + 1);
                snprintf(descr, 50, "description of project %d-%d-%d", a + 1, b + 1, c + 1);
                switch ((c + 1) % 8)
                {
                case 1:
                    state = PROJECT_STATE_INITIAL;
                    break;
                case 2:
                    state = PROJECT_STATE_CLIENT_INFORMED;
                    break;
                case 3:
                    state = PROJECT_STATE_CLIENT_RESPONSE;
                    break;
                case 4:
                    state = PROJECT_STATE_OFFER;
                    break;
                case 5:
                    state = PROJECT_STATE_REJECTED;
                    break;
                case 6:
                    state = PROJECT_STATE_CONTRACT_SIGNED;
                    break;
                case 7:
                    state = PROJECT_STATE_EXECUTION;
                    break;
                case 0:
                    state = PROJECT_STATE_PAYMENT;
                    break;
                }

                switch ((c + 1) % 3)
                {
                case 0:
                    currency = SCRIBA_CURRENCY_RUB;
                    cost = 1000 * c;
                    break;
                case 1:
                    currency = SCRIBA_CURRENCY_USD;
                    cost = 50 * c;
                    break;
                case 2:
                    currency = SCRIBA_CURRENCY_EUR;
                    cost = 40 * c;
                    break;
                }

                scriba_addProjectWithID(proj_id, title, descr, company_id, state,
                                        currency, cost);
                entries++;

                for (int d = 0; d < params.events_per_proj; d++)
                {
                    scriba_id_t event_id;
                    scriba_id_create(&event_id);
                    char descr[50];
                    enum ScribaEventType type;
                    char outcome[50];
                    scriba_time_t timestamp = (scriba_time_t)time(NULL) + a + b + c + d;
                    enum ScribaEventState state;

                    snprintf(descr, 50, "Event %d-%d-%d-%d", a + 1, b + 1, c + 1, d + 1);
                    snprintf(outcome, 50, "Outcome of event %d-%d-%d-%d",
                             a + 1, b + 1, c + 1, d + 1);

                    switch (d % 3)
                    {
                    case 1:
                        type = EVENT_TYPE_MEETING;
                        break;
                    case 2:
                        type = EVENT_TYPE_CALL;
                        break;
                    case 0:
                        type = EVENT_TYPE_TASK;
                        break;
                    }

                    switch ((b + d) % 3)
                    {
                    case 1:
                        state = EVENT_STATE_SCHEDULED;
                        break;
                    case 2:
                        state = EVENT_STATE_COMPLETED;
                        break;
                    case 0:
                        state = EVENT_STATE_CANCELLED;
                        break;
                    }

                    scriba_addEventWithID(event_id, descr, company_id,
                                          poc_id, proj_id, type,
                                          outcome, timestamp, state);
                    entries++;
                }
            }
        }
    }
    clock_gettime(CLOCK_MONOTONIC_RAW, &generation_ts);
    printf("done\n");
    fflush(stdout);

    // export entries
    printf("Exporting data...");
    fflush(stdout);
    scriba_list_t *companies = scriba_getAllCompanies();
    scriba_list_t *people = scriba_getAllPeople();
    scriba_list_t *projects = scriba_getAllProjects();
    scriba_list_t *events = scriba_getAllEvents();
    unsigned long buflen = 0;
    void *buf = scriba_serialize(companies, events, people, projects, &buflen);

    // clean up
    scriba_cleanup();
    unlink(TEMP_DB);

    // write data to file
    int fd = open(OUTPUT_FILE, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd == -1)
    {
        perror("Failed to create output file");
        return errno;
    }
    write(fd, buf, (size_t)buflen);
    close(fd);
    clock_gettime(CLOCK_MONOTONIC_RAW, &export_ts);
    printf("done\n");
    fflush(stdout);

    long gen_time = (generation_ts.tv_sec - start_ts.tv_sec) * 1000;
    long msec_diff = (generation_ts.tv_nsec - start_ts.tv_nsec) / 1000000;
    gen_time += (msec_diff >= 0) ? msec_diff : 1000 - msec_diff;

    long exp_time = (export_ts.tv_sec - generation_ts.tv_sec) * 1000;
    msec_diff = (export_ts.tv_nsec - generation_ts.tv_nsec) / 1000000;
    exp_time += (msec_diff >= 0) ? msec_diff : 1000 - msec_diff;

    printf("Written %ld entries to %s\n", entries, OUTPUT_FILE);
    printf("Generation time: %ld ms\n", gen_time);
    printf("Export time: %ld ms\n", exp_time);

    return OK;
}
