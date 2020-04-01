#include"list.h"
void vListInitialiseItem(ListItem_t * const pxItem)
{
    pxItem->pvContainer = NULL;
}

void vListInitialise(List_t* const pxList)
{
    /*list index point to the last one*/
    pxList->pxIndex = (ListItem_t*) &(pxList->xListEnd);
    /*the last one ItemValue set to be max, make sure it will be the last one*/
    pxList->xListEnd.xItemValue =portMAX_DELAY;
    /**/
    pxList->xListEnd.pxNext = (ListItem_t *) &(pxList->xListEnd);
    pxList->xListEnd.pxPrevious = (ListItem_t*)&(pxList->xListEnd);

    /*set List Number of items to be zero , indicate that the List is empty */
    pxList->uxNumberOfItems = (UBaseType_t)0;

}

void vListInsertEnd(List_t* const pxList, ListItem_t * pxNewListItem)
{
    ListItem_t * const pxIndex = pxList->pxIndex;
    pxNewListItem->pxNext = pxIndex;
    pxNewListItem->pxPrevious = pxIndex->pxPrevious;
    pxIndex->pxPrevious->pxNext = pxNewListItem;
    pxIndex->pxPrevious = pxNewListItem;

    /*remeber the List that contains the Item*/
    pxNewListItem->pvContainer = (void*)pxList;

    (pxList->uxNumberOfItems)++;

}

void vListInsert(List_t* const pxList, ListItem_t * const pxNewListItem)
{
    ListItem_t *pxIterator;

    /*get the ItemValue*/
    const TickType_t xValueOfInsertion = pxNewListItem->xItemValue;

    /*find the place that we should insert */
    if(xValueOfInsertion == portMAX_DELAY)
    {
        pxIterator = pxList->xListEnd.pxPrevious;
    }
    else
    {
        for(pxIterator=(ListItem_t*)&(pxList->xListEnd);
        pxIterator->pxNext->xItemValue<= xValueOfInsertion;
        pxIterator = pxIterator->pxNext)
        {

        }
    }
    /*Insert the  new item*/
    pxNewListItem->pxNext = pxIterator->pxNext;
    pxNewListItem->pxNext->pxPrevious = pxNewListItem;
    pxNewListItem->pxPrevious = pxIterator;
    pxIterator->pxNext = pxNewListItem;

    /*set the specific List that contains the item*/
    pxNewListItem->pvContainer = (void*)pxList;
    (pxList->uxNumberOfItems)++;
}

UBaseType_t uxListRemove (ListItem_t* const pxItemToRemove)
{
    /*get the list that contains the item*/
    List_t* const pxList = (List_t*)pxItemToRemove->pvContainer;

    pxItemToRemove->pxNext->pxPrevious = pxItemToRemove->pxPrevious;
    pxItemToRemove->pxPrevious->pxNext = pxItemToRemove->pxNext;

    /*adjust the index of the list*/
    if(pxList->pxIndex = pxItemToRemove)
    {
        pxList->pxIndex = pxItemToRemove->pxPrevious;
    }

    pxItemToRemove->pvContainer=NULL;
    /*index number -- */
    (pxList->uxNumberOfItems)--;

    return pxList->uxNumberOfItems;
    
}