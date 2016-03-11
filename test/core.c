#include <linux/version.h>
#include <sys/utsname.h>
#include <stdlib.h>
#include <stdio.h>
#include <test.h>

#define KVER_STRLEN 20

struct ndctl_test {
	unsigned int kver;
	int attempt;
	int skip;
};

static unsigned int get_system_kver(void)
{
	struct utsname utsname;
	int a, b, c;

	uname(&utsname);

	sscanf(utsname.release, "%d.%d.%d", &a, &b, &c);

	return KERNEL_VERSION(a,b,c);
}

struct ndctl_test *ndctl_test_new(unsigned int kver)
{
	struct ndctl_test *test = calloc(1, sizeof(*test));

	if (!test)
		return NULL;

	if (!kver)
		test->kver = get_system_kver();
	else
		test->kver = kver;

	return test;
}

int ndctl_test_result(struct ndctl_test *test, int rc)
{
	if (ndctl_test_get_skipped(test))
		fprintf(stderr, "attempted: %d skipped: %d\n",
				ndctl_test_get_attempted(test),
				ndctl_test_get_skipped(test));
	if (rc && rc != 77)
		return rc;
	if (ndctl_test_get_skipped(test) >= ndctl_test_get_attempted(test))
		return 77;
	/* return success if no failures and at least one test not skipped */
	return 0;
}

static char *kver_str(char *buf, unsigned int kver)
{
	snprintf(buf, KVER_STRLEN, "%d.%d.%d",  (kver >> 16) & 0xffff,
			(kver >> 8) & 0xff, kver & 0xff);
	return buf;
}

int __ndctl_test_attempt(struct ndctl_test *test, unsigned int kver,
		const char *caller, int line)
{
	char requires[KVER_STRLEN], current[KVER_STRLEN];

	test->attempt++;
	if (kver <= test->kver)
		return 1;
	fprintf(stderr, "%s: skip %s:%d requires: %s current: %s\n",
			__func__, caller, line, kver_str(requires, kver),
			kver_str(current, test->kver));
	test->skip++;
	return 0;
}

void __ndctl_test_skip(struct ndctl_test *test, const char *caller, int line)
{
	test->skip++;
	test->attempt = test->skip;
	fprintf(stderr, "%s: explicit skip %s:%d\n", __func__, caller, line);
}

int ndctl_test_get_attempted(struct ndctl_test *test)
{
	return test->attempt;
}

int ndctl_test_get_skipped(struct ndctl_test *test)
{
	return test->skip;
}
