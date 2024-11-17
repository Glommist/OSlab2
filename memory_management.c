#include <stdlib.h>
#include <stdio.h>
#define PROCESS_NAME_LEN 32 /*进程名长度*/
#define MIN_SLICE 10 /*最小碎片的大小*/
#define DEFAULT_MEM_SIZE 1024 /*内存大小*/
#define DEFAULT_MEM_START 0 /*起始位置*/
#define MA_FF 1
#define MA_BF 2
#define MA_WF 3
#define SIZE_ENOUGH 1
#define SIZE_NOT_ENOUGH 0
int mem_size=DEFAULT_MEM_SIZE; /*内存大小*/
int ma_algorithm = MA_FF;

int pid;
struct free_block_type
{
    int size;
    int start_addr;
    struct free_block_type *next;
};
struct allocated_block
{   int pid;
    int size;
    int start_addr;
    char process_name[PROCESS_NAME_LEN];
    struct allocated_block *next;
};
struct allocated_block *allocated_block_head = NULL;
struct allocated_block *ab = NULL;
struct free_block_type *free_block;
struct free_block_type *ftb_to_ab;
int rearrange_FF();
int rearrange_WF();
int rearrange_BF();
int rearrange();
int free_mem(struct allocated_block *ab);
int allocate_mem(struct allocated_block *ab);
int dispose(struct allocated_block *free_ab);
void insert_sorted(struct free_block_type **head, struct free_block_type *node, int algorithm);
/*初始化空闲块，默认为一块，可以指定大小及起始地址*/
struct free_block_type* init_free_block(int mem_size)
{
    struct free_block_type *fb;
    fb=(struct free_block_type *)malloc(sizeof(struct free_block_type));
    if(fb==NULL)
    {
        printf("No mem\n");
        return NULL;
    }
    fb->size = mem_size;
    fb->start_addr = DEFAULT_MEM_START;
    fb->next = NULL;
    return fb;
}
/*显示菜单*/
void display_menu()
{
    printf("\n");
    printf("1 - Set memory size (default=%d)\n", DEFAULT_MEM_SIZE);
    printf("2 - Select memory allocation algorithm\n");
    printf("3 - New process \n");
    printf("4 - Terminate a process \n");
    printf("5 - Display memory usage \n");
    printf("0 - Exit\n");
}
/*设置内存的大小*/
int flag = 0;
int set_mem_size()
{
    int size;
    if(flag != 0)
    { //防止重复设置
        printf("Cannot set memory size again\n");
        return 0;
    }
    printf("Total memory size =");
    scanf("%d", &size);
    if(size>0)
    {
        mem_size = size;
        free_block->size = mem_size;
    }
    flag=1;
    return 1;
}
/* 设置当前的分配算法 */
void set_algorithm()
{
    int algorithm;
    printf("\t1 - First Fit\n");
    printf("\t2 - Best Fit \n");
    printf("\t3 - Worst Fit \n");
    scanf("%d", &algorithm);
    if(algorithm>=1 && algorithm <=3)
    ma_algorithm=algorithm;
    //按指定算法重新排列空闲区链表
    struct free_block_type *sorted = NULL;
    struct free_block_type *current = free_block;
    while (current != NULL)
    {
        struct free_block_type *next = current->next;
        current->next = NULL;
        insert_sorted(&sorted, current, MA_FF);
        current = next;
    }
    free_block = sorted;
    if(ab != NULL)
        rearrange(ma_algorithm);
}
/*按指定的算法整理内存空闲块链表*/
int rearrange(int algorithm)
{
    int outcome = SIZE_NOT_ENOUGH;
    switch(algorithm)
    {
        case MA_FF: outcome = rearrange_FF(); break;
        case MA_BF: outcome = rearrange_BF(); break;
        case MA_WF: outcome = rearrange_WF(); break;
    }
    return outcome;
}
/*按 FF 算法重新整理内存空闲块链表*/
int rearrange_FF()
{
    ftb_to_ab = free_block;
    while(ftb_to_ab!=NULL)
    {
        if(ftb_to_ab->size > ab->size)
            return SIZE_ENOUGH;
        else
        {
            ftb_to_ab = ftb_to_ab->next;
        }
    }
    return SIZE_NOT_ENOUGH;
}
/*按 BF 算法重新整理内存空闲块链表*/
int rearrange_BF()
{
    struct free_block_type *best_fit = NULL;
    ftb_to_ab = free_block;
    while(ftb_to_ab!=NULL)
    {
        if(ftb_to_ab->size > ab->size)
        {
            if(best_fit == NULL || ftb_to_ab->size < best_fit ->size)
            {
                best_fit = ftb_to_ab;
            }
        }
        ftb_to_ab = ftb_to_ab->next;
    }
    if(best_fit != NULL)
    {
        ftb_to_ab = best_fit;
        return SIZE_ENOUGH;
    }
    else
        return SIZE_NOT_ENOUGH;
}
/*按 WF 算法重新整理内存空闲块链表*/
int rearrange_WF()
{
    struct free_block_type *max_fbt = NULL;
    max_fbt = free_block;
    ftb_to_ab = free_block;
    while(ftb_to_ab != NULL)
    {
        if(ftb_to_ab->size > max_fbt->size)
        {
            if(max_fbt == NULL || ftb_to_ab->size > max_fbt->size)
            {
                max_fbt = ftb_to_ab;
            }
        }    
        ftb_to_ab = ftb_to_ab->next;
    }
    if(max_fbt != NULL)
    {
        ftb_to_ab = max_fbt;
        return SIZE_ENOUGH;
    }
    else
        return SIZE_NOT_ENOUGH;
}
/*创建新的进程，主要是获取内存的申请数量*/
int new_process()
{
    int size; int ret;
    ab=(struct allocated_block *)malloc(sizeof(struct allocated_block));
    if(!ab) exit(-5);
    ab->next = NULL;
    pid++;
    sprintf(ab->process_name, "PROCESS-%02d", pid);
    ab->pid = pid;
    printf("Memory for %s:", ab->process_name);
    scanf("%d", &size);
    if(size>0) ab->size = size;
    ret = allocate_mem(ab); 
    /* 从空闲区分配内存， ret==1 表示分配 ok*/
    /*如果此时 allocated_block_head 尚未赋值，则赋值*/
    if((ret==1) &&(allocated_block_head == NULL))
    {
        allocated_block_head=ab;
        return 1; 
    }
    /*分配成功，将该已分配块的描述插入已分配链表*/
    else if (ret==1)
    {
        ab->next=allocated_block_head;
        allocated_block_head=ab;
        return 2;
    }
    else if(ret==-1)
    { /*分配不成功*/
        printf("Allocation fail\n");
        free(ab);
        return -1;
    }
    return 3;
}
/*分配内存模块*/
int allocate_mem(struct allocated_block *ab)
{
    struct free_block_type *fbt, *pre;
    int request_size=ab->size;
    fbt = pre = free_block;
    int return_flag = SIZE_NOT_ENOUGH;
    //根据当前算法在空闲分区链表中搜索合适空闲分区进行分配，分配时注意以下情况：
    return_flag = rearrange(ma_algorithm);
    // 1. 找到可满足空闲分区且分配后剩余空间足够大，则分割
    if(return_flag == SIZE_ENOUGH && (ftb_to_ab->size - ab->size) > MIN_SLICE)
    {
        ab->start_addr = ftb_to_ab->start_addr;
        ftb_to_ab->size = ftb_to_ab->size - ab->size;
        ftb_to_ab->start_addr = ftb_to_ab->start_addr + ab->size;
        return 1;
    }
    // 2. 找到可满足空闲分区且但分配后剩余空间比较小，则一起分配
    if(return_flag == SIZE_ENOUGH && (ftb_to_ab->size - ab->size) <= MIN_SLICE)
    {
        ab->size = ftb_to_ab->size;
        ab->start_addr = ftb_to_ab->start_addr;
        if(ftb_to_ab == free_block)
        {
            free_block = free_block->next;
            free(ftb_to_ab);
        }
        else
        {
            while(fbt != ftb_to_ab)
            {
                pre = fbt;
                fbt = fbt->next;
            }
            pre->next = fbt->next;
            free(fbt);
        }
        return 1;
    }
    // 3. 找不可满足需要的空闲分区但空闲分区之和能满足需要，则采用内存紧缩技术，
    //进行空闲分区的合并，然后再分配
    if (return_flag == SIZE_NOT_ENOUGH)
    {
        struct free_block_type *sorted = NULL;
        struct free_block_type *current = free_block;
        while (current != NULL)
        {
            struct free_block_type *next = current->next;
            current->next = NULL;
            insert_sorted(&sorted, current, MA_FF);
            current = next;
        }
        free_block = sorted;
        fbt = free_block;
        int max_size = 0;
        struct free_block_type *temp_fbt = fbt;
        struct allocated_block *temp_ab = allocated_block_head;
        // 计算空闲分区的总大小
        while (temp_fbt != NULL)
        {
            max_size += temp_fbt->size;
            pre = temp_fbt;
            temp_fbt = temp_fbt->next;   
        }
        
        // 如果总空闲内存小于需要分配的内存，返回失败
        if (max_size < ab->size)
            return -1;
        else
        {
            // 合并空闲内存块
            fbt = free_block;
            while (fbt != NULL && fbt->size < ab->size)
            {
                if (fbt->next != NULL)
                {
                    // 合并当前空闲块和下一个块
                    /*
                    算法思路如下：
                    开始对free_block进行遍历：以第一个拿到的空闲块为起点
                    将后续块移动到该第一个块后面，修改相应的指针。直到块大小满足要求
                    随后进行递归调用。
                    */
                    temp_ab = allocated_block_head;
                    fbt->size += fbt->next->size;
                    while(temp_ab!=NULL)
                    {
                        if(temp_ab->start_addr > fbt->start_addr && temp_ab->start_addr < fbt->next->start_addr)
                        {
                            temp_ab->start_addr += fbt->next->size;
                        }
                        temp_ab = temp_ab->next;
                    }
                    struct free_block_type *temp = fbt->next;
                    fbt->next = fbt->next->next;
                    free(temp);
                }
            }
        }
        return allocate_mem(ab);
    }
    // 4. 在成功分配内存后，应保持空闲分区按照相应算法有序

    // 5. 分配成功则返回 1，否则返回-1
    return 1;
}
struct allocated_block *find_process(int pid)
{
    struct allocated_block *temp = allocated_block_head;
    while(temp != NULL)
    {
        if(temp->pid == pid)
        {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}
/*删除进程，归还分配的存储空间，并删除描述该进程内存分配的节点*/
void kill_process()
{
    struct allocated_block *ab_t;
    int pid;
    printf("Kill Process, pid=");
    scanf("%d", &pid);
    ab_t=find_process(pid);
    if(ab_t!=NULL)
    {
        free_mem(ab_t); /*释放 ab 所表示的分配区*/
        dispose(ab_t); /*释放 ab 数据结构节点*/
    }
}
void insert_sorted(struct free_block_type **head, struct free_block_type *node, int algorithm)
{
    struct free_block_type *current;
    if (*head == NULL || 
        (algorithm == MA_FF && (*head)->start_addr >= node->start_addr) ||
        (algorithm == MA_BF && (*head)->size >= node->size) ||
        (algorithm == MA_WF && (*head)->size <= node->size))
        {
            node->next = *head;
            *head = node;
        }
        else 
        {
            current = *head;
            while (current->next != NULL &&
               ((algorithm == MA_FF && current->next->start_addr < node->start_addr) ||
                (algorithm == MA_BF && current->next->size < node->size) ||
                (algorithm == MA_WF && current->next->size > node->size)))
                {
                    current = current->next;
                }
            node->next = current->next;
            current->next = node;
        }
}

/*将 ab 所表示的已分配区归还，并进行可能的合并*/
int free_mem(struct allocated_block *ab)
{
    int algorithm = ma_algorithm;
    struct free_block_type *fbt, *work,*temp_free_mem;
    fbt=(struct free_block_type*) malloc(sizeof(struct free_block_type));
    if(!fbt) return -1;
    // 进行可能的合并，基本策略如下
    fbt->next = NULL;
    fbt->size = ab->size;
    fbt->start_addr = ab->start_addr;
    
    // 1. 将新释放的结点插入到空闲分区队列末尾
    temp_free_mem = free_block;
    while(temp_free_mem->next != NULL)
    {
        temp_free_mem = temp_free_mem->next;
    }
    temp_free_mem->next = fbt;
    // 2. 对空闲链表按照地址有序排列
    struct free_block_type *sorted = NULL;
    struct free_block_type *current = free_block;
    while (current != NULL)
    {
        struct free_block_type *next = current->next;
        current->next = NULL;
        insert_sorted(&sorted, current, MA_FF);
        current = next;
    }
    free_block = sorted;
    // 3. 检查并合并相邻的空闲分区
    work = free_block;
    int myflag = 0;
    while (work != NULL && work->next != NULL)
    {
        if (work->start_addr + work->size == work->next->start_addr)
        {
            work->size += work->next->size;
            struct free_block_type *temp = work->next;
            work->next = work->next->next;
            free(temp);
        }
        else
        {
            work = work->next;
        }
    }
    // 4. 将空闲链表重新按照当前算法排序
    sorted = NULL;
    current = free_block;
    while (current != NULL)
    {
        struct free_block_type *next = current->next;
        current->next = NULL;
        insert_sorted(&sorted, current, ma_algorithm);
        current = next;
    }
    free_block = sorted;
    return 1;
}
/*释放 ab 数据结构节点*/
int dispose(struct allocated_block *free_ab)
{
    struct allocated_block *pre, *ab;
    if(free_ab == allocated_block_head)
    { 
        /*如果要释放第一个节点*/
        allocated_block_head = allocated_block_head->next;
        free(free_ab);
        return 1;
    }
    pre = allocated_block_head;
    ab = allocated_block_head->next;
    while(ab!=free_ab){ pre = ab; ab = ab->next;}
    pre->next = ab->next;
    free(ab);return 2;
}
/* 显示当前内存的使用情况，包括空闲区的情况和已经分配的情况 */
int display_mem_usage()
{
    struct free_block_type *fbt=free_block;
    struct allocated_block *ab=allocated_block_head;
    if(fbt==NULL) return(-1);
    printf("----------------------------------------------------------\n");
    /* 显示空闲区 */
    printf("Free Memory:\n");
    printf("%20s %20s\n", " start_addr", " size");
    while(fbt!=NULL)
    {
        printf("%20d %20d\n", fbt->start_addr, fbt->size);
        fbt=fbt->next;
    }
    /* 显示已分配区 */
    printf("\nUsed Memory:\n");
    printf("%10s %20s %10s %10s\n", "PID", "ProcessName", "start_addr", " size");
    while(ab!=NULL)
    {
        printf("%10d %20s %10d %10d\n", ab->pid, ab->process_name,
        ab->start_addr, ab->size);
        ab=ab->next;
    }
    printf("----------------------------------------------------------\n");
    return 0;
}
void do_exit()
{

}
int main()
{
    char choice; pid = 0;
    free_block = init_free_block(mem_size); //初始化空闲区
    while(1) 
    {
        display_menu(); //显示菜单
        fflush(stdin);
        choice = getchar();//获取用户输入
        
        switch(choice)
        {
            case '1': set_mem_size(); break; //设置内存大小
            case '2': set_algorithm();flag=1; break;//设置算法
            case '3': new_process(); flag=1; break;//创建新进程
            case '4': kill_process(); flag=1; break;//删除进程
            case '5': display_mem_usage(); flag=1; break; //显示内存使用
            case '0': do_exit(); exit(0); //释放链表并退出
            default: break;
        }
    }
}