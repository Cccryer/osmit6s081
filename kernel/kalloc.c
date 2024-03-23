// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

//使用run链表管理空闲物理页，将物理页解释为run结构体（r = (struct run*)pa），分配出去时抹去run内容（memset((char*)r, 5, PGSIZE)，回收时解释为run结构体（r = (struct run*)pa）。方便管理

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

struct ref_pgnum
{
  struct spinlock lock;
  int cnt[PHYSTOP / PGSIZE];
}refpgnum;

int iscowpage(pagetable_t pagetable, uint64 va)
{
  //判断va所在页是不是cow页
  if(va > MAXVA)
    return -1;
  pte_t *pte = walk(pagetable, va, 0);
  if(pte == 0)
    return -1;
  //同时u v cow才是cowpage
  if((*pte & PTE_COW) == 0 || (*pte & PTE_V) == 0 || (*pte & PTE_U) == 0)
    return -1;
  return 0;

}


int cowhandler(pagetable_t pagetable, uint64 va)
{
  if(iscowpage(pagetable, va) != 0)
    return -1;
  char *mem;
  if((mem = kalloc()) == 0) //无物理页可分配
    return -1;

  pte_t *pte = walk(pagetable, va, 0);//取vapte
  uint64 pa = PTE2PA(*pte);//取va对应pa
  memmove((char*)mem, (char*)pa, PGSIZE);//pa内容复制到mem
  (*pte) &= ~PTE_V;
  if(mappages(pagetable, PGROUNDDOWN(va), PGSIZE, (uint64)mem, (PTE_FLAGS(*pte) | PTE_W) & ~PTE_COW) != 0) {//将mem映射到va所在页,一定注意是穿页地址不是va
      kfree(mem);
      *pte |= PTE_V;
      return -1;
    }
  kfree((void *)pa);
  return 0;
}


int kaddrefcnt(void *pa)
{
  if(((uint64)pa % PGSIZE) != 0 || (char *)pa < end || (uint64)pa >= PHYSTOP)
      return -1;
  acquire(&refpgnum.lock);
  ++refpgnum.cnt[(uint64)pa / PGSIZE];
  release(&refpgnum.lock);
  return 0;
}

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&refpgnum.lock, "ref");
  freerange(end, (void*)PHYSTOP);

}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
  {
    refpgnum.cnt[(uint64)p / PGSIZE] = 1;//to正常执行kfree(p)
    kfree(p);
  }
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.

  acquire(&refpgnum.lock);
  if(--refpgnum.cnt[(uint64)pa / PGSIZE] == 0)//引用为0才回收
  {
      release(&refpgnum.lock);


      memset(pa, 1, PGSIZE);

      r = (struct run*)pa; //将pa所指物理页解释成run结构体，串起来

      acquire(&kmem.lock);
      r->next = kmem.freelist;
      kmem.freelist = r;
      release(&kmem.lock);

  }
  else
  {
    release(&refpgnum.lock);
  }

}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
  {
    kmem.freelist = r->next;
    acquire(&refpgnum.lock);
    refpgnum.cnt[(uint64)r / PGSIZE] = 1;//分配时初始化为1
    release(&refpgnum.lock);
  }
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
