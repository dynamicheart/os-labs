// Fork a binary tree of processes and display their structure.

#include <inc/lib.h>

#define DEPTH 3

void forktrees(const char *cur);

void
forkchilds(const char *cur, char branch)
{
	char nxt[DEPTH+1];

	if (strlen(cur) >= DEPTH)
		return;

	snprintf(nxt, DEPTH+1, "%s%c", cur, branch);
	if (sfork() == 0) {
		forktrees(nxt);
		exit();
	}
}

void
forktrees(const char *cur)
{
	cprintf("%04x: I am '%s'\n", sys_getenvid(), cur);

	forkchilds(cur, '0');
	forkchilds(cur, '1');
}

void
umain(int argc, char **argv)
{
	forktrees("");
}

