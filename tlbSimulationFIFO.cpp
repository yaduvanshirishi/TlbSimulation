/*
 if 64 bit LA
   
   Page Size 2MB so offset = 21 bits
   k-way, k = 4
   Tlb entries = 64
   so, #set = 2^6/2^2 = 2^4 = 16
  Tag = 64 - 4 - 21 = 49bits
  
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
    short int valid;
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

void fifo(int setValue,ui tagValue,struct node *tlbF)
{
    int size = ASSOCIATIVITY;
    int i = count[setValue];

    (tlbF + setValue * k_wayG + i)->tag = tagValue;
    (tlbF + setValue * k_wayG + i)->frame = rand() % ((ui)pow(2, frame));
    count[setValue] = (count[setValue] + 1)%size;
}

int contains(int setValue,ui tagValue,struct node *tlbC)
{
    int i,validC;
    ui tagC;

    for (i = 0; i < k_wayG; i++)
    {
        validC = (tlbC + setValue * k_wayG + i)->valid;
        tagC = (tlbC + setValue * k_wayG + i)->tag;
        if (validC && (tagC == tagValue)) 
        {
            return 1;//HIT
        }
    }
    return 0; //MISS
}

int pageFault(int setValue,ui tagValue,struct node *tlbPF)
{
    int size = ASSOCIATIVITY, checkValid;
    ui tV;

    int i = count[setValue];

    tV = (tlbPF + setValue*size + i)->tag ;
    checkValid = (tlbPF + setValue*size + i)->valid;
    if(contains(setValue,tagValue,tlbPF)) return 1; //CASE 1- NOT FULL AND HIT , CASE 2- HIT AND FULL
    if((full[setValue])) {                //CASE 3: FULL AND MISS
        fifo(setValue,tagValue,tlbPF);
        return 0;
    }
        
    if((!checkValid)) //CASE 4: NOT FULL AND MISS
    {
        (tlbPF + setValue * k_wayG + i)->tag = tagValue;
        (tlbPF + setValue * k_wayG + i)->frame = rand() % ((ui)pow(2, frame));
        (tlbPF + setValue * k_wayG + i)->valid = 1;
        if(size-1 == i) full[setValue] = true;
        count[setValue] = (count[setValue] + 1)%size;
            
    }
    return 0;
}

int calculateBits(int lAddr)
{
    pageOffsetG = PAGEOFFSET;
    if (ASSOCIATIVITY == 0)
        return -1;

    setG = ENTRIES / ASSOCIATIVITY;

    setGB = log2(setG);
    tagG = lAddr - setGB - pageOffsetG;
}

void setValidBit(struct node *tlbFT) // This function is used to initialize valid bit by 0
{
    unsigned int s;
    int i, j;
    s = setG;
    for (i = 0; i < s; i++)
    {
        for (j = 0; j < k_wayG; j++)
        {
            (tlbFT + i * k_wayG + j)->valid = 0;
        }
    }
}

int getPageFrame(struct node *tlbGPF, ui logicalAddressL)
{
    ui pageValueL = logicalAddressL & 0x000000000009FFFFF;
    int setValue = (logicalAddressL & 0x0000000001E00000) >> 21;
    ui tagValue = (logicalAddressL & 0xFFFFFFFFFE000000) >> 25;

    int val=pageFault(setValue,tagValue,tlbGPF);
    if(val == 1) return 1; //HIT
    return 0; //MISS
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

    count = (int *)malloc(setG*sizeof(int));//This is to tack the TLB
    count[setG] = {0};//Initialize all with 0

    full = (bool *)malloc(setG*sizeof(bool));
    full[setG] = {false};
    setValidBit(tlb);

    int temp;
    long int countH = 0, total = 0;
    fstream myfile(INPUT_FILE, ios_base::in);
    cout << "Input is taken from address.txt file:\n";
    ui address;
    while (myfile >> address)
    {
        int x = getPageFrame(tlb, address);
        if (x == 1)
            countH++;
        total++;
    }

    cout << "\n-----------------STATS----------------------------------------\n"
         << endl;

    cout<<"Logical Address Bits: "<<LADDR<<" bits"<<"\n";
    cout<<"TLB Entries: "<<ENTRIES<<" entries"<<"\n";
    cout<<"Associativity: "<<ASSOCIATIVITY<<"-way"<<"\n";
    cout<<"Page Offset Bits: "<<PAGEOFFSET<<" bits"<<"\n\n";
    cout<<"Tag: "<<tagG<<" bits,"<<" Set Bits: "<<setGB<<" bits"<<"\n";
    cout<<"Total Address : "<<total<<"\n";
    cout<<"Total Hits : "<<countH<<"\n";
    cout<<"Hit Ratio : "<<((double)countH / (double)total)<<"\n\n";
}
