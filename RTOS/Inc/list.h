#ifndef _LIST_H
#define _LIST_H
#include<stdint.h>
#include<stddef.h>
#include"portmacro.h"

struct xLIST_ITEM
{
    uint32_t xItemValue;/*use it to sort the item*/
    struct xLIST_ITEM *pxNext;
    struct xLIST_ITEM *pxPrevious;
    void *pvOwner;
    void *pvContainer;
};
typedef struct xLIST_ITEM ListItem_t;

struct xMINI_LIST_ITEM
{
    uint32_t xItemValue; 
    struct xLIST_ITEM *pxNext;
    struct xLIST_ITEM *pxPrevious;
};
typedef struct xMINI_LIST_ITEM MiniListItem_t;

typedef struct xLIST
{
    uint32_t uxNumberOfItems;
    ListItem_t * pxIndex;
    MiniListItem_t xListEnd;
}List_t;


/* ³õÊ¼»¯½ÚµãµÄÓµÓÐÕß */
#define listSET_LIST_ITEM_OWNER( pxListItem, pxOwner )		( ( pxListItem )->pvOwner = ( void * ) ( pxOwner ) )
/* »ñÈ¡½ÚµãÓµÓÐÕß */
#define listGET_LIST_ITEM_OWNER( pxListItem )	( ( pxListItem )->pvOwner )

/* ³õÊ¼»¯½ÚµãÅÅÐò¸¨ÖúÖµ */
#define listSET_LIST_ITEM_VALUE( pxListItem, xValue )	( ( pxListItem )->xItemValue = ( xValue ) )

/* »ñÈ¡½ÚµãÅÅÐò¸¨ÖúÖµ */
#define listGET_LIST_ITEM_VALUE( pxListItem )	( ( pxListItem )->xItemValue )

/* »ñÈ¡Á´±í¸ù½ÚµãµÄ½Úµã¼ÆÊýÆ÷µÄÖµ */
#define listGET_ITEM_VALUE_OF_HEAD_ENTRY( pxList )	( ( ( pxList )->xListEnd ).pxNext->xItemValue )

/* »ñÈ¡Á´±íµÄÈë¿Ú½Úµã */
#define listGET_HEAD_ENTRY( pxList )	( ( ( pxList )->xListEnd ).pxNext )

/* »ñÈ¡Á´±íµÄµÚÒ»¸ö½Úµã */
#define listGET_NEXT( pxListItem )	( ( pxListItem )->pxNext )

/* »ñÈ¡Á´±íµÄ×îºóÒ»¸ö½Úµã */
#define listGET_END_MARKER( pxList )	( ( ListItem_t const * ) ( &( ( pxList )->xListEnd ) ) )

/* ÅÐ¶ÏÁ´±íÊÇ·ñÎª¿Õ */
#define listLIST_IS_EMPTY( pxList )	( ( BaseType_t ) ( ( pxList )->uxNumberOfItems == ( UBaseType_t ) 0 ) )

/* »ñÈ¡Á´±íµÄ½ÚµãÊý */
#define listCURRENT_LIST_LENGTH( pxList )	( ( pxList )->uxNumberOfItems )

/* »ñÈ¡Á´±í½ÚµãµÄOWNER£¬¼´TCB */
#define listGET_OWNER_OF_NEXT_ENTRY( pxTCB, pxList )										\
{																							\
	List_t * const pxConstList = ( pxList );											    \
	/* ½ÚµãË÷ÒýÖ¸ÏòÁ´±íµÚÒ»¸ö½Úµãµ÷Õû½ÚµãË÷ÒýÖ¸Õë£¬Ö¸ÏòÏÂÒ»¸ö½Úµã£¬
    Èç¹ûµ±Ç°Á´±íÓÐN¸ö½Úµã£¬µ±µÚN´Îµ÷ÓÃ¸Ãº¯ÊýÊ±£¬pxInedexÔòÖ¸ÏòµÚN¸ö½Úµã */\
	( pxConstList )->pxIndex = ( pxConstList )->pxIndex->pxNext;							\
	/* µ±Ç°Á´±íÎª¿Õ */                                                                       \
	if( ( void * ) ( pxConstList )->pxIndex == ( void * ) &( ( pxConstList )->xListEnd ) )	\
	{																						\
		( pxConstList )->pxIndex = ( pxConstList )->pxIndex->pxNext;						\
	}																						\
	/* »ñÈ¡½ÚµãµÄOWNER£¬¼´TCB */                                                             \
	( pxTCB ) = ( pxConstList )->pxIndex->pvOwner;											 \
}

#define listGET_OWNER_OF_HEAD_ENTRY( pxList )  ( (&( ( pxList )->xListEnd ))->pxNext->pvOwner )
void vListInitialiseItem(ListItem_t * const pxItem);
void vListInitialise(List_t* const pxList);
void vListInsertEnd(List_t* const pxList, ListItem_t * pxNewListItem);
void vListInsert(List_t* const pxList, ListItem_t * const pxNewListItem);
UBaseType_t uxListRemove (ListItem_t* const pxItemToRemove);

#endif 