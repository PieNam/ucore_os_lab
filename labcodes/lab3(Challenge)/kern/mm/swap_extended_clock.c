#include <defs.h>
#include <x86.h>
#include <stdio.h>
#include <string.h>
#include <swap.h>
#include <swap_extended_clock.h>
#include <list.h>


list_entry_t pra_list_head;

static int
_ec_init_mm(struct mm_struct *mm) {     
    list_init(&pra_list_head);
    mm->sm_priv = &pra_list_head;
    //cprintf(" mm->sm_priv %x in ec_init_mm\n",mm->sm_priv);
    return 0;
}

static int
_ec_map_swappable(struct mm_struct * mm, uintptr_t addr,
                    struct Page * page, int swap_in) {
    list_entry_t *head=(list_entry_t*) mm->sm_priv;
    list_entry_t *entry=&(page->pra_page_link);
    assert(entry != NULL && head != NULL);
    // insert the new page to the back of the list
    list_add(head->prev, entry);
    // set its dirty bit to 0
    struct Page * ptr = le2page(entry, pra_page_link);
    pte_t * pte = get_pte(mm->pgdir, ptr->pra_vaddr, 0);
    *pte &= ~PTE_D;
    return 0;
}

// get the victim: scan the list
// if the dirty bit of the page = 1, set it to 0
// else if the dirty bit = 0, we got the victim
static int
_ec_swap_out_victim(struct mm_struct * mm,
                    struct Page ** ptr_page, int in_tick) {
    list_entry_t *head=(list_entry_t*) mm->sm_priv;
    assert(head != NULL);
    assert(in_tick==0);
    list_entry_t *scanner = head;
    // loop and scan
    while (1) {
        scanner = list_next(scanner);
        if (scanner == head) {
            scanner = list_next(scanner);
        }
        struct Page * ptr = le2page(scanner, pra_page_link);
        pte_t * pte = get_pte(mm->pgdir, ptr->pra_vaddr, 0);
        // if the dirty bit = 1, set it to 0
        if ((*pte & PTE_D) == 1) {
            *pte &= ~PTE_D;
        }
        // else if the dirty bit = 0, set it as the victim
        else {
            assert(ptr != NULL);
            *ptr_page = ptr;
            list_del(scanner);
            break;
        }
    }
    return 0;
}

static int
_ec_check_swap(void) {
    cprintf("write Virt Page c in ec_check_swap\n");
    *(unsigned char *)0x3000 = 0x0c;
    assert(pgfault_num==4);
    cprintf("write Virt Page a in ec_check_swap\n");
    *(unsigned char *)0x1000 = 0x0a;
    assert(pgfault_num==4);
    cprintf("write Virt Page d in ec_check_swap\n");
    *(unsigned char *)0x4000 = 0x0d;
    assert(pgfault_num==4);
    cprintf("write Virt Page b in ec_check_swap\n");
    *(unsigned char *)0x2000 = 0x0b;
    assert(pgfault_num==4);
    cprintf("write Virt Page e in ec_check_swap\n");
    *(unsigned char *)0x5000 = 0x0e;
    assert(pgfault_num==5);
    cprintf("write Virt Page b in ec_check_swap\n");
    *(unsigned char *)0x2000 = 0x0b;
    assert(pgfault_num==5);
    cprintf("write Virt Page a in ec_check_swap\n");
    *(unsigned char *)0x1000 = 0x0a;
    assert(pgfault_num==6);
    cprintf("write Virt Page b in ec_check_swap\n");
    *(unsigned char *)0x2000 = 0x0b;
    assert(pgfault_num==7);
    cprintf("write Virt Page c in ec_check_swap\n");
    *(unsigned char *)0x3000 = 0x0c;
    assert(pgfault_num==8);
    cprintf("write Virt Page d in ec_check_swap\n");
    *(unsigned char *)0x4000 = 0x0d;
    assert(pgfault_num==9);
    cprintf("write Virt Page e in ec_check_swap\n");
    *(unsigned char *)0x5000 = 0x0e;
    assert(pgfault_num==10);
    cprintf("write Virt Page a in ec_check_swap\n");
    assert(*(unsigned char *)0x1000 == 0x0a);
    *(unsigned char *)0x1000 = 0x0a;
    assert(pgfault_num==11);
    return 0;
}


static int
_ec_init(void) {
    return 0;
}

static int
_ec_set_unswappable(struct mm_struct *mm, uintptr_t addr) {
    return 0;
}

static int
_ec_tick_event(struct mm_struct *mm) { 
    return 0; 
}


struct swap_manager swap_manager_ec = {
     .name            = "extended_clock swap manager",
     .init            = &_ec_init,
     .init_mm         = &_ec_init_mm,
     .tick_event      = &_ec_tick_event,
     .map_swappable   = &_ec_map_swappable,
     .set_unswappable = &_ec_set_unswappable,
     .swap_out_victim = &_ec_swap_out_victim,
     .check_swap      = &_ec_check_swap,
};
