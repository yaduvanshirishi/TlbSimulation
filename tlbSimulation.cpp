#include <iostream>
#include <cstdio>
#include <cstdint>
#include <cmath>
#include <fstream>
#include <cstdlib>
#include <assert.h>
#define PAGE_REPLACEMENT 1 //1 for LRU, comment it for FIFO
#define LADDR 64 //Logical Address bits
#define ENTRIES 64
#define ASSOCIATIVITY 4
#define PAGEOFFSET 21
#define INPUT_FILE "addresses.txt"
#define FRAME 20 //number of bits for frame
#define ui uint64_t



struct TlbRow
{
    ui tag;
    unsigned int frame;
    int valid;
    int counter;
};

int setGB{};
int setG{};
int k_wayG {ASSOCIATIVITY};
int turnG {};
ui pageOffsetG{};
ui pageValueG{};
ui tagG{};
unsigned int frame{FRAME};
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
    return 0;
}

void setValidBit(struct TlbRow **tlbFT) 
{   // This function is used to initialize valid bit by 0
    unsigned int s;
    unsigned int i;
    int j;
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
//LRU start
void updateCounter(ui setValue, int index, struct TlbRow *tlbUC)
{
    for (int i = 0; i < k_wayG; i++)
    {
        int v = (tlbUC + setValue * k_wayG + i)->valid;
        if (v && i!=index)
        {
            (tlbUC + setValue * k_wayG + i)->counter += 1;
        }
    }
}

void updateCounterHit(ui setValue, int index, struct TlbRow *tlbUCH)//It will update counter
{
    //Counter of the referenced block is reset to 0
    //Counters with values originally lower than the referenced one are incremented by 1, and all others ramain unchanged.

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

}

int getPageFrameLRU(struct TlbRow *tlbGPF, ui logicalAddressL)
{
    //ui pageValueL = logicalAddressL & 0x000000000001FFFFF;
    int setValue = (logicalAddressL & 0x0000000001E00000) >> 21;
    ui tagValue = (logicalAddressL & 0xFFFFFFFFFE000000) >> 25;

    
    int size = ASSOCIATIVITY - 1;
    int index = 0;
    if (!full[setValue])
        for (int i = 0; i < k_wayG; i++) //NOT FILL
        {
            int v = (tlbGPF + setValue * k_wayG + i)->valid;
            ui t = (tlbGPF + setValue * k_wayG + i)->tag;

            if (v && tagValue == t) //HIT
            {
                index = i;
                updateCounterHit(setValue, index, tlbGPF);
                return 1;
            }

            if (!v && i != size) //for all elements except last one till not full
            {
                (tlbGPF + setValue * k_wayG + i)->valid = 1;
                (tlbGPF + setValue * k_wayG + i)->counter = 0;
                (tlbGPF + setValue * k_wayG + i)->tag = tagValue;
                index = i;
                updateCounter(setValue, index, tlbGPF);
                return 0;
            }

            if (!v && i == size) //for last element to fill
            {
                (tlbGPF + setValue * k_wayG + i)->valid = 1;
                (tlbGPF + setValue * k_wayG + i)->counter = 0;
                (tlbGPF + setValue * k_wayG + i)->tag = tagValue;
                full[setValue] = true;
                index = i;
                updateCounter(setValue, index, tlbGPF);
                return 0;
            }
        }

    else
    {
        for (int i = 0; i < k_wayG; i++) //FULL
        {
            int v = (tlbGPF + setValue * k_wayG + i)->valid;
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
        updateCounter(setValue, maxIndex, tlbGPF);
        (tlbGPF + setValue * k_wayG + maxIndex)->tag = tagValue;
        (tlbGPF + setValue * k_wayG + maxIndex)->counter = 0;
        (tlbGPF + setValue * k_wayG + maxIndex)->valid = 1;
        (tlbGPF + setValue * k_wayG + maxIndex)->frame = rand();

        return 0;
    }
    return 0;
}
//LRU END


//FIFO Start
void fifo(int setValue,ui tagValue,struct TlbRow *tlbF)
{
    int size = ASSOCIATIVITY;
    int i = count[setValue];

    (tlbF + setValue * k_wayG + i)->tag = tagValue;
    (tlbF + setValue * k_wayG + i)->frame = rand() % ((ui)pow(2, frame));
    count[setValue] = (count[setValue] + 1)%size;
}

int contains(int setValue,ui tagValue,struct TlbRow *tlbC)
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

int pageFault(int setValue,ui tagValue,struct TlbRow *tlbPF)
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

int getPageFrameFIFO(struct TlbRow *tlbGPF, ui logicalAddressL)
{
    //ui pageValueL = logicalAddressL & 0x000000000001FFFFF;
    int setValue = (logicalAddressL & 0x0000000001E00000) >> 21;
    ui tagValue = (logicalAddressL & 0xFFFFFFFFFE000000) >> 25;

    int val=pageFault(setValue,tagValue,tlbGPF);
    if(val == 1) return 1; //HIT
    return 0; //MISS
}

//FIFO END
int main()
{
    std::cout << "\nSimulation is starting ... \n"
         << std::endl;

    int check = calculateBits(LADDR);
    if (check == -1)
    {
        std::cout << "Not allowed, Div by Zero Error" << std::endl;
        return 0;
    }

    struct TlbRow *tlb = (struct TlbRow *)malloc(setG * k_wayG * sizeof(struct TlbRow));
    assert(tlb != NULL); 

    #ifndef PAGE_REPLACEMENT // This count is for FIFO
    count = (int *)malloc(setG*sizeof(int));//This is to tack the TLB
    count[setG] = {0};//Initialize all with 0
    #endif

    full = (bool *)malloc(setG * sizeof(bool));
    full[setG] = {false};
    setValidBit(&tlb);

    long int countH = 0, total = 0;
    std::fstream myfile(INPUT_FILE, std::ios_base::in);
    std::cout << "Input is taken from address.txt file:\n";
    ui address;
    while (myfile >> address)
    {
        #ifdef PAGE_REPLACEMENT
        int x = getPageFrameLRU(tlb, address);
        #else 
        int x = getPageFrameFIFO(tlb, address);
        #endif
        if (x == 1)
            countH++;
        total++;
    }

    std::cout << "\n-----------------STATS----------------------------------------\n"
         << std::endl;

    std::cout<<"Logical Address Bits: "<<LADDR<<" bits"<<"\n";
    std::cout<<"TLB Entries: "<<ENTRIES<<" entries"<<"\n";
    std::cout<<"Associativity: "<<ASSOCIATIVITY<<"-way"<<"\n";
    std::cout<<"Page Offset Bits: "<<PAGEOFFSET<<" bits"<<"\n\n";

    #ifdef PAGE_REPLACEMENT
    std::cout<<"-:LRU PAGE REPLACEMENT:-\n";
    #else
    std::cout<<"-:FIFO PAGE REPLACEMENT:-\n";
    #endif
 
    std::cout<<"Tag: "<<tagG<<" bits,"<<" Set Bits: "<<setGB<<" bits"<<"\n";
    std::cout<<"Total Address : "<<total<<"\n";
    std::cout<<"Total Hits : "<<countH<<"\n";
    
    std::cout<<"Hit Ratio : "<<((double)countH / (double)total)<<"\n\n";

    return 0;
}
