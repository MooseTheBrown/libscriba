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

#ifndef SCRIBA_COMMON_TEST_H
#define SCRIBA_COMMON_TEST_H

void test_company();
void test_poc();
void test_project();
void test_project_time();
void test_event();
void test_create_with_id();
void test_company_search();
void test_ru_company_search();
void test_event_search();
void test_ru_event_search();
void test_poc_search();
void test_ru_poc_search();
void test_project_search();
void test_ru_project_search();
void clean_local_db();

#endif // SCRIBA_COMMON_TEST_H
