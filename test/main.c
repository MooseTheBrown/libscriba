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

#include "frontend_test.h"
#include "sqlite_backend_test.h"
#include <CUnit/Basic.h>
#include <stdio.h>

int main()
{
    CU_pSuite frontend_test_suite= NULL;
    CU_pSuite sqlite_backend_test_suite= NULL;
    int ret = 0;

    if (CUE_SUCCESS != CU_initialize_registry())
    {
        ret = CU_get_error();
        goto exit;
    }

    /* Frontend test suite */
    frontend_test_suite = CU_add_suite(FRONTEND_TEST_NAME,
                                       frontend_test_init,
                                       frontend_test_cleanup);
    if (frontend_test_suite == NULL)
    {
        ret = CU_get_error();
        goto cleanup;
    }

    CU_add_test(frontend_test_suite, "Frontend company test", test_company);
    CU_add_test(frontend_test_suite, "Frontend POC test", test_poc);
    CU_add_test(frontend_test_suite, "Frontend project test", test_project);
    CU_add_test(frontend_test_suite, "Frontend event test", test_event);
    CU_add_test(frontend_test_suite, "Frontend create with ID test", test_create_with_id);

    /* SQLite backend test suite */
    sqlite_backend_test_suite = CU_add_suite(SQLITE_BACKEND_TEST_NAME,
                                             sqlite_backend_test_init,
                                             sqlite_backend_test_cleanup);
    if (sqlite_backend_test_suite == NULL)
    {
        ret = CU_get_error();
        goto cleanup;
    }

    CU_add_test(sqlite_backend_test_suite, "SQLite backend company test", test_company);
    CU_add_test(sqlite_backend_test_suite, "SQLite backend POC test", test_poc);
    CU_add_test(sqlite_backend_test_suite, "SQLite backend project test", test_project);
    CU_add_test(sqlite_backend_test_suite, "SQLite backend event test", test_event);
    CU_add_test(sqlite_backend_test_suite,
                "SQLite backend create with ID test",
                test_create_with_id);

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();

    ret = CU_get_error();

cleanup:
    CU_cleanup_registry();

exit:
    return ret;
}
