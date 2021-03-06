/*
 * Copyright (C) 2014 Mikhail Sapozhnikov
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

#ifndef SCRIBA_SERIALIZER_TEST_H
#define SCRIBA_SERIALIZER_TEST_H

#define SERIALIZER_TEST_NAME "Serializer test"

int serializer_test_init();
int serializer_test_cleanup();
void test_serializer();
void test_serializer_ru();
void test_serializer_remote_override();
void test_serializer_local_override();

#endif // SCRIBA_SERIALIZER_TEST_H
