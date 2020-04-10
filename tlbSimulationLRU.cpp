/*
 if 64 bit LA
   
   Page Size 2MB so offset = 21 bits
   k-way, k = 4
   Tlb entries = 4
   so, #set = 2^3/2^2 = 2^1 = 2
  Tag = 64 - 1 - 21 = 42bits
  
 --------------------------------------------
|    tag    |   set offset |   page offset   |
 --------------------------------------------
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
#define PAGEOFFSET 21
#define INPUT_FILE "addresses.txt"
#define FRAME 20 //number of bits for frame
#define ui uint64_t

using namespace std;

struct node
{
    ui tag;
    unsigned int frame;
    int valid;
    int counter;
};

int setGB = 0;
int setG = 0;
int k_wayG = ASSOCIATIVITY;
int turnG = 0;
ui pageOffsetG = 0;
ui pageValueG = 0;
ui tagG = 0;
unsigned int frame = FRAME;
int *count;
bool *full;

int calculateBits(int lAddr)
{
    pageOffsetG = PAGEOFFSET;
    if (ASSOCIATIVITY == 0)
        return -1;

    setG = ENTRIES / ASSOCIATIVITY;

    setGB = log2(setG);
    tagG = lAddr - setGB - pageOffsetG;
}

void setValidBit(struct node **tlbFT) // This function is used to initialize valid bit by 0
{
    unsigned int s;
    int i, j;
    s = setG;

    for (i = 0; i < s; i++)
    {
        for (j = 0; j < k_wayG; j++)
        {
            (*tlbFT + i * k_wayG + j)->valid = 0;
            (*tlbFT + i * k_wayG + j)->counter = 0;
        }
    }
}

int updateCounter(ui setValue, int index, struct node *tlbUC)
{
    //cout << "Before Update Counter:\n";
    for (int i = 0; i < setG; i++)
    {
        for (int j = 0; j < k_wayG; j++)
        {
            int c = (tlbUC + i * k_wayG + j)->counter;
            int v = (tlbUC + i * k_wayG + j)->valid;
            ui t = (tlbUC + i * k_wayG + j)->tag;
            //cout << " Count: " << c << " Valid: " << v << " Tag: " << t<<"|";
        }
        //cout << "\n";
    }
    int x = 0;
    for (int i = 0; i < k_wayG; i++)
    {
        int v = (tlbUC + setValue * k_wayG + i)->valid;
        if (v && i!=index)
        {
            (tlbUC + setValue * k_wayG + i)->counter += 1;
        }
    }
    //cout << "After Update Counter:\n";
    for (int i = 0; i < setG; i++)
    {
        for (int j = 0; j < k_wayG; j++)
        {
            int c = (tlbUC + i * k_wayG + j)->counter;
            int v = (tlbUC + i * k_wayG + j)->valid;
            ui t = (tlbUC + i * k_wayG + j)->tag;
            //cout << " Count: " << c << " Valid: " << v << " Tag: " << t<<"|";
        }
        //cout << "\n";
    }
    //cout << "\n";
}
int updateCounterHit(ui setValue, int index, struct node *tlbUCH)
{
    //cout << "Before Update HIT Counter:"<<endl;
    for (int i = 0; i < setG; i++)
    {
        for (int j = 0; j < k_wayG; j++)
        {
            int c = (tlbUCH + i * k_wayG + j)->counter;
            int v = (tlbUCH + i * k_wayG + j)->valid;
            ui t = (tlbUCH + i * k_wayG + j)->tag;
            //cout << " Count: " << c << " Valid: " << v << " Tag: " << t<<"|";
        }
        //cout << "\n";
    }
    int x = 0;
    int c = (tlbUCH + setValue * k_wayG + index)->counter;
    for (int i = 0; i < k_wayG; i++)
    {
        int v = (tlbUCH + setValue * k_wayG + i)->valid;
        int c1 = (tlbUCH + setValue * k_wayG + i)->counter;
        if (v && c1 < c)
        {
            (tlbUCH + setValue * k_wayG + i)->counter += 1;
        }
    }
    (tlbUCH + setValue * k_wayG + index)->counter = 0;
    //cout << "After Update HIT Counter:\n";
    for (int i = 0; i < setG; i++)
    {
        for (int j = 0; j < k_wayG; j++)
        {
            int c = (tlbUCH + i * k_wayG + j)->counter;
            int v = (tlbUCH + i * k_wayG + j)->valid;
            ui t = (tlbUCH + i * k_wayG + j)->tag;
            //cout << " Count: " << c << " Valid: " << v << " Tag: " << t<<"|";
        }
        //cout << "\n";
    }
}
int getPageFrame(struct node *tlbGPF, ui logicalAddressL)
{
    static int time = 0;
    /*
    ui pageSize = (ui)pow(2, pageOffsetG);
    ui pageValueL = logicalAddressL % pageSize;
    logicalAddressL = logicalAddressL / pageSize; //PAGE BITS REMOVED
    int setValue = logicalAddressL % setG;
    //printf("setValue :%d\n",setValue);
    logicalAddressL = logicalAddressL / setG;
    ui tagSize = (ui)pow(2, tagG);
    ui tagValue = logicalAddressL % tagSize; //TAG BITS REMAINED
    */
    /*
    ui pageValueL = logicalAddressL & 0x000000000001FFFFF;
    int setValue = (logicalAddressL & 0x0000000000200000) >> 21;
    ui tagValue = (logicalAddressL & 0xFFFFFFFFFFC00000) >> 22;*/

    ui pageValueL = logicalAddressL & 0x000000000001FFFFF;
    int setValue = (logicalAddressL & 0x0000000001E00000) >> 21;
    ui tagValue = (logicalAddressL & 0xFFFFFFFFFE000000) >> 25;

    
    int size = ASSOCIATIVITY - 1;
    int index = 0;
	//time++;
    //cout <<"\nIN GET PAGE FRAME :"<<time<<endl;
    //cout<<"\nTAG:"<<tagValue<<endl;
    int d, h;
    if (!full[setValue])
        for (int i = 0; i < k_wayG; i++) //NOT FILL
        {
            int v = (tlbGPF + setValue * k_wayG + i)->valid;
            int c = (tlbGPF + setValue * k_wayG + i)->counter;
            ui t = (tlbGPF + setValue * k_wayG + i)->tag;

            if (v && tagValue == t) //HIT
            {
                index = i;
                updateCounterHit(setValue, index, tlbGPF);
                return 1;
            }
            if (!v && i != size) //tof ill elements except last one
            {
                (tlbGPF + setValue * k_wayG + i)->valid = 1;
                (tlbGPF + setValue * k_wayG + i)->counter = 0;
                (tlbGPF + setValue * k_wayG + i)->tag = tagValue;
                index = i;
                d = updateCounter(setValue, index, tlbGPF);
                return 0;
            }
            if (!v && i == size) //for last element to fill
            {
                (tlbGPF + setValue * k_wayG + i)->valid = 1;
                (tlbGPF + setValue * k_wayG + i)->counter = 0;
                (tlbGPF + setValue * k_wayG + i)->tag = tagValue;
                full[setValue] = true;
                index = i;
                d = updateCounter(setValue, index, tlbGPF);
                return 0;
            }
        }

    else
    {
        for (int i = 0; i < k_wayG; i++) //FULL
        {
            int v = (tlbGPF + setValue * k_wayG + i)->valid;
            int c = (tlbGPF + setValue * k_wayG + i)->counter;
            ui t = (tlbGPF + setValue * k_wayG + i)->tag;

            if (v && tagValue == t) //HIT
            {
                index = i;
                updateCounterHit(setValue, index, tlbGPF);
                return 1;
            }
        }
        int c, max = 0, maxIndex = 0;
        max = (tlbGPF + setValue * k_wayG + 0)->counter;
        for (int i = 1; i < k_wayG; i++) //FIND max counter
        {
            c = (tlbGPF + setValue * k_wayG + i)->counter;
            if (max < c)
            {
                max = c;
                maxIndex = i;
            }
        }
        //cout<<"\n"<<"MAX INDEX:"<<maxIndex<<endl;
        updateCounter(setValue, maxIndex, tlbGPF);
        (tlbGPF + setValue * k_wayG + maxIndex)->tag = tagValue;
        (tlbGPF + setValue * k_wayG + maxIndex)->counter = 0;
        (tlbGPF + setValue * k_wayG + maxIndex)->valid = 1;
        (tlbGPF + setValue * k_wayG + maxIndex)->frame = rand();
        //cout << "IN GET PAGE FRAME: After Update COunter\n";
        for (int i = 0; i < setG; i++)
        {
            for (int j = 0; j < k_wayG; j++)
            {
                int c = (tlbGPF + i * k_wayG + j)->counter;
                int v = (tlbGPF + i * k_wayG + j)->valid;
                ui t = (tlbGPF + i * k_wayG + j)->tag;
                //cout << " Count: " << c << " Valid: " << v << " Tag: " << t<<"|";
            }
            //cout << "\n";
        }
        return 0;
    }
}
int main()
{
    cout << "\nSimulation is starting ... \n"
         << endl;

    int check = calculateBits(LADDR);
    if (check == -1)
    {
        cout << "Not allowed, Div by Zero Error" << endl;
        return 0;
    }

    struct node *tlb = (struct node *)malloc(setG * k_wayG * sizeof(struct node));
    assert(tlb != NULL);

    count = (int *)malloc(setG * sizeof(int)); //This is to tack the TLB
    count[setG] = {0};                         //Initialize all with 0

    full = (bool *)malloc(setG * sizeof(bool));
    full[setG] = {false};
    setValidBit(&tlb);

    int temp;
    long int countH = 0, total = 0;
    fstream myfile(INPUT_FILE, ios_base::in);
    cout << "Input is taken from address.txt file:\n";
    ui address;
    while (myfile >> address)
    {
        //printf("%" PRIu64"\n" ,address);
        int x = getPageFrame(tlb, address);
        if (x == 1)
            countH++;
        total++;
    }
    /*
    for(int i=0;i<setG;i++)
    {
        for(int j=0;j<ASSOCIATIVITY;j++)
        {
            cout<<"Valid-"<<(tlb + i * k_wayG + j)->valid<<" TAG "<<(tlb + i * k_wayG + j)->tag<<" ";
        }
        cout<<endl;
    }*/
    cout << "\n-DETAILS----------------------------------------\n"
         << endl;
    printf("Total Address : %ld\n", total);
    printf("Total Hits : %ld\n", countH);
    printf("Hit Ratio : %f\n", (double)countH / (double)total);
}