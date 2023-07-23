#include <stdio.h>
#include <stdlib.h>

typedef struct node {
    int data;
    struct node *link;
} nodet;

void nprint(nodet * current);
void addend(nodet *current,int val);
void addbeg(nodet **current,int val);
int rmbeg(nodet **head);
int rmend(nodet *head);

int main() {
    nodet *head=(nodet *) malloc(sizeof(nodet));
    head->data=6;
    head->link=(nodet *) malloc(sizeof(nodet));
    head->link->data=9;
    head->link->link=NULL;

    addend(head,88);
    addbeg(&head, 66);
    addbeg(&head, 99);

    nprint(head);
    printf("\n\n");
    rmbeg(&head);
    nprint(head);

    printf("\n\n");
    rmend(head);
    nprint(head);

    return 0;
}

void nprint(nodet * current) {
    while(current!=NULL) {
        printf("%d\n", current->data);
        current=current->link;
    }
}

void addend(nodet *current,int val){
    while(current->link!=NULL) {
        current=current->link;
    }
    current->link=(nodet *) malloc(sizeof(nodet));
    current->link->data=val;
    current->link->link=NULL;
}

void addbeg(nodet **head,int val) {
    nodet *newhead=(nodet *) malloc(sizeof(nodet));
    newhead->data = val;
    newhead->link=*head;
    *head=newhead;
}

int rmbeg(nodet **head) {
    int rval = -1;
    if(*head==NULL) {
        return -1;
    }
    nodet *newhead=(nodet *) malloc(sizeof(nodet));
    rval=(*head)->data;
    newhead=(*head)->link;
    free(*head);
    *head=newhead;
    printf("dropped %d from start\n", rval);
    return rval;
}

int rmend(nodet *head) {
    int rval=0;
    if(head->link=NULL) {
        rval=head->data;
        free(head);
        return rval;
    }
    nodet * current = head;
    printf("test\n");
    // crash here: Segmentation fault (core dumped)//
    while (current->link->link != NULL) {
        current = current->link;
    }
    rval=current->link->data;
    free(current->link);
    current->link=NULL;
    printf("removed %d from th end\n", rval);
    return rval;
}
