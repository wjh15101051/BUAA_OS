#include "lib.h"
#include <unistd.h>
#include <mmu.h>
#include <env.h>
#include <trap.h>
#include <semaphore.h>

void syscall_putchar(char ch)
{
	msyscall(SYS_putchar, (int)ch, 0, 0, 0, 0);
}


u_int syscall_getenvid(void)
{//user_panic("syscall_getenvid\n");
	return msyscall(SYS_getenvid, 0, 0, 0, 0, 0);
}

void syscall_yield(void)
{
	msyscall(SYS_yield, 0, 0, 0, 0, 0);
}


int syscall_env_destroy(u_int envid)
{
	return msyscall(SYS_env_destroy, envid, 0, 0, 0, 0);
}
int syscall_set_pgfault_handler(u_int envid, void (*func)(void), u_int xstacktop)
{
	return msyscall(SYS_set_pgfault_handler, envid, (int)func, xstacktop, 0, 0);
}

int syscall_mem_alloc(u_int envid, u_int va, u_int perm)
{
	return msyscall(SYS_mem_alloc, envid, va, perm, 0, 0);
}

int syscall_mem_map(u_int srcid, u_int srcva, u_int dstid, u_int dstva, u_int perm)
{
	return msyscall(SYS_mem_map, srcid, srcva, dstid, dstva, perm);
}

int syscall_mem_unmap(u_int envid, u_int va)
{
	return msyscall(SYS_mem_unmap, envid, va, 0, 0, 0);
}

int syscall_set_env_status(u_int envid, u_int status)
{
	return msyscall(SYS_set_env_status, envid, status, 0, 0, 0);
}

int syscall_set_trapframe(u_int envid, struct Trapframe *tf)
{
	return msyscall(SYS_set_trapframe, envid, (int)tf, 0, 0, 0);
}

void syscall_panic(char *msg)
{
	msyscall(SYS_panic, (int)msg, 0, 0, 0, 0);
}

int syscall_ipc_can_send(u_int envid, u_int value, u_int srcva, u_int perm)
{
	return msyscall(SYS_ipc_can_send, envid, value, srcva, perm, 0);
}

void syscall_ipc_recv(u_int dstva)
{
	msyscall(SYS_ipc_recv, dstva, 0, 0, 0, 0);
}

int syscall_cgetc()
{
	return msyscall(SYS_cgetc, 0, 0, 0, 0, 0);
}
int syscall_write_dev(u_int va,u_int dev,u_int offset)
{
    return msyscall(SYS_write_dev, va, dev, offset, 0, 0);
}
int syscall_read_dev(u_int va,u_int dev,u_int offset)
{
    return msyscall(SYS_read_dev, va, dev, offset, 0, 0);
}

void syscall_pthread_finish(void) {
    msyscall(SYS_pthread_finish, 0, 0, 0, 0, 0);
}

extern struct Pthread* pthreads;

void syscall_pthread_exit(void *retval) {
    msyscall(SYS_pthread_exit, retval, 0, 0, 0, 0);
}

int syscall_pthread_cancel(pthread_t thread) {
    return msyscall(SYS_pthread_cancel, thread, 0, 0, 0, 0);
}

int syscall_pthread_join(pthread_t thread, void **retval) {
    return msyscall(SYS_pthread_join, thread, retval, 0, 0, 0);
}

int syscall_pthread_alloc() {
    return msyscall(SYS_pthread_alloc, 0, 0, 0, 0, 0);
}

int syscall_set_pth_status(pthread_t pthread, u_int status) {
    return msyscall(SYS_set_pth_status, pthread, status, 0, 0, 0);
}

u_int syscall_read_pth_status(pthread_t pthread) {
    return msyscall(SYS_read_pth_status, pthread, 0, 0, 0, 0);
}

int syscall_set_pth_retval(pthread_t pthread, u_int retval) {
    return msyscall(SYS_set_pth_retval, pthread, retval, 0, 0, 0);
}

u_int syscall_read_pth_retval(pthread_t pthread) {
    return msyscall(SYS_read_pth_retval, pthread, 0, 0, 0, 0);
}

sem_t syscall_sem_init(u_int pshared, u_int init_val) {
    return msyscall(SYS_sem_init, pshared, init_val, 0, 0, 0);
}

void syscall_sem_destroy(sem_t sem) {
    msyscall(SYS_sem_destroy, sem, 0, 0, 0, 0);
}

int syscall_sem_trywait(sem_t sem) {
    return msyscall(SYS_sem_trywait, sem, 0, 0, 0, 0);
}

void syscall_sem_wait(sem_t sem) {
    msyscall(SYS_sem_wait, sem, 0, 0, 0, 0);
}

void syscall_sem_post(sem_t sem) {
    msyscall(SYS_sem_post, sem, 0, 0, 0, 0);
}

int syscall_sem_getvalue(sem_t sem) {
    msyscall(SYS_sem_getvalue, sem, 0, 0, 0, 0);
}