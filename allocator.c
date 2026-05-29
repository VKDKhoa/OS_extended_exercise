#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#define MAX (1 << 21) // 2^21 bytes = 2MB

struct framephy_struct{
    uint32_t start;
    uint32_t size;
    char is_allocated;
    char *owner;
    struct framephy_struct *next;
};

struct memphy_struct{
    int maxsz;
    struct framephy_struct *lst_frames;
};
struct framephy_struct *create_new_frame(uint32_t size, char is_allocated, char *owner){
    struct framephy_struct *new_frame = (struct framephy_struct*)malloc(sizeof(struct framephy_struct));
    new_frame->size = size;
    new_frame->is_allocated = is_allocated;
    new_frame->owner = (char *)malloc(sizeof(char) * 256);
    strcpy(new_frame->owner,owner);
    new_frame->next = NULL;
    return new_frame;

}
// contiguous allocation strategies
// worst fit: find maximum hole
void add_frame_worstfit(struct memphy_struct *mram, struct framephy_struct *frame){
    struct framephy_struct *curr_frame = mram->lst_frames; //assume it is not null
    uint32_t max_size_hole = curr_frame->size; 
    while(curr_frame != NULL){
        if(curr_frame->size > max_size_hole){
            max_size_hole = curr_frame->size;
        }
        curr_frame = curr_frame->next;
    }
    if(max_size_hole < frame->size){
        printf("Not enough mem for Process %s size %u\n",frame->owner,frame->size);
        free(frame->owner);
        free(frame);
        return;
    }

    curr_frame = mram->lst_frames; //move back to head
    if(curr_frame->next == NULL) //meaning that is first time to add frame
    {
        frame->start = curr_frame->start;
        frame->next = curr_frame; 
        
        mram->lst_frames = frame;

        curr_frame->start += frame->size;
        curr_frame->size -= frame->size;
    }
    else
    {
        struct framephy_struct *next_frame = NULL;
        struct framephy_struct *prev_frame = NULL;
        while(curr_frame != NULL)
        {
            if(curr_frame->next != NULL)
            {
                if(curr_frame->next->size == max_size_hole)
                    prev_frame = curr_frame;
            }
            else if(curr_frame->size == max_size_hole){
                next_frame = curr_frame->next;
                break;
            }
            curr_frame = curr_frame->next;
        }
        frame->start = curr_frame->start; //ok
        prev_frame->next = frame;
        frame->next = curr_frame;
        curr_frame->start += frame->size;
        curr_frame->size -= frame->size;
    } 
}

//first fit: find first hole approriate
void add_frame_firstfit(struct memphy_struct *mram, struct framephy_struct *frame){
    struct framephy_struct *curr_frame = mram->lst_frames;
    while(curr_frame != NULL){
        //printf("frame [%u:%u], alloc_stat = %c size = %u\n",curr_frame->start,curr_frame->start+curr_frame->size-1,curr_frame->is_allocated,curr_frame->size);
        if(curr_frame->size >= frame->size && curr_frame->is_allocated == 'H'){
            //printf("SUIT FRAME ADDRESS[%u:%u]\n",curr_frame->start,curr_frame->size);
            frame->start = curr_frame->start;
            if(frame->start != 0){
                struct framephy_struct *link = mram->lst_frames;
                while(link->next != curr_frame){
                    link = link->next;
                }
                link->next = frame;
            }
            else{
                mram->lst_frames = frame;
            }
            curr_frame->size -= frame->size;
            if(curr_frame->size > 0)
            {
                curr_frame->start = frame->start + frame->size;
                frame->next = curr_frame;
            }
            else
            {
                frame->next = curr_frame->next;
                free(curr_frame->owner);
                free(curr_frame);
            }

            return;
        }
        curr_frame = curr_frame->next;
    }
    if(curr_frame == NULL){
        printf("Not enough memory for Process %s size %u\n",frame->owner,frame->size);
        free(frame->owner);
        free(frame);
    }
}
// best fit: find best suitable hole for process

void add_frame_bestfit(struct memphy_struct *mram, struct framephy_struct *frame){
    struct framephy_struct *curr_frame = mram->lst_frames;
    struct framephy_struct *best_hole = NULL;
    uint32_t best_fit = mram->maxsz;
    while(curr_frame != NULL){
        if(curr_frame->is_allocated != 'H'){
            curr_frame = curr_frame->next;
            continue;
        }
        if(curr_frame->size - frame->size > 0 && curr_frame->size - frame->size < best_fit){
            best_hole = curr_frame;
        }
        curr_frame = curr_frame->next;
    }
    if(best_hole == NULL){
        printf("not enough memory\n");
        return;
    }
    curr_frame = mram->lst_frames;
    while(curr_frame != NULL){
        if(curr_frame == best_hole){
            frame->start = curr_frame->start;
            if(frame->start == 0){
                mram->lst_frames = frame;
            }
            else{
                struct framephy_struct *link = mram->lst_frames;
                while(link->next != curr_frame){
                    link = link->next;
                }
                link->next = frame;
            }
            curr_frame->size -= frame->size;
            if(curr_frame->size > 0)
            {
                curr_frame->start = frame->start + frame->size;
                frame->next = curr_frame;
            }
            else
            {
                frame->next = curr_frame->next;
                free(curr_frame->owner);
                free(curr_frame);
            }
            return;

        }
        curr_frame = curr_frame->next;
    }
}
// handling hole
void merge_hole(struct memphy_struct *mram){
    struct framephy_struct *curr_frame = mram->lst_frames;
    while(curr_frame != NULL ){
        if(curr_frame->next == NULL){
            return;
        }
        if(curr_frame->is_allocated == 'H' && curr_frame->next->is_allocated == 'H'){
            uint32_t tot_size = curr_frame->size + curr_frame->next->size;
            curr_frame->size = tot_size;
            struct framephy_struct *tmp = curr_frame->next;
            curr_frame->next = tmp->next;
            if(tmp->next != NULL)
                curr_frame->next->start = curr_frame->start + curr_frame->size;
        }
        curr_frame = curr_frame->next;
    }
}
void release(char *Process_name,struct memphy_struct *mram){
    struct framephy_struct *curr_frame = mram->lst_frames;
    while(curr_frame != NULL){
        if(strcmp(curr_frame->owner,Process_name) == 0){
            curr_frame->is_allocated = 'H';
            strcpy(curr_frame->owner,"");
            break;
        }
        curr_frame = curr_frame->next;
    }
    if (curr_frame == NULL){
        printf("Process %s is not found in memory\n",Process_name);
        return;
    }
    merge_hole(mram);
}

void compact(struct memphy_struct *mram){
    struct framephy_struct *used = NULL;
    struct framephy_struct *tmp = mram->lst_frames;
    uint32_t tot_size_hole = 0;
    while(tmp != NULL){
        if(tmp->is_allocated == 'H')
        {
            tot_size_hole += tmp->size;
        }
        else
        {
            if(used == NULL){
                used = create_new_frame(tmp->size,tmp->is_allocated,tmp->owner);
                used->start = 0;
            }
            else
            {
                struct framephy_struct *new_node = create_new_frame(tmp->size,tmp->is_allocated,tmp->owner);
                struct framephy_struct *tmp1 = used;
                while(tmp1->next != NULL){
                    tmp1 = tmp1->next;
                }
                new_node->start = tmp1->start + tmp1->size;
                tmp1->next = new_node;
            }
        }
        tmp = tmp->next;
    }
    if(tot_size_hole == 0){
        mram->lst_frames = used;
        return;
    }
    if(used == NULL){
        struct framephy_struct *hole = create_new_frame(tot_size_hole,'H',"");
        hole->start = 0;
        mram->lst_frames = hole;
        return;
    }
    
    struct framephy_struct *hole = create_new_frame(tot_size_hole,'H',"");
    tmp = used;
    while(tmp->next != NULL){
        tmp = tmp->next;
    }
    hole->start = tmp->start + tmp->size;
    tmp->next = hole;
    mram->lst_frames = used;
}
int main(int argc, char *argv[]){
    uint32_t n = atoi(argv[1]);
    
    struct memphy_struct *mram = (struct memphy_struct *)malloc(sizeof(struct memphy_struct));
    mram->maxsz = n;
    mram->lst_frames = create_new_frame(mram->maxsz,'H',"");
    mram->lst_frames->start = 0;
    char *token[4];
    for(int i = 0; i < 4; i++){
        token[i] = NULL;
    }
    while(1)
    {
        printf("allocator>");
        char request[256];
        fgets(request, sizeof(request),stdin);
        request[strcspn(request,"\n")] = '\0';
        
        if(strcmp(request,"X") == 0){
            break;
        }
        else if(strcmp(request,"STAT") == 0)
        {
            struct framephy_struct *head = mram->lst_frames;
            while(head != NULL)
            {
                char name[256];
                if(head->is_allocated == 'H'){
                    strcpy(name,"Unused");
                }
                else
                    sprintf(name,"Process %s",head->owner);
                printf("Address [%u:%u] %s\n",head->start,head->start + head->size-1,name);
                head = head->next;
            }
            //continue;
        }
        else if(strcmp(request,"C") == 0){
            compact(mram);
        }

        token[0] = strtok(request," ");
        for(int i = 1; i < 4; i++){
            token[i] = strtok(NULL," ");
        }
        if(strcmp(token[0],"RQ") == 0){
            // RQ P0 1000 W
            struct framephy_struct *new_frame = create_new_frame(atoi(token[2]),'P',token[1]);
            if(strcmp("W",token[3]) == 0)
                add_frame_worstfit(mram,new_frame);
            else if(strcmp("F",token[3]) == 0){
                add_frame_firstfit(mram,new_frame);
            }
                
            else
                add_frame_bestfit(mram,new_frame);

        }
        else if(strcmp(token[0],"RL") == 0){
            release(token[1],mram);
        }
    }
    //free all allocated memory
    while(mram->lst_frames != NULL){
        struct framephy_struct *temp = mram->lst_frames;
        mram->lst_frames = mram->lst_frames->next;
        free(temp->owner);
        free(temp);
    }
    free(mram);
    return 0;
}