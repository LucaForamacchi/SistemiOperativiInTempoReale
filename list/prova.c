#include <stdio.h>
#include "list.h"

int main(int argc, char* argv[]) {
    LIST mylist;
    ItemType i;

    mylist = NewList();
    printf("Fase di inizializzazione: Creata lista\n");

    printf("Fase di Controllo: ");
    if (isEmpty(mylist))
        printf(" La lista e' vuota\n");
    else
        printf(" La lista non e' vuota \n");

    i.value = 1;
    mylist = EnqueueLast(mylist,i);
    
    i.value = 2;
    mylist = EnqueueLast(mylist,i);
    
    i.value = 3;
    mylist = EnqueueLast(mylist,i);
    
    i.value = 4;
    mylist = EnqueueLast(mylist,i);
    
    printf("Contenuto della lista: [");
    PrintList(mylist);
    printf("]\n");
    
    i.value = 5;
    mylist = EnqueueOrdered(mylist,i, 3);

    printf("Contenuto della lista: [");
    PrintList(mylist);
    printf("]\n");

    i.value = 10;
    mylist = EnqueueFirst(mylist,i);

    printf("Contenuto della lista: [");
    PrintList(mylist);
    printf("]\n");

    mylist = DequeueLast(mylist);

    printf("Contenuto della lista: [");
    PrintList(mylist);
    printf("]\n");

    printf("Fase di Distruzione dati\n");
    mylist = DeleteList(mylist);

    printf("Fase di Controllo: ");
    if (isEmpty(mylist))
        printf(" La lista e' vuota\n");
    else
        printf(" La lista non e' vuota \n");

    return 0;

} /* Main */
