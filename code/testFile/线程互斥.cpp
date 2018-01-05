#include<iostream>
#include <ctime>
#define HAVE_STRUCT_TIMESPEC
#include<pthread.h>
using namespace std;

pthread_t t1;           //pthread_t����t1�����ڻ�ȡ�߳�1��ID
pthread_t t2;           //pthread_t����t2�����ڻ�ȡ�߳�2��ID
char Share[10];         //������Դ��
pthread_mutex_t work_mutex;                    //����������work_mutex

void* My_thread_1(void* args){
	while(1){
	   char *p=Share;
	   pthread_mutex_lock(&work_mutex);      //����
       int n = 10;
       int i = 0;
       while (i++ < n) {
            Sleep(1000);
            cout << "P1" << endl;
       }
	   pthread_mutex_unlock(&work_mutex);   //����
	   //Sleep(100)            //���û�����ʱҲȥ��ע�ͣ�Ϊ���̵����ṩʱ��
    }
}
void* My_thread_2(void* args){
	while(1){
	   char *p=Share;
	   pthread_mutex_lock(&work_mutex);     //����
       int n = 10;
       int i = 0;
       while (i++ < n) {
            Sleep(1000);
            cout << "P2" << endl;
       }
	   pthread_mutex_unlock(&work_mutex);   //����
	   //Sleep(100)            //���û�����ʱҲȥ��ע�ͣ�Ϊ���̵����ṩʱ��
    }
}
int main(){
	   pthread_mutex_init(&work_mutex, NULL);   //��ʼ��������
       pthread_create(&t1,NULL,My_thread_1,NULL);
	   pthread_create(&t2,NULL,My_thread_2,NULL);
       pthread_exit(NULL);
	   pthread_mutex_destroy(&work_mutex);      //���ٻ�����
       return 0;
}
