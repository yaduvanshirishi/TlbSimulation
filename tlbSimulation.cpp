#include <iostream>
#include <cstdio>
#include <cstdint>
#include <cmath>
#include <fstream>
#define TLB_ENTRIES 32
#define ASSOCIATIVITY 4
#define FILL_TLB 8888888888888888888
#define FRAME_FILL_TLB 234576
#define SET TLB_ENTRIES / ASSOCIATIVITY
#define PAGEOFFSET 55
#define ui uint64_t

using namespace std;
ui pageOffset;
struct node
{
    ui tag;
    unsigned int frame;
};

int turn = 0;
/*
set = TLB_ENTRIES/ASSOCIATIVITY
set = 32/4 = 8 or 3bits
*/
struct node tlb[SET][ASSOCIATIVITY];

void fillTlb(ui tagL, ui setL)
{
    ui tagTemp = tagL; //NEW EDIT
    ui frameTemp = FRAME_FILL_TLB;
    int i, j;
    unsigned int s;
    s = SET;
    for (i = 0; i < ASSOCIATIVITY; i++)
    {
        for (j = 0; j < s; j++)
        {
            tlb[setL % s][i].tag = tagTemp;
            tlb[setL % s][i].frame = frameTemp++;
            tagTemp++;
            setL++;
        }
    }
}

int getPageFrame(ui logicalAddressL)
{

    /* assuming 64 bit LA
   
   Page Size 4KB so offset = 12bits
   k-way, k = 4
   Tlb entries = 32
   so, #set = 2^5/2^2 = 2^3 = 8
  Tag = 64 - 3 - 12 = 49bits
*/
    ui p = PAGEOFFSET;
    ui pO = (ui)pow(2, p);
    pageOffset = logicalAddressL % pO;
    logicalAddressL /= pO; //PAGE BITS REMOVED

    //printf("pageOffset :%u\n", pageOffset);

    int set = TLB_ENTRIES / ASSOCIATIVITY;

    //printf("set :%d\n",set);

    int setValue = logicalAddressL % set;

    //printf("setValue :%d\n",setValue);

    logicalAddressL = logicalAddressL / set; //SET BITS REMOVED
    ui tag = logicalAddressL;                //TAG BITS REMAINED

    //tag = tag%(uint64_t) pow(2,17);

    if (turn == 0)
    {
        fillTlb(tag, setValue);
        ++turn;
    }
    int i, j, a;
    a = ASSOCIATIVITY;
    for (i = 0; i < a; i++)
    {
        if (tlb[setValue][i].tag == tag)
            return 1;
    }

    return 0;
}
int countDigits(int digits)
{
    int counts = 1;
    ui d;
    while (digits > 0)
    {
        d = (ui)pow(10, counts);
        digits = digits / d;
        ++counts;
    }
    d = d * 10; //IT WILL RETURN 10^DIGITS
    return d;
}
int main(void)
{

    int temp;
    long int count = 0, total = 0;
    fstream myfile("address.txt", ios_base::in);
    printf("Input is taken from address.txt file:\n");
    getPageFrame(FILL_TLB);
    ui address;
    while (myfile >> address)
    {
        //printf("%" PRIu64"\n" ,address);
        int x = getPageFrame(address);
        if (x == 1)
            count++;
        total++;
    }
    printf("Total Address : %ld\n", total);
    printf("Total Hits : %ld\n", count);
    printf("Hit Ratio : %f\n", (double)count / (double)total);

    /*
       Code shown Below code will give physical address = frame + offset;
        Uncomment the code if you need physical address as well
    */
    /*
    uint64_t logicalAddress = 8888888888888888888;
    uint64_t t;
    unsigned long int pageFrame = getPageFrame(logicalAddress);
    printf("%" PRIu64"\n", logicalAddress);
    printf("%" PRIu64"\n",pageFrame);
    int d = countDigits(pageFrame);
    pageFrame = pageFrame*d;
    pageFrame = pageFrame + pageOffset;
    
    if(pageFrame==0) 
      printf("\nMiss\n");
    else
      printf("Physical Address : %lu\n",pageFrame);
	*/

    return 0;
}