#ifndef __SVC_S_H
#define __SVC_S_H

int __svc(0x00) svc_service_add(int , int );
int __svc(0x01) svc_service_sub(int , int );
int __svc(0x02) svc_service_mul(int , int );
int __svc(0x03) svc_service_div(int , int );
void SVC_Handler_C(unsigned int *);
void Test_SVC_Func();

#endif 