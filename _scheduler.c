/*
 * @Descripttion: C语言实现多级反馈队列调度算法
 * @version: 1.0
 * @Author: HanJingKang
 * @Date: 2022-05-22 14:09:52
 * @LastEditTime: 2022-05-24 23:42:00
 */

/* 题目描述：采用三级调度策略，所有进程先按到达顺序进入一级队列，
按照时间片为2轮转一次，一个时间片内未完成的进程被依次移入二队列尾部。
当一级队列中没有进程时，开始调度二级队列，按照时间片4轮转一次，
一个时间片内未完成的进程被依次移入三队列尾部。当一级队列和二级队列中都没有进程时，
开始调度三级队列，三级队列按照FCFS调度。队列之间按抢占式优先级调度，即只有高级队
列中无进程时才能执行低级队列中的进程，且一级队列中的新来进程可抢占二级队列或者三级队列中的进程。

要求：进程的到达时间和运行时间随机生成，进程的数量不少于5个。
程序应能够输出调度过程，所有进程结束后输出甘特图，并计算响应时间、周转时间、等待时间等信息。
验收要点： 1）实验结果必须能够验证队列1中正确按照轮转（时间片2）调度
          2）实验结果必须能够验证队列2中正确按照轮转（时间片4）调度
          3）实验结果必须能够验证队列3中正确按照FCFS调度
          4）实验结果必须能够观察到高级队列新来进程抢占低级队列中正在运行的进程的情况
          5）实验结果中应输出每个时间点三个就绪队列的进程信息，以及被调度的进程。
          6）可能需要多次运行才能观察到上述实验结果，请仔细分析实验结果。

编程提示：建议程序调试阶段先将进程初始化成固定的优先级、到达时间和执行时间（调试数据），
并画出正确甘特图，然后观察实验结果验证。程序调试完成后在将上述数据随机初始化（测试数据），
将到达时间的随机数区间设置得大一点更容易观察上述结果。
 */
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>

#define PNUM 5   //进程的数量
#define TIMER 10 //定时器,最长CPU区间时间
#define SLICE 2  //一级序列的时间片
#define SLICE2 4 //二级序列的时间片

typedef struct node
{
    int pid;     //进程号
    int arrival; //到达时间
    int burst;   //区间大小
    int Allrest; //总剩余时间
    int L1rest;  //第一级剩余时间
    int L2rest;  //第二级剩余时间
    char state;  //进程状态,'N'新建,'R'运行,'W'等待CPU(就绪),'T'终止
    int nowin;   //当前该进程所在的多级队列
    struct node *next;
} PCB;

int gantt[TIMER * PNUM] = {0}; //用一个gantt数组记录调度过程,每个时刻调度的进程号
PCB *job;                      //所有作业的序列,带头节点(为简化编程)
PCB *run = NULL;               //正在运行中的进程,不带头结点
PCB *finish = NULL;            //已经结束的程序,不带头结点
PCB *firstQ = NULL;            //第一级队列
PCB *secondQ = NULL;           //第二级队列
PCB *thirdQ = NULL;            //第三级队列
PCB *FQ_tail = NULL;           //第一级队列的尾节点
PCB *SQ_tail = NULL;           //第二级队列的尾节点
PCB *TQ_tail = NULL;           //第三级队列的尾节点

