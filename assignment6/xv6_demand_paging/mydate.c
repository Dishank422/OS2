#include "types.h"
#include "user.h"
#include "date.h"

int main(void)
{
    struct rtcdate r;
    int a = mydate(&r);
    if(a<0)
    {
        printf(2, "exec failed: mydate");
        exit();
    }
    printf(2, "Year: %d\nMonth: %d\nDate: %d\nHour: %d\nMinute: %d\nSecond: %d\n", r.year, r.month, r.day, r.hour, r.minute, r.second);
    exit();
}

