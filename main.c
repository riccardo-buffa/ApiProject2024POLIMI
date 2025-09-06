#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#define MAXINPUT 1050
#define MAXTABLE 2000
#define MAXHEAPREADY 30
#define MAXSTRING 5
#define MAXCOMMAND 20
#define MAXSTRINGLIMITS 30

//I organize Ingredients in linked lists where HeadIngredient contains only the ingredient name 
//and in Ingredient the remaining data, this way I avoid saving the ingredient name multiple times
struct ingredient{
    int weight;
    int expdate;
    struct ingredient *next;
};
typedef struct ingredient Ingredient; 
struct HeadIngredient
{
    char *name;
    Ingredient *firstIngredient;
    struct HeadIngredient *next;

};
typedef struct HeadIngredient HeadIngredient; 

//Recipes are made of recipe which contains the name, a circular list of IngredientName which in turn contains the direct
//pointer to the ingredient, this way I avoid searching for the ingredient multiple times.
struct IngredientName{
    int weight;
    HeadIngredient *ingredient; 
    struct IngredientName *next;
};
typedef struct IngredientName IngredientName; 

struct recipe{
    char *name; 
    IngredientName *CIngred; 
    struct recipe *next;
};
typedef struct recipe Recipe; 
//Orders instead, besides the necessary data, contain the pointer to the recipe, also in this case I avoid 
//saving the string and reduce search times
struct order{
    int ammount, time;
    int weight;
    Recipe *orecipe;
    struct order *next;
};
typedef struct order Order; 
//Orders are saved in two different data structures:
//->Pending orders are a queue
struct PendingList{
    Order* Head; 
    Order* Tail; 
};
typedef struct PendingList PendingList; 
//->Ready orders are a heap
struct ReadyArray
{
    Order *Array; 
    int size;
    int capacity;
};
typedef struct ReadyArray ReadyArray; 
//Recipes and ingredients are hash tables where collisions are managed through linked lists 
struct HashRecipe{
    Recipe *table[MAXTABLE];
};
typedef struct HashRecipe HashRecipe; 
struct HashIngredient{
    HeadIngredient *table[MAXTABLE];
};
typedef struct HashIngredient HashIngredient; 

//The hash function is a djb2
unsigned int hash(const char *key) {
    unsigned int hash = 5381;
    while (*key) {
        hash = ((hash << 5)+hash) + *key++;
    }
    return hash % MAXTABLE;
}

