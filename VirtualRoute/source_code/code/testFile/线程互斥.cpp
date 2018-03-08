#include<iostream>
#include <ctime>
#define HAVE_STRUCT_TIMESPEC
#include<pthread.h>
using namespace std;

pthread_t t1;           //pthread_t变量t1，用于获取线程1的ID
pthread_t t2;           //pthread_t变量t2，用于获取线程2的ID
char Share[10];         //共享资源区
pthread_mutex_t work_mutex;                    //声明互斥量work_mutex

void* My_thread_1(void* args){
	while(1){
	   char *p=Share;
	   pthread_mutex_lock(&work_mutex);      //加锁
       int n = 10;
       int i = 0;
       while (i++ < n) {
            Sleep(1000);
            cout << "P1" << endl;
       }
	   pthread_mutex_unlock(&work_mutex);   //解锁
	   //Sleep(100)            //启用互斥量时也去除注释，为进程调度提供时间
    }
}
void* My_thread_2(void* args){
	while(1){
	   char *p=Share;
	   pthread_mutex_lock(&work_mutex);     //加锁
       int n = 10;
       int i = 0;
       while (i++ < n) {
            Sleep(1000);
            cout << "P2" << endl;
       }
	   pthread_mutex_unlock(&work_mutex);   //解锁
	   //Sleep(100)            //启用互斥量时也去除注释，为进程调度提供时间
    }
}
int main(){
	   pthread_mutex_init(&work_mutex, NULL);   //初始化互斥量
       pthread_create(&t1,NULL,My_thread_1,NULL);
	   pthread_create(&t2,NULL,My_thread_2,NULL);
       pthread_exit(NULL);
	   pthread_mutex_destroy(&work_mutex);      //销毁互斥量
       return 0;
}
