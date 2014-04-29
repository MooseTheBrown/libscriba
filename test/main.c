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

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();

    ret = CU_get_error();

cleanup:
    CU_cleanup_registry();

exit:
    return ret;
}
