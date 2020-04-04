   /* if 64 bit LA
   
   Page Size 4KB so offset = 12bits
   k-way, k = 4
   Tlb entries = 32
   so, #set = 2^5/2^2 = 2^3 = 8
  Tag = 64 - 3 - 12 = 49bits
*/  

#include <iostream>
#include <cstdio>
#include <cstdint>
#include <cmath>
#include <fstream>
#include <cstdlib>
#include <assert.h>
#define LADDR 64 //Logical Address bits
#define ENTRIES 64
#define ASSOCIATIVITY 4
#define PAGEOFFSET 30
#define INPUT_FILE "address.txt"
#define FRAME 20 //number of bits for frame
#define ui uint64_t

using namespace std;

struct node
{
    ui tag;
    unsigned int frame;
};


int setGB = 0;
int setG = 0;
int k_wayG = ASSOCIATIVITY;
int turnG = 0;
ui pageOffsetG = 0;
ui pageValueG = 0;
ui tagG = 0;
unsigned int frame = FRAME;

/*
 --------------------------------------------
|    tag    |   set offset |   page offset   |
 --------------------------------------------

*/

int calculateBits(int lAddr)
{
    pageOffsetG = PAGEOFFSET;
    if(ASSOCIATIVITY==0)
        return -1;
    
    setG = ENTRIES/ASSOCIATIVITY;
    
    setGB = log2(setG);
    tagG = lAddr - setGB - pageOffsetG;
    //cout<<setGB<<" "<<setG<<" "<<lAddr<<" "<<tagG<<" "<<pageOffsetG<<endl;
}

void updateTlb(ui tagFT, int setV, struct node *tlbFT) // THis function is used when miss then that values is added in tlb
{
    int i,j;
    for(i=0;i<k_wayG;i++)
    {
        if((tlbFT+(setV-1)*k_wayG + i)->tag != tagFT) 
        (tlbFT+(setV-1)*k_wayG + i)->tag = tagFT;
    }
}

void fillTlb(struct node *tlbFT) // This function is used to fill just random values
{
    unsigned int s;
    int i,j;
    s = setG;
    for (i = 0; i < s; i++)
    {
        for (j = 0; j < ASSOCIATIVITY; j++)
        {
            (tlbFT + i*ASSOCIATIVITY + j)->tag = rand();
            (tlbFT + i*ASSOCIATIVITY + j)->frame = rand();
        }
    }
}


int getPageFrame(struct node *tlbGPF, ui logicalAddressL)
{

    ui pageSize = (ui)pow(2, pageOffsetG);
    ui pageValueL = logicalAddressL % pageSize;
    logicalAddressL = logicalAddressL/ pageSize; //PAGE BITS REMOVED

    int setValue = logicalAddressL % setG;

    //printf("setValue :%d\n",setValue);

    logicalAddressL = logicalAddressL / setG;
    ui tagSize = (ui)pow(2,tagG); 
    ui tagValue = logicalAddressL % tagSize ;                //TAG BITS REMAINED

    int i, j, a;
    a = ASSOCIATIVITY;

    for(j=0;j<k_wayG;j++)
    {
            
        if((tlbGPF+(setValue-1)*k_wayG+j)->tag == tagValue) 
        {
            cout<<(tlbGPF+(setValue-1)*k_wayG+j)->tag <<" "<<tagValue<<" HIT"<<endl;
            return 1;
        }
    }
    cout<<(tlbGPF+(setValue-1)*k_wayG+j)->tag <<" "<<tagValue<<" MISS"<<endl;
    updateTlb(tagValue, setValue, tlbGPF);
    /*
    for (i = 0; i < a; i++)
    {
        //if (tlbGPF[setValue][i].tag == tag)
        if((tlbGPF+setValue+i)->tag == tagValue)
            return (tlbGPF+setValue+i)->frame;
    }
    */
    
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

int main()
{
    cout<<"\nSimulation is starting ... \n" << endl;
    int check = calculateBits(LADDR);
    if(check==-1)
    {
        cout<<"Not allowed Div by Zero Error"<<endl;
        return 0;
    }
    struct node *tlb =(struct node *)malloc(setG*k_wayG*sizeof(struct node));
    assert(tlb!=NULL);
    fillTlb(tlb);

    int temp;
    long int count = 0, total = 0;
    fstream myfile(INPUT_FILE, ios_base::in);
    printf("Input is taken from address.txt file:\n");
    ui address;
    while (myfile >> address)
    {
        //printf("%" PRIu64"\n" ,address);
        int x = getPageFrame(tlb,address);
        if (x == 1)
            count++;
        total++;
    }

    cout<<"--------------------------------------------------------Statistics------------------------------------------------------------------"<<endl;
    printf("Total Address : %ld\n", total);
    printf("Total Hits : %ld\n", count);
    printf("Hit Ratio : %f\n", (double)count / (double)total);

    
    
}