int timenow = 0;
PCB *init(int pid, int arrival, int burst, int Allrest, char state)
{
    PCB *p;
    p = (PCB *)malloc(sizeof(PCB)); //生成新节点(新进程)
    p->pid = pid;
    p->arrival = arrival; //随机生成到达时刻0-9,(预计到达就绪队列的时间)
    p->burst = burst;     //随机生成CPU区间时间:1~10;(估计运行时间)
    p->Allrest = burst;
    p->state = state; //初始化进程的状态为'新建'
    p->next = NULL;
    p->L1rest = SLICE;
    p->L2rest = SLICE2;
    p->nowin = 1;
    return p;
}
void InitialJob()
{
    int i = 0;
    PCB *p, *tail;
    job = (PCB *)malloc(sizeof(PCB)); //生成头节点,其它域无意义
    job->next = NULL;
    tail = job;
    /* p = init(1, 1, 4, 4, 'N');
    tail->next = p;
    tail = p;
    p = init(2, 2, 2, 2, 'N');
    tail->next = p;
    tail = p;
    p = init(3, 3, 3, 3, 'N');
    tail->next = p;
    tail = p; */
    for (i = 0; i < PNUM; i++)
    {
        p = (PCB *)malloc(sizeof(PCB)); //生成新节点(新进程)
        p->pid = i + 1;
        p->arrival = rand() % TIMER;   //随机生成到达时刻0-9,(预计到达就绪队列的时间)
        p->burst = rand() % TIMER + 1; //随机生成CPU区间时间:1~10;(估计运行时间)
        p->Allrest = p->burst;
        p->state = 'N'; //初始化进程的状态为'新建'
        p->next = NULL;
        p->L1rest = SLICE;
        p->L2rest = SLICE2;
        p->nowin = 1;
        tail->next = p;
        tail = p; //带头结点
    }
}
void DisplayPCB(PCB *pcb) //显示队列
{
    struct node *p = pcb;
    if (pcb == NULL)
    {
        printf("XXXXXX\n");
        return;
    }
    printf("进程号 到达时刻 区间大小 剩余时间 进程状态\n");
    do
    {
        printf("P%-3d\t", p->pid);
        printf("%3d\t", p->arrival);
        printf("%4d\t", p->burst);
        printf("%4d\t", p->Allrest);
        printf("%6c\t", p->state);
        printf("\n");
        p = p->next;
    } while (p != NULL);
}
void DisplayGantt() //显示甘特数组
{
    int i = 0;
    for (i = 0; i < timenow; i++)
    {
        if (gantt[i] == 0)
            printf("空闲,");
        else
            printf("P%d,", gantt[i]);
    }
    printf("\n");
}
void DisplayTime() //显示周转时间t,等待时间w和响应时间r
{
    int t = 0, w = 0, r = 0;
    float t_avg = 0, w_avg = 0, r_avg = 0;
    int i, j;
    PCB *p; //用p遍历finish队列,查找进程Pi的到达时间,调用该函数时所有进程都已放入finish队列
    if (finish == NULL)
    {
        return;
    }
    printf("进程号    周转时间    等待时间    响应时间\n");
    for (i = 1; i <= PNUM; i++)
    {
        p = finish;
        while (p->pid != i)
            p = p->next;
        j = 0;
        while (gantt[j] != i)
            j++; //遍历甘特数组,求进程Pi的响应时间
        r = j;   //响应时刻
        t = j + 1;
        for (j = r + 1; j < timenow; j++) //继续遍历,求周转时间
        {
            if (i == gantt[j])
                t = j + 1;
        }                         //结束时刻
        r = r - p->arrival;       //响应时间=响应时刻-到达时刻
        t = t - p->arrival;       //周转时间=结束时刻-到达时刻
        w = t - p->burst;         //等待时间=周转时间-运行时间
        r_avg += (float)r / PNUM; //平均响应时间
        w_avg += (float)w / PNUM; //平均等待时间
        t_avg += (float)t / PNUM; //平均周转时间

        printf("P%d       %4d       %4d       %4d\n", i, t, w, r);
    }
    printf("平均周转时间:%.2f,平均等待时间%.2f,平均响应时间%.2f\n", t_avg, w_avg, r_avg);
}
void DisplayEachQueue()
{
    printf("一级队列:\n");
    DisplayPCB(firstQ);
    printf("二级队列:\n");
    DisplayPCB(secondQ);
    printf("三级队列:\n");
    DisplayPCB(thirdQ);
}
void ReadyQueue(int t) //t:当前时刻
{
    struct node *jpre, *jcur, *rpre, *rcur;
    if (job->next == NULL)
    {
        printf("作业队列为空!\n");
        DisplayEachQueue();
        return;
    }
    jpre = job;
    jcur = job->next;
    while (jcur != NULL) //遍历作业序列中选择已到达进程,将其从作业队列移入就绪队列,直到作业队列为空
    {
        if (jcur->arrival <= t) //如果当前时刻进程已经到达,则将其插入到就绪队列的合适位置
        {
            printf("P%d到达.\n", jcur->pid);
            jpre->next = jcur->next; //将jcur从作业队列移除
            jcur->state = 'W';       //将进程状态设置为就绪
            if (firstQ == NULL)      //就绪队列为空
            {
                jcur->next = NULL;
                firstQ = jcur;
                FQ_tail = firstQ;
            }
            else //就绪队列不为空,遍历就绪队列,将jcur插入到合适位置
            {
                rpre = firstQ;
                rcur = firstQ;
                while ((rcur != NULL) && (jcur->arrival >= rcur->arrival))
                {
                    rpre = rcur;
                    rcur = rcur->next;
                }

                if (rcur == NULL) // 插入点在就绪队列尾部
                {
                    jcur->next = NULL;
                    rpre->next = jcur;
                    FQ_tail = jcur;
                }
                else if (rcur == firstQ) //插入点在头部
                {
                    jcur->next = rcur;
                    firstQ = jcur;
                }
                else //插入到rpre和rcur之间
                {
                    jcur->next = rcur;
                    rpre->next = jcur;
                }
            }
            jcur = jpre->next; //下一个作业
        }
        else //当前作业未达到
        {
            jpre = jcur;
            jcur = jcur->next;
        } //下一个作业
    }
    printf("\n作业队列:\n");
    DisplayPCB(job->next);
    DisplayEachQueue();
}
void FreshQueue(PCB *job, int i) //转移进程
{
    struct node *jpre, *jcur, *rpre, *rcur, *temp_tail;
    PCB *nextQ;
    if (i == 2)
    {
        nextQ = secondQ;
    }
    if (i == 3)
    {
        nextQ = thirdQ;
    }
    printf("P%d移入下一级.\n", job->pid);
    job->state = 'W';  //将进程状态设置为就绪
    if (nextQ == NULL) //就绪队列为空
    {
        job->next = NULL;
        nextQ = job;
        temp_tail = nextQ;
    }
    else //就绪队列不为空,将jcur插入到下级队列尾部
    {
        rpre = nextQ;
        rcur = nextQ;
        while ((rcur != NULL))
        {
            rpre = rcur;
            rcur = rcur->next;
        }
        job->next = NULL;
        rpre->next = job;
        temp_tail = job;
    }
    DisplayEachQueue();
}
bool checkQueue(int queueNum) //检查是否为空
{
    PCB *p;
    switch (queueNum)
    {
    case 1:
        p = firstQ;
        break;
    case 2:
        p = secondQ;
        break;
    case 3:
        p = thirdQ;
        break;
    default:
        break;
    }
    while (p)
    {
        if (p != NULL)
        {
            return false;
        }
        p = p->next;
    }
    return true;
}
bool checkFinish(PCB *job)
{
    if (job->Allrest == 0)
    {
        struct node *rpre, *rcur;
        switch (job->nowin)
        {
        case 1:
            rpre = firstQ;
            rcur = firstQ;
            while ((rcur != NULL))
            {
                if (rcur == job)
                {
                    if (rcur == rpre)
                    {
                        firstQ = NULL;
                        break;
                    }
                    rpre->next = rcur->next;
                    break;
                }
                rpre = rcur;
                rcur = rcur->next;
            }
            printf("\n进程P%d在队列一结束", job->pid);
            break;
        case 2:
            rpre = secondQ;
            rcur = secondQ;
            while ((rcur != NULL))
            {
                if (rcur == job)
                {
                    if (rcur == rpre)
                    {
                        secondQ = NULL;
                        break;
                    }
                    rpre->next = rcur->next;
                    break;
                }
                rpre = rcur;
                rcur = rcur->next;
            }
            printf("\n进程P%d在队列二结束", job->pid);
            break;
        case 3:
            rpre = thirdQ;
            rcur = thirdQ;
            while ((rcur != NULL))
            {
                if (rcur == job)
                {
                    if (rcur == rpre)
                    {
                        thirdQ = NULL;
                        break;
                    }
                    rpre->next = rcur->next;
                    break;
                }
                rpre = rcur;
                rcur = rcur->next;
            }
            printf("\n进程P%d在队列三结束", job->pid);
            break;

        default:
            break;
        }
        gantt[timenow] = job->pid; //记录当前时刻调度进程的ID号
        job->state = 'T';
        job->next = finish; //新完成的节点插入到finish的头结点,简单一点
        finish = job;
        job = NULL;
        printf("\n结束进程队列:\n");
        DisplayPCB(finish);
        return true;
    }
    else
    {
        return false;
    }
}

void runAjob(PCB *job)
{
    job->state = 'R';
    bool to2 = false;
    bool to3 = false;
    switch (job->nowin)
    {
    case 1:
        if (job->L1rest >= 1)
        {
            job->L1rest--;
            job->Allrest--;
            if (checkFinish(job) == true)
            {
                run = NULL;
                return;
            }
            if (job->L1rest == 0)
            {
                job->nowin = 2;
                if (secondQ == NULL)
                {
                    job->next = NULL;
                    secondQ = job;
                    SQ_tail = secondQ;
                }
                else
                {
                    struct node *rpre, *rcur;
                    rpre = secondQ;
                    rcur = secondQ;
                    while (rcur != NULL) //遍历到尾部
                    {
                        rpre = rcur;
                        rcur = rcur->next;
                    }
                    if (rcur == NULL) // 插入点在该级队列尾部
                    {
                        job->next = NULL;
                        rpre->next = job;
                        SQ_tail = job;
                    }
                }
                to2 = true;

                job->state = 'w';
            }

            break;
        }
    case 2:
        if (job->L2rest >= 1)
        {
            job->L2rest--;
            job->Allrest--;
            if (checkFinish(job) == true)
            {
                run = NULL;
                return;
            }
            if (job->L2rest == 0)
            {
                job->nowin = 3;
                if (thirdQ == NULL)
                {
                    job->next = NULL;
                    thirdQ = job;
                    TQ_tail = thirdQ;
                }
                else
                {
                    struct node *rpre, *rcur;
                    rpre = thirdQ;
                    rcur = thirdQ;
                    while (rcur != NULL) //遍历到尾部
                    {
                        rpre = rcur;
                        rcur = rcur->next;
                    }
                    if (rcur == NULL) // 插入点在该级队列尾部
                    {
                        job->next = NULL;
                        rpre->next = job;
                        TQ_tail = job;
                    }
                }
                to3 = true;

                job->state = 'w';
            }

            break;
        }
    case 3:
        if (job->Allrest >= 1)
        {
            job->Allrest--;
            if (checkFinish(job) == true)
            {
                run = NULL;
                return;
            }
        }
        break;
    default:
        break;
    }
    printf("\nP%d正在运行.......\n", job->pid);
    printf("运行进程:\n");
    DisplayPCB(job);
    gantt[timenow] = job->pid; //记录当前时刻调度进程的ID号
    if (to2)
    {
        printf("\nP%d在第一级的时间片用完，现转移到二级队列!\n", job->pid);
        to2 = false;
        run=NULL;
    }
    else if (to3)
    {
        printf("\nP%d在第二级的时间片用完，现转移到三级队列!\n", job->pid);
        to3 = false;
        run=NULL;
    }
    

}
void MFQS()
{
    while (true)
    {
        printf("\n当前时刻:%d\n", timenow);
        ReadyQueue(timenow); //刷新各级就绪队列
        if (job->next == NULL && firstQ == NULL && secondQ == NULL && thirdQ == NULL && run == NULL)
            break;                                                              //没有进程,结束循环
        if (firstQ != NULL || secondQ != NULL || thirdQ != NULL || run != NULL) //有进程处于就绪或者运行状态
        {
            if (run == NULL) //若CPU空闲
            {
                if (checkQueue(1) == false)
                {
                    run = firstQ; //将CPU分配给一级队列的第一个进程
                    firstQ = firstQ->next;
                    run->next = NULL;
                    printf("\n第1级的进程P%d被调度程序分派CPU!\n", run->pid);
                }
                else if (checkQueue(2) == false)
                {
                    run = secondQ; //将CPU分配给二级队列的第一个进程
                    secondQ = secondQ->next;
                    run->next = NULL;
                    printf("\n第2级的进程P%d被调度程序分派CPU!\n", run->pid);
                }
                else if (checkQueue(3) == false)
                {
                    run = thirdQ; //将CPU分配给三级队列的第一个进程
                    thirdQ = thirdQ->next;
                    run->next = NULL;
                    printf("\n第3级的进程P%d被调度程序分派CPU!\n", run->pid);
                }
                runAjob(run);
            }
            else ///抢占式
            {
                switch (run->nowin)
                {
                case 2:
                    if (checkQueue(1) == false)
                    {
                        run->state = 'W';
                        printf("\n第1级的进程P%d抢占了第2级的进程P%d,P%d返回第二级队列!\n", firstQ->pid, run->pid);
                        run->next=secondQ;
                        secondQ=run;
                        run = firstQ; //将CPU分配给一级队列的第一个进程
                        firstQ = firstQ->next;
                        run->next = NULL;
                    }
                    break;
                case 3:
                    if (checkQueue(1) == false)
                    {
                        run->state = 'W';
                        printf("\n第1级的进程P%d抢占了第3级的进程P%d,P%d返回第三级队列!\n", firstQ->pid, run->pid, run->pid);
                        run->next=thirdQ;
                        thirdQ=run;
                        run = firstQ; //将CPU分配给一级队列的第一个进程
                        firstQ = firstQ->next;
                        run->next = NULL;
                    }
                    else if (checkQueue(2) == false)
                    {
                        run->state = 'W';
                        printf("\n第2级的进程P%d抢占了第3级的进程P%d!,P%d返回第三级队列!\n", secondQ->pid, run->pid, run->pid);
                        run->next=thirdQ;
                        thirdQ=run;
                        run = secondQ; //将CPU分配给一级队列的第一个进程
                        secondQ = secondQ->next;
                        run->next = NULL;
                    }
                default:
                    break;
                }
                runAjob(run);
            }
        }
        timenow++; //下一时刻继续扫描作业队列
    }
}

int main()
{
    srand((int)time(NULL)); //随机数种子
    InitialJob();
    DisplayPCB(job->next);
    MFQS();
    printf("全部进程运行结束\n");
    DisplayGantt();
    DisplayTime();
}