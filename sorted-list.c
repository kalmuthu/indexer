#include <stdio.h>
#include <stdlib.h>
#include "sorted-list.h"


void nodeswap(struct Node * one, struct Node * two)    // helper function to use when sorting a list 
{
  void * temp = one->data;
  one->data = two->data;
  two->data = temp;
}

void destroy(struct Node * node) 
{
  free(node->data);
  free(node);
}

void SLDestroy(SortedListPtr list)
{
   if(!list)
   {
    printf("No list to destroy \n");
    return;
    }

    struct Node* target = NULL;

    while(list->head)
    {
        target = list->head;           // target the head node
        list->df(target->data);   // free data at the head
        list->head = list->head->next; //  move the head
        free(target);                  // free the target node , repeat
    }
    free(list);
    printf("List deleted successfully \n");
    return;
}


void sortList(SortedListPtr list) 
{

  if (!list)
  {
    printf("No remaining list to sort \n");
    return;
  }

  struct Node * ptr = list->head;

  while (ptr->next != NULL && list->cf(ptr->data, ptr->next->data) <= 0)    // continue comparing two nodes until a larger one is found at ptr-Next
  {
    nodeswap(ptr, ptr->next);  // swap the data.
    ptr = ptr->next;           // continue looking for unsorted elements
  }

}


SortedListPtr SLCreate(CompareFuncT cf, DestructFuncT df) 
{
  struct SortedList* list = malloc(sizeof(*list));
  list->cf = cf;
  list->df = df;
  list->head = NULL;
  list->numitems = 0;
  list->nextList = NULL;
  return list;
}

int SLInsert(SortedListPtr list, void *newObj) 
{

  if(!list || !newObj)
    {
      printf("Insertion failed \n");      
      return 0;              
    }

 /* struct Node * duptr = list->head;

  while (duptr)              // checks if data already exists in list
  {
   if (list->cf(duptr->data, newObj) == 0)
   {
    printf("Insertion failed, duplicate data \n");
    return 0;
   }
   duptr = duptr->next;
  } */
    
  struct Node * newNode = malloc(sizeof(struct Node));
  newNode->data = newObj;
  newNode->next = list->head;  // put new object at the front of the list;
  newNode->refs = 0;
  newNode->refs++;        // increase reference count
  newNode->ghost = 0;    // node is not a ghost (yet :0)
  list->head = newNode;
  list->numitems++;
  sortList(list);
  return 1;
}

int SLRemove(SortedListPtr list, void *newObj) 
{

  if(!list || !newObj)
  {  
    printf("Removal failed \n");    
    return 0;              
  }


struct Node *target = list->head;
struct Node *prev = NULL;


while(target != NULL)
{
       
        if(list->cf(target->data, newObj) == 0)
        {
            if(!prev)         //deleting head
            {    
              list->head = list->head->next;
                if(list->head)
                {
                    list->head->refs++;     //New Head gets one more ref count
                }
              target->refs--;    //target no longer has the sorted list pointing to it
              target->ghost = 1;
                if(target->refs < 1)
                {     
                    list->df(target->data);
                    if(target->next !=NULL)
                    {   
                        target->next->refs--;  //decrement the reference count
                    }
                    free(target);
                    printf("Head Node was free'd from memory \n");
                    list->numitems--;
                    return 1;
                }
              list->numitems--;
              printf("Head Node has been ghosted \n");
              return 1;
            }
            else                                  // not deleting the head
            {                                  
                prev->next = target->next;  
                target->refs--;
                target->ghost = 1;   
                if(target->next)              // if the target has a next node
                { 
                  target->next->refs++;              // if it has a next, now be pointed to by two nodes
                }
                if(target->refs < 1)             // if refs go to zero, free it
                {
                    list->df(target->data);
                    if(target->next)
                    {
                        target->next->refs--;
                    }
                    free(target);
                    printf("A Node was free'd from memory \n");
                    list->numitems--;
                    return 1;
                }
              list->numitems--;  
              printf("A Node has been ghosted \n");
              return 1;  
            }
        }
  prev = target;
  target = target->next;
}
    printf("Target Node could not be found, deletion failed \n");
    return 0;

}


void * SLNextItem(SortedListIteratorPtr iter) 
{
  if (!iter->curr) 
  {
    printf("No current Node to iterate from \n");
    return NULL;
  } 
    
  if (iter->curr->next)
  {
    iter->curr->refs--;
    iter->curr = iter->curr->next;
    iter->curr->refs++;
    return iter->curr->data;
  }

  else 
  {
   // printf("iteration error, could not find next Node \n");
    return NULL;
  }
}

void * SLGetItem(SortedListIteratorPtr iter)
{
  void* data;

  if (iter->curr)
  {
    data = iter->curr->data;
      if (!data)
      {
        printf("No data found at specified Node \n");
        return NULL;
      }
      else
        return data;
  }
  else
    printf("No node found to retrieve data from \n");
    return NULL;
}

void SLDestroyIterator(SortedListIteratorPtr iter) 
{
  if (iter->curr)
  {
  iter->curr->refs--;
  }
  free(iter);
  printf("Iterator destroyed \n");
}

SortedListIteratorPtr SLCreateIterator(SortedListPtr list) 
{
  SortedListIteratorPtr iter = malloc(sizeof(SortedListIteratorPtr));
  iter->curr = list->head;
  iter->curr->refs++;
  return iter;
}