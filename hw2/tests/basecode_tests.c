#include <stdio.h>

#include <criterion/criterion.h>
#include <criterion/logging.h>

#include "test_common.h"

#define STANDARD_LIMITS "ulimit -t 10; ulimit -f 2000"

/*
 * Compare the binary output from the program being tested with
 * reference output.
 */
void assert_binfile_matches(char *name)
{
	char cmd[500];
	sprintf(cmd,
		"cmp %s/%s.bin %s/%s.bin",
		test_log_outfile, name, TEST_REF_DIR, name);
	int err = system(cmd);
	cr_assert_eq(err, 0,
		     "The output was not what was expected (cmp exited with "
		     "status %d).\n",
		     WEXITSTATUS(err));
}

Test(basecode_suite, no_arguments) {
    char *name = "no_arguments";
    putenv("ASM_INPUT_PATH=tests/rsrc/");
    sprintf(program_options, "%s", "");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_expected_status(EXIT_FAILURE, err);
    assert_outfile_matches(name, NULL);
}

Test(basecode_suite, nontrivial_path) {
    char *name = "nontrivial_path";
    putenv("ASM_INPUT_PATH=foo/:bar/:tests/rsrc/");
    sprintf(program_options, "%s", "-O test_output/ empty.16");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_expected_status(EXIT_SUCCESS, err);
    assert_outfile_matches(name, NULL);
}

Test(basecode_suite, asm_basic) {
    char *name = "asm_basic";
    putenv("ASM_INPUT_PATH=tests/rsrc/");
    sprintf(program_options, "%s", "-O test_output/ moo.16");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_expected_status(EXIT_SUCCESS, err);
    assert_outfile_matches(name, NULL);
    assert_binfile_matches("moo");
}

Test(basecode_suite, empty_input) {
    char *name = "empty_input";
    putenv("ASM_INPUT_PATH=tests/rsrc/");
    sprintf(program_options, "%s", "-O test_output/ empty.16");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_expected_status(EXIT_SUCCESS, err);
    assert_outfile_matches(name, NULL);
    assert_binfile_matches("empty");
}

/*
 * Run the program with default options on a non-empty input file
 * and use valgrind to check for uninitialized values.
 */
Test(basecode_suite, valgrind_uninitialized) {
    char *name = "valgrind_uninitialized";
    putenv("ASM_INPUT_PATH=tests/rsrc/");
    sprintf(program_options, "%s", "-O test_output/ moo.16");
    int err = run_using_system(name, "", "valgrind --leak-check=full --undef-value-errors=no --error-exitcode=37", STANDARD_LIMITS);
    assert_no_valgrind_errors(err);
    assert_normal_exit(err);
}
