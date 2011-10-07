#include <string.h>
#include <unistd.h>

#include <libcapsicum.h>

#include "ctest.h"
#include "tests.h"

int
test_fdlist()
{
	int success = PASSED;
	const char *subsystem = "org.freebsd.libcapsicum.test";
	const char *classname = "File";
	struct lc_fdlist *fdlistp;
	int fd;

	CHECK(lc_fdlist_global() == 0);

	fdlistp = lc_fdlist_new();
	CHECK(fdlistp != 0);

	// What we want is to be able to install an fdlist as the global one
	// without mucking about with shared memory segments ourselves.

	// Create a temporary file to put in the FD list.
	char tmpfilename[80];
	strncpy(tmpfilename, "/tmp/libcapsicum.test.XXXX", sizeof(tmpfilename));
	REQUIRE(fd = mkstemp(tmpfilename));

	// Ensure that lc_fdlist_add() works.
	CHECK_SYSCALL_SUCCEEDS(lc_fdlist_add,
	    fdlistp, subsystem, classname, "raw", fd);

	char *relative_name;
	int found;
	REQUIRE(found = lc_fdlist_find(
				       fdlistp, subsystem, classname, "raw", (const char **)&relative_name));

	CHECK(found == fd);
	CHECK(strnlen(relative_name, 1) == 0);

	// Check lc_fdlist_addcap().
	cap_rights_t rights = CAP_READ | CAP_SEEK;
	CHECK_SYSCALL_SUCCEEDS(lc_fdlist_addcap,
	    fdlistp, subsystem, classname, "raw_cap", fd, rights);

	REQUIRE(found = lc_fdlist_find(
				       fdlistp, subsystem, classname, "raw_cap", (const char **) &relative_name));

	cap_rights_t rights_out;
	CHECK_SYSCALL_SUCCEEDS(cap_getrights, found, &rights_out);
	CHECK(rights_out == rights);

	return success;
}

