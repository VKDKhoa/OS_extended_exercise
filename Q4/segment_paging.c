#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>

// one segment
struct sgn_t{
    uint32_t sgn; //segment number
    struct pgn_t pgn; // page which this segment is holding 
};

struct seg_table{
    struct sgn_t *sgn_list; // list of segment
    uint32_t size; // size of segment table
};

// one page
struct pgn_t{
    uint32_t pgn; // mot page chua trang cua no va danh sach frame trong main mem phy
    struct framephy_struct *fp_list;
};

// page table contain list of page
struct page_table{
    struct pgn_t *page_list;
    uint32_t size;
};

// main memory
// frame
struct framephy_struct{
    uint32_t fpn;
    struct framephy_struct *next;
};

struct memphy_struct {
   /* Basic field of data and size */
   char *storage;
   int maxsz;
   struct framephy_struct *fp_list;
//    /* Sequential device fields */ 
//    int rdmflg;
//    int cursor;

   /* Management structure */
//    struct framephy_struct *free_fp_list;
//    struct framephy_struct *used_fp_list;
};

int main(){
    char logicAddress[7];

}