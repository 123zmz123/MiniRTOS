#include"Svc_s.h"
int __svc(0x00) svc_service_add(int x, int y);
int __svc(0x01) svc_service_sub(int x, int y);
int __svc(0x02) svc_service_mul(int x, int y);
int __svc(0x03) svc_service_div(int x, int y);

int svc_x, svc_y, svc_z;

void Test_SVC_Func()
{
    svc_x=2;
    svc_y=7;
    svc_z=svc_service_add(svc_x,svc_y);
    svc_z=svc_service_sub(svc_x,svc_y);
    svc_z=svc_service_mul(svc_x,svc_y);
    svc_z=svc_service_div(svc_x,svc_y);

}

void SVC_Handler_C(unsigned int *svc_args)
{
   unsigned int svc_number;
   svc_number = ((char*)svc_args[6])[-2] ;
   switch (svc_number)
   {
   case 0 :
       svc_args[0] = svc_args[0] + svc_args[1];
       break;
    case 1:
        svc_args[0] = svc_args[0] - svc_args[1];
        break;
    case 2:
        svc_args[0] = svc_args[0] * svc_args[1];
        break;
    case 3:
        svc_args[0] = svc_args[0] / svc_args[1];
        break;
   default:
       break;
   }
}