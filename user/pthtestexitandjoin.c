#include "lib.h"

int a[2] = {15,22};

void *add(void * a) {
    writef("%%%%%%%% a : %08x\n", a);
    int i;
    for (i = 0; i < 8; ++i) writef("%d", (int) (*((char *) (a + i))));
    writef("\n");
    int * x = (int *) a;
    writef("add %d %d\n", x[0], x[1]);
    int res = x[0] + x[1];
    syscall_pthread_exit((void *) res);
}

void umain() {
    pthread_t thread;
    int iret;
    writef("%%%%%%%% a : %08x\n", a);
    iret = syscall_pthread_create(&thread, NULL, add, (void *) a);
    int ret_val;
    syscall_pthread_join(thread, &ret_val);
    writef("return val : %d\n", ret_val);
    return;
}