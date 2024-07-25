// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

struct hashbucket
{
  struct spinlock lock;
  struct buf *bucketinnerbuf[NBUF];
  uint innerbuf_num;
};


struct {
  struct spinlock lock;
  struct buf buf[NBUF];
  struct hashbucket hashbkt[NBUCKET];
  uint glob_ttime;  //时间戳
  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  //struct buf head;
} bcache;


struct hashbucket*
bhash(uint blockno) {
    return &bcache.hashbkt[blockno % NBUCKET];
}

void
binit(void)
{
  struct buf *b;
int i;
  initlock(&bcache.lock, "bcache");
    bcache.glob_ttime=0;
struct hashbucket *temp;
  //对于bcache的每一个桶，进行初始化

for(temp=bcache.hashbkt;temp<bcache.hashbkt+NBUCKET;temp++)
{
  initlock(&temp->lock,"buffer");
  temp->innerbuf_num=0;
}


  
  for(i=0,b = bcache.buf; b < bcache.buf+NBUF; b++,i++){
        b->blockno = i;
        b->refcnt = 0;
        b->ttime = 0;
       
        b->valid = 0;
        struct hashbucket *bkt = bhash(b->blockno);
        b->bktplace = bkt->innerbuf_num;
        bkt->bucketinnerbuf[bkt->innerbuf_num++] = b;
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b=0;
  struct hashbucket* bkt = bhash(blockno);
  acquire(&bkt->lock);//小锁

  // Is the block already cached?
  for(int i=0;i<bkt->innerbuf_num;i++){
    b=bkt->bucketinnerbuf[i];
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bkt->lock);
      acquiresleep(&b->lock);
      return b;
    }
  }
//没有找到！！
release(&bkt->lock);
acquire(&bcache.lock);
acquire(&bkt->lock);
//上面的释放再分配的主要目的是为了避免死锁，要先大锁再小锁

//于是 重新检查一遍
for(int i=0;i<bkt->innerbuf_num;i++){
    b=bkt->bucketinnerbuf[i];
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.lock);
      release(&bkt->lock);
      acquiresleep(&b->lock);
      return b;
    }
  }

while(1){
struct buf* eviction_buf=0;//将要指向替换的指针
int min_ttime=2147483647;
for(struct buf *temp=bcache.buf;temp<bcache.buf+NBUF;temp++)
{
  if(temp->refcnt==0&&temp->ttime<min_ttime)
  {
    min_ttime=temp->ttime;
    eviction_buf=temp;
  
  
  }





}
if(eviction_buf==0){
panic("死锁啦啊啊啊");
}
struct hashbucket* eviction_bkt = bhash(eviction_buf->blockno);//获得倒霉蛋的桶
//确定不要重复加锁了
if(eviction_bkt!=bkt)
  acquire(&eviction_bkt->lock);
if(eviction_buf->refcnt!=0)
{
  if(eviction_bkt!=bkt)
  release(&eviction_bkt->lock);
  continue;
}
//正常情况
//驱逐倒霉蛋
uint num=eviction_bkt->innerbuf_num; 
if(eviction_buf->bktplace<num-1)//不是最后一个，换最后一个的位置并且修改有关值
{
eviction_bkt->bucketinnerbuf[eviction_buf->bktplace]=eviction_bkt->bucketinnerbuf[num-1];
eviction_bkt->bucketinnerbuf[eviction_buf->bktplace]->bktplace=eviction_buf->bktplace;
}
eviction_bkt->innerbuf_num--;
if(eviction_bkt!=bkt)
  release(&eviction_bkt->lock);//已完成倒霉蛋所在的桶的相关操作

//按照给的代码一样操作
eviction_buf->bktplace=bkt->innerbuf_num;
bkt->bucketinnerbuf[bkt->innerbuf_num++]=eviction_buf;
 eviction_buf->dev = dev;
eviction_buf->blockno = blockno;
  eviction_buf->valid = 0;
  eviction_buf->refcnt = 1;
      release(&bcache.lock);
      release(&bkt->lock);
      acquiresleep(&eviction_buf->lock);
         return eviction_buf;
}
  
  
  
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  bcache.glob_ttime=b->ttime=bcache.glob_ttime+1;
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  bcache.glob_ttime=b->ttime=bcache.glob_ttime+1;
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");
//因为我们有桶，所以用小锁
  struct hashbucket* bkt = bhash(b->blockno);
  releasesleep(&b->lock);

  acquire(&bkt->lock);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
   b->ttime = 0;
  }
  
  release(&bkt->lock);
}

void
bpin(struct buf *b) {
  //因为我们有桶，所以用小锁
  struct hashbucket* bkt = bhash(b->blockno);
  acquire(&bkt->lock);
  b->refcnt++;
  bcache.glob_ttime=b->ttime=bcache.glob_ttime+1;
  release(&bkt->lock);
}

void
bunpin(struct buf *b) {
 struct hashbucket* bkt = bhash(b->blockno);
  acquire(&bkt->lock);
  b->refcnt--;
  release(&bkt->lock);
}