//startFunction for Ingredient
HeadIngredient *newNodeHeadIngredient(char *name, HashIngredient *ht)
{
    //Space is allocated for HeadIngredient and for the string
    HeadIngredient* newnode;
    newnode= (HeadIngredient*)malloc(sizeof(HeadIngredient));
    if(name!=NULL)
    {
        newnode->name= (char*)malloc(sizeof(char)*strlen(name)+1);
        strcpy(newnode->name, name);
    }
    else
    {
        newnode->name=NULL;
    }
    newnode->next=NULL;
    newnode->firstIngredient=NULL; 
    //I already perform the insertion into the hash table 
    int index=hash(name); 
    if(ht->table[index]==NULL)
        ht->table[index] = newnode;
    else
    {
        newnode->next = ht->table[index];
        ht->table[index]=newnode; 
    } 
    return newnode; 
}
Ingredient* newNodeIngredient(int weight, int expdate)
{
    Ingredient* newnode;
    newnode= (Ingredient*)malloc(sizeof(Ingredient));
    newnode->next= NULL;
    newnode->expdate=expdate;
    newnode->weight =weight; 
    return newnode;
}
HashIngredient *InizializeTableIngredient()
{
    HashIngredient *ht; 
    ht= (HashIngredient*)calloc(1,sizeof(HashIngredient));
    return ht;
}
void freeAllHashTableIngredient(HashIngredient *ht)
{
    Ingredient *elim,*same;
    HeadIngredient *temp, *elimHead; 
    for(int i=0; i<MAXTABLE; i++)
    {
        temp=ht->table[i];
        while(temp!=NULL)
        {
            same=temp->firstIngredient;
            while (same!=NULL)
            {
                elim=same; 
                same=same->next; 
                free(elim);
            }
            elimHead=temp; 
            temp=temp->next; 
            free(elimHead->name);
            free(elimHead);
        }
    }
    free(ht);
}
HeadIngredient *Search_Ingredient(HashIngredient *ht, unsigned int index, char *name)
{
    HeadIngredient *temp=ht->table[index];
    while (temp!=NULL){
        if (strcmp(temp->name+MAXSTRING , name+MAXSTRING) == 0) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}
void Add_Ingredient(HashIngredient *ht, char* name,int weight, int expdate)
{
    unsigned int index = hash(name);
    //I perform an insertion of an ingredient
    Ingredient *newNode = newNodeIngredient(weight, expdate);
    HeadIngredient *HeadNode=Search_Ingredient(ht, index, name);
    //If the HeadIngredient hasn't been created, it gets created and inserted into the hash table and the ingredient 
    //to be created is placed as the first element
    if (HeadNode == NULL) {
        HeadNode = newNodeHeadIngredient(name, ht);
        HeadNode->firstIngredient=newNode; 
    } 
    else {
        Ingredient *temp = HeadNode->firstIngredient, *prec=NULL;  
        //Otherwise if the HeadIngredient already exists, the ingredient is inserted respecting the descending order of expiration dates 
        while (temp != NULL && temp->expdate < expdate) {
            prec = temp;
            temp = temp->next;
        }
        if(prec==NULL)
        {
            newNode->next=HeadNode->firstIngredient;
            HeadNode->firstIngredient=newNode;
        }
        else
        {
            prec->next=newNode;
            newNode->next=temp; 
        }
    }
    //The time complexity is O(1+a) where a would be the sum of HeadIngredients with the same name and ingredients in the list
}

//start Function for recipe
HashRecipe *InizializeTableRecipe()
{
    HashRecipe *ht; 
    ht= (HashRecipe*)calloc(1,sizeof(HashRecipe));
    return ht;
}
void freeAllHashTableRecipe(HashRecipe *ht)
{
    Recipe *curr, *temp;
    IngredientName *elim, *head;
    for(int i=0; i<MAXTABLE; i++)
    {
        curr=ht->table[i];
        temp=curr;
        while(temp!=NULL)
        {
            curr=temp; 
            temp=temp->next;
            head=curr->CIngred;
            do
            {
                elim=curr->CIngred;
                curr->CIngred=curr->CIngred->next;
                free(elim);
            }while (curr->CIngred!=head);
            
            free(curr->name);
            free(curr);
        }
    }
    free(ht);
}
Recipe* newNodeRecipe(char *name, IngredientName **CIngred)
{
    Recipe* newnode;
    if(!(newnode= (Recipe*)malloc(sizeof(Recipe))))
    {
        return NULL;
    }
    if(name!=NULL)
    {
        if(!(newnode->name= (char*)malloc(sizeof(char)*strlen(name)+1)))
        {
            return NULL;
        }
        else
        {
            strcpy(newnode->name, name);
        }
    }
    else
    {
        newnode->name=NULL;
    }
    newnode->CIngred = *CIngred; 
    newnode->next=NULL; 
    return newnode;
}
Recipe *Search_Recipe(HashRecipe *ht, unsigned int index,  char *name)
{
    Recipe *temp=ht->table[index];
    while (temp!=NULL){
        if (strcmp(temp->name+MAXSTRING, name+MAXSTRING) == 0) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}
void Insert_Recipe(HashRecipe *ht, char *name, IngredientName **List){
    unsigned int index = hash(name);
    Recipe *newNode=Search_Recipe(ht, index, name);
    if(newNode==NULL)
    {   
        newNode = newNodeRecipe(name, List);
        if (ht->table[index] == NULL) {
            ht->table[index] = newNode;
        } else {
            Recipe *temp = ht->table[index];
            while (temp->next!=NULL) {
                temp = temp->next;
            }
            temp->next = newNode;
        }
    }
    else
    {
        newNode->CIngred=*List; 
    }
    //Recipe insertion is done similarly to ingredient insertion
    //The time complexity is always O(1+a)
}
void Remove_Recipe(HashRecipe *ht, char *name)
{
    unsigned int index = hash(name);
    Recipe *temp=ht->table[index];
    Recipe *prec = NULL;

    while (temp!=NULL && strcmp(temp->name+MAXSTRING, name+MAXSTRING) != 0) {
        prec = temp;
        temp = temp->next;
    }
    if(temp!=NULL)
    {
        if(prec==NULL)
        {
            ht->table[index] = temp->next;
        }
        else
        {
            prec->next=temp->next;
        }
        IngredientName *elim, *Head=temp->CIngred;
        do
        {
            elim=temp->CIngred;
            temp->CIngred=temp->CIngred->next;
            free(elim);
        }while (temp->CIngred!=Head);
        free(temp->name);
        free(temp);
        printf("rimossa\n");
    }
    else
    {
        printf("non presente\n");
    }
    //Recipe removal always has time complexity O(1+a)
}
IngredientName *newNodeIngredientName(char *name, int weight, HashIngredient *hti)
{
    IngredientName* newnode;
    
    newnode= (IngredientName*)malloc(sizeof(IngredientName));
    newnode->next= NULL;
    //In this case I search if the ingredient exists
    newnode->ingredient=Search_Ingredient(hti,hash(name), name);
    if(newnode->ingredient==NULL)
    {
        newnode->ingredient=newNodeHeadIngredient(name, hti);
    }
    //If it exists the pointer points to that ingredient otherwise a new one is created, because it's certain that 
    //the respective ingredient will also be created
    newnode->weight =weight; 
    return newnode;
}
void enqueueIngredientName(IngredientName **Head, IngredientName *e, IngredientName **Tail)
{
    if(*Head==NULL)
    {
        *Head= e;
        *Tail=e;
    }
    else
    {
        (*Tail)->next = e;
        *Tail = e; 
    }
}

//start Function for order
Order* NewOrder(char *name, int ammount, int weight, int time, HashRecipe *htr, Recipe *R)
{
    Order *o = (Order*)malloc(sizeof(Order));
    o->ammount=ammount;
    o->weight=weight;
    o->next=NULL;
    o->orecipe = R;
    o->time = time; 
    return o; 

}
PendingList *newList()
{
    PendingList *newL = (PendingList*)malloc(sizeof(PendingList));
    newL->Head = NULL; 
    newL->Tail = NULL; 
    return newL; 
}
ReadyArray *newArrayReady()
{
    ReadyArray *newL = (ReadyArray*)malloc(sizeof(ReadyArray));
    newL->Array=(Order*)calloc(MAXHEAPREADY, sizeof(Order));
    newL->capacity=MAXHEAPREADY;
    newL->size=0; 
    return newL; 
}
void ReallocNewArray(ReadyArray *list)
{
    list->capacity=list->capacity*2;
    list->Array=(Order*)realloc(list->Array, list->capacity*sizeof(Order));
}
void swap(Order *a, Order *b)
{
    Order temp=*a; 
    *a=*b;
    *b=temp;
}
int parent(int i)
{
    return i/2;
}
int left(int i)
{
    return 2*i;
}
int right(int i)
{
    return 2*i+1;
}
void heapify(ReadyArray *list, int i)
{
    int smallest = i; 
    int l=left(i);
    int r=right(i);
    if(l<=list->size&&list->Array[l].time<list->Array[i].time)
    {
        smallest=l;
    }
    else
    {
        smallest=i;
    }
    if(r<=list->size&&list->Array[r].time<list->Array[smallest].time)
    {
        smallest=r;
    }
    if(smallest!=i)
    {
        swap(&list->Array[i], &list->Array[smallest]);
        heapify(list, smallest);
    }
}
void heapsort(ReadyArray *list)
{
    for(int i=(list->size)/2; i>=0; i--)
    {
        heapify(list, i);
    }
    for(int i=list->size-1; i>0; i--)
    {
        swap(&list->Array[0], &list->Array[i]);
        heapify(list, i);
    }
}
int searchReadyOrder(ReadyArray *heap, char *name)
{
    int i=0;
    while(i<heap->size)
    {
        if(strcmp(heap->Array[i].orecipe->name+MAXSTRING, name+MAXSTRING)==0)
        {
            return i; 
        }
        i++;
    }
    return -1;
}
int extractmin(ReadyArray *heap)
{
    heap->Array[0]=heap->Array[heap->size-1];
    heap->size=heap->size-1;
    heapify(heap, 0);
    return 0; 
}
void EnqueuePending(PendingList *queue, Order *o)
{
    if(queue->Tail==NULL)
    {
        queue->Head=o;
        queue->Tail=o;
    }
    else
    {
        queue->Tail->next = o;
        queue->Tail=o;
    }
}
void InsertReadyOrder(ReadyArray *heap, Order *o)
{
    if(heap->size==heap->capacity)
    {
        ReallocNewArray(heap);
    }
    heap->Array[heap->size]=*o;
    heap->size=heap->size+1;
    int i=heap->size-1; 
    while (i>0&& heap->Array[parent(i)].time>heap->Array[i].time)
    {
        swap(&heap->Array[parent(i)],&heap->Array[i]);
        i=parent(i);
    }
    free(o);
    //The time complexity is log(n)
}
void UpdateStock(HashIngredient *hti, Recipe *r1, Order *o, int time)
{
    int weight, i;
    Ingredient *m, *elim, *prec;
    IngredientName *r=r1->CIngred;
    do
    {
        //This way I avoid the search
        m=r->ingredient->firstIngredient; 
        i=1;
        //r is the recipe ingredient so weight corresponds to how much weight is needed for that ingredient
        weight=(r->weight)*(o->ammount);
        prec=NULL; 
        while (m!=NULL&&i!=0)
        {
            //I don't check if there are enough ingredients, since it's done in ControlPending
            if(m->expdate > time)
            {
                if(m->weight - weight >=0)
                {
                    //I update the ingredient weight 
                    m->weight = m->weight - weight; 
                    i=0;
                }
                else
                {
                    //I delete the ingredient with weight 0
                    elim=m;
                    weight=weight-m->weight; 
                    if(prec==NULL)
                    {
                        m=m->next;
                        r->ingredient->firstIngredient=m; 
                    }
                    else
                    {
                        prec->next=m->next; 
                        m=m->next;
                    }
                    free(elim); 
                }
            }
            else
            {
                prec = m; 
                m = m->next; 
            }
        }     
        //Also in this case I delete the ingredient with weight 0
        if(m->weight==0)
        {
            elim=m;
            if(prec==NULL)
            {
                m=m->next;
                r->ingredient->firstIngredient=m; 
            }
            else
            {
                prec->next=m->next; 
                m=m->next;
            }
            free(elim); 
        }
        r=r->next;
    }while(r!=r1->CIngred);
    //The complexity is O(n*a) where n is the number of recipe ingredients and a is the number of ingredients with the same name
}
void Insert_Order(HashIngredient *hti,Order *o, PendingList *Pending, ReadyArray *ready, int time)
{
    int i=1;
    Recipe *r1=o->orecipe;
    IngredientName *e = r1->CIngred;
    Ingredient *m; 
    int weight;
    do
    {
        m=e->ingredient->firstIngredient;
        weight=0; 
        //I sum the weights of non-expired ingredients
        while (m!=NULL&&weight<(e->weight)*(o->ammount))
        {
            if(m->expdate>time)
            {
                weight = weight + m->weight;
            }
            m=m->next; 
        }
        //If there isn't enough material the order is inserted into the queue
        if(weight<(e->weight)*(o->ammount))
        {
            i=0;
            EnqueuePending(Pending, o);
            r1->CIngred=e;
        }
        else
        {
            e=e->next;
        }
    }while(e!=r1->CIngred&&i!=0);
    if(i==1)
    {
        //If I reach the end and the flag i isn't triggered, then I can insert the order into ready
        UpdateStock(hti, r1, o, time);
        InsertReadyOrder(ready, o); //Complexity O(log(n))
    }
    //The complexity is O(n) where n is the number of ingredients
}
Order *searchOrder(Order *Head, char *name)
{
    Order *o=Head;
    while (o!=NULL&&strcmp(name+MAXSTRING, o->orecipe->name+MAXSTRING)!=0)
    {
        o=o->next; 
    }
    return o;
}
void ControlPendingList(PendingList *pending, HashRecipe *htr, HashIngredient *hti, ReadyArray *ready, int time )
{
    Order *e=pending->Head, *next, *prec=NULL;
    IngredientName  *ListIngred;
    Ingredient *m;
    Recipe *r;
    int flag=0;
    int weight;
    //I need to scan all the waiting ingredients
    while (e!=NULL)
    {
        r=e->orecipe;
        ListIngred = r->CIngred;
        flag=0;
        do
        {
            flag=0;
            m=ListIngred->ingredient->firstIngredient;
            weight=0; 
            while (m!=NULL&&weight<(ListIngred->weight)*(e->ammount))
            {
                if(m->expdate>time)
                {
                    weight = weight + m->weight;
                }
                m=m->next; 
            }
            if(weight < (ListIngred->weight)*(e->ammount))
            {
                flag=1;
                //I save the point where scanning is interrupted, so that if in the next check
                //of an order with the same ingredient, the order is immediately left waiting
                r->CIngred=ListIngred;
            }else
            {
                ListIngred=ListIngred->next;
            }
        }while (ListIngred!=r->CIngred&&flag!=1);

        next = e->next;
        if(flag==0)
        {
            //as for order insertion, if the flag is triggered it goes to the ready heap
            if(prec==NULL)
            {
                pending->Head = e->next; 
            }
            else
            {
                prec->next=e->next;
            }
            if(e->next==NULL)
                pending->Tail = prec; 
            e->next=NULL;
            UpdateStock(hti, r, e, time);
            InsertReadyOrder(ready, e); 
        }
        else
        {
            prec=e;
        }
        e = next;
    }        
    //The complexity is O(n*m*i)
    //n->number of waiting orders
    //m->number of recipe ingredients 
    //i->number of ingredients with the same name
}
void DeleteAllList(PendingList *queue)
{
    Order *o=queue->Head, *elim;
    while (o!=NULL)
    {
        elim=o;
        o=o->next;
        free(elim);
    }
    free(queue);
}
void DeleteAllListReady(ReadyArray *heap)
{
    free(heap->Array);
    free(heap);
}

// courier
void CreateListCourier(ReadyArray *ready, ReadyArray *readycourrier, int *aweight, int w_courier)
{
    //in this function I select the elements whose sum of weights doesn't exceed the courier weight
    int i=0; 
    int size = ready->size; 
    while (i<size&&(w_courier >= *aweight+ready->Array[0].weight*ready->Array[0].ammount))
    {
        //if the array size is exceeded the array is reallocated
        if(readycourrier->size==readycourrier->capacity)
        {
            ReallocNewArray(readycourrier);
        }

        readycourrier->Array[i]=ready->Array[0];
        *aweight=*aweight+ready->Array[0].weight*ready->Array[0].ammount;
        extractmin(ready);// The complexity is log(n)
        readycourrier->size=readycourrier->size+1;
        i++;
    }
    //The complexity is O(nlog(n))
    //I decided to use Heaps, despite the fact that creating the list of orders to send to the courier is less efficient
    //than with lists, insertion is definitely log(n) and therefore since insertions are much more than
    //courier calls I decided to use heaps
}
void merge(ReadyArray *ready, int p, int q, int r) {
    int n1 = q - p + 1;
    int n2 = r - q;

    Order *L=calloc(n1 + 1,sizeof(Order)), *R=calloc(n2 + 1,sizeof(Order));

    for (int i = 0; i < n1; i++)
        L[i] = ready->Array[p + i];
    for (int i = 0; i < n2; i++)
        R[i] = ready->Array[q + 1 + i];

    L[n1].weight = INT_MIN;  
    L[n1].time = INT_MAX;  //I also insert a sentinel for time 
    R[n2].weight = INT_MIN;
    R[n2].time = INT_MAX;
    int i = 0, j = 0;
    for (int k = p; k <= r; k++) {
        if (L[i].weight*L[i].ammount > R[j].weight*R[j].ammount || (L[i].weight*L[i].ammount == R[j].weight*R[j].ammount && L[i].time < R[j].time)) {
            ready->Array[k] = L[i];
            i++;
        } else {
            ready->Array[k] = R[j];
            j++;
        }
    }
    free(L);
    free(R);
}
void MergeSort(ReadyArray *ready, int p, int r) {
    //I chose merge sort since it was the most efficient among stable sorting algorithms
    if (p < r) {
        int q = (p + r) / 2;
        MergeSort(ready, p, q);
        MergeSort(ready, q + 1, r);
        merge(ready, p, q, r);
    }
}
void courier(ReadyArray *ready, int w_courier, ReadyArray *readyCourier)
{
    int aweight=0; 
    int i=0; 
    //I create the list of orders to send to the courier O(nlog(n))
    CreateListCourier(ready,readyCourier, &aweight, w_courier);
    //I sort the list of orders to send to the courier O(nlog(n)) by weight order
    MergeSort(readyCourier, 0, readyCourier->size-1);
    //I print
    while(i<readyCourier->size)
    {
        printf("%d %s %d\n", readyCourier->Array[i].time, readyCourier->Array[i].orecipe->name, readyCourier->Array[i].ammount);
        i++;
        
    }  
    readyCourier->size=0; 
    //The complexity remains O(nlog(n)) where n is the number of orders to insert into the van
}
int getWord(char *input, char **word, int *length)
{
    //I created this function because I noticed that using strtok was taking a bit more time
    int i=0, flag=0;
    (*word)[i]='\0'; 
    while (input[*length] != ' ' && input[*length] != '\n' && input[*length] != '\0') {
        (*word)[i]=input[*length];
        (*length)++;
        i++; 
    }
    if(input[*length]=='\n')
    {
        flag=1;
    }
    else{
        (*length)++;
    }
    (*word)[i]='\0';
    return flag; 

}

int main(int argc, char *argv[])
{
    PendingList *pending=newList();
    ReadyArray *ready=newArrayReady(); 
    ReadyArray *readyCourrier=newArrayReady();
    HashRecipe *htr=InizializeTableRecipe(); 
    HashIngredient *hti=InizializeTableIngredient(); 
    IngredientName *HeadListIngred, *TailListIngred;
    Order *O;
    Recipe *m;
    IngredientName *e, *I;
    char input[MAXINPUT];
    char *num=malloc(sizeof(char)*MAXSTRING), *command=malloc(sizeof(char)*MAXCOMMAND), *name=malloc(sizeof(char)*MAXSTRINGLIMITS);
    char *numexp=malloc(sizeof(char)*MAXSTRING), *namesave=malloc(sizeof(char)*MAXSTRINGLIMITS);
    int t_courier=0, i=0, flag=0;
    int w_courier=0, lenght=0;
    int  adweight=0, pos;
    if(fgets(input, sizeof(input), stdin)!=NULL)
    {
        getWord(input, &num, &lenght);
        t_courier = atoi(num);
        getWord(input, &num, &lenght);
        w_courier = atoi(num); 
    }
    while (fgets(input, sizeof(input), stdin)!=NULL)
    {
        lenght=0; 
        getWord(input, &command, &lenght);        
        if(i%t_courier==0&&i!=0)
        {
            if(&ready->Array[0]!=NULL)
                courier(ready,w_courier, readyCourrier);
            else
                printf("camioncino vuoto\n");
        }
        if(strcmp(command, "aggiungi_ricetta")==0)
        {
            HeadListIngred = NULL;
            TailListIngred = NULL; 
            getWord(input, &namesave, &lenght);
            m=Search_Recipe(htr, hash(namesave),namesave);
            flag=0; 
            if (m==NULL)
            {                
                while(flag!=1)
                {
                    getWord(input, &name, &lenght);
                    flag = getWord(input, &num, &lenght);
                    e = newNodeIngredientName(name, atoi(num), hti); 
                    enqueueIngredientName(&HeadListIngred, e, &TailListIngred); 
                }
                TailListIngred->next=HeadListIngred;
                Insert_Recipe(htr, namesave, &HeadListIngred);  
                printf("aggiunta\n");
            }
            else
            {
                printf("ignorato\n");
            }

        }
        if(strcmp(command, "rimuovi_ricetta")==0)
        {
            getWord(input, &name, &lenght);
            Order *temp1=NULL;
            temp1= searchOrder(pending->Head, name);
            pos= searchReadyOrder(ready, name);
            if(temp1!=NULL||pos!=-1)
            {
                printf("ordini in sospeso\n");
            }
            else
            {
                Remove_Recipe(htr, name);
            }
        }
        if(strcmp(command, "rifornimento")==0)
        {
            flag=0;
            while(flag!=1)
            {
                getWord(input, &name, &lenght);
                getWord(input, &num, &lenght);
                flag = getWord(input, &numexp, &lenght);
                if(atoi(numexp)>i)
                {
                    Add_Ingredient(hti, name, atoi(num), atoi(numexp)); 
                }
            }
            printf("rifornito\n"); 
            if(pending->Head!=NULL)
            {
                ControlPendingList(pending, htr, hti, ready, i); 
            }
        }
        if(strcmp(command, "ordine")==0)
        {
            Recipe *R;
            adweight=0;
            getWord(input, &name, &lenght);
            getWord(input, &num, &lenght);
            R=Search_Recipe(htr,hash(name), name);
            if(R!=NULL)
            {
                I=R->CIngred;
                do
                {
                    adweight = adweight + I->weight;
                    I=I->next;
                }while(I!=R->CIngred);
                O = NewOrder(name, atoi(num), adweight, i, htr, R);
                Insert_Order(hti, O, pending, ready, i); 
                printf("accettato\n");
            }
            else
            {
                printf("rifiutato\n"); 
            }

        }
        i++;
        
    }

    if(i%t_courier==0&&i!=0)
    {
        if(&ready->Array[0]!=NULL)
            courier(ready,w_courier, readyCourrier);
        else
            printf("camioncino vuoto\n");
    }

    freeAllHashTableRecipe(htr);
    freeAllHashTableIngredient(hti);
    DeleteAllList(pending);
    DeleteAllListReady(ready);
    DeleteAllListReady(readyCourrier);
    free(num);
    free(numexp);
    free(command);
    free(name);
    free(namesave);
    return 0;
}
