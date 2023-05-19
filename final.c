
#include "myapi.h"

void main()
{
    Msg msg;
    int i;
    init(&msg);
    while (1)
    {
        printf("do you want to lock 1 for yes 2 for Exit!\n");
        scanf("%d", &i);
        if (i == 1)
        {
            lock(&msg);
            printf("Want to come out of Critical Section press 1 \n");
            scanf("%d", &i);
            unlock(&msg);
        }
        else if (i == 2)
        {
            break;
        }
        else
        {
            continue;
        }
    }
}
