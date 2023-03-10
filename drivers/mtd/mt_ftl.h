#ifndef __LINUX_MTFTL_H__
#define __LINUX_MTFTL_H__

#include <linux/mutex.h>
#include <linux/workqueue.h>
#include "linux/mtd/ubi.h"

/* #define MT_FTL_PROFILE */
#define MT_FTL_PARAM_LOCK_SUPPORT
#define MT_FTL_SUPPORT_COMPR

#define MT_FTL_SUCCESS 0
#define MT_FTL_FAIL -1

#define MT_INVALID_BLOCKPAGE		0xFFFFFFFE
#define MT_PAGE_HAD_BEEN_READ		0x9EAD
#define MT_MAGIC_NUMBER			0x3105

#define NAND_DEFAULT_VALUE		0xFFFFFFFF
#ifdef CONFIG_64BIT
#define NAND_DEFAULT_SECTOR_VALUE	0xffffffffffffffffULL
#else
#define NAND_DEFAULT_SECTOR_VALUE	NAND_DEFAULT_VALUE
#endif
#define NAND_RESERVED_PEB               (3) /* for WL/Scrub max:3, min:1*/
#define NAND_VOLUME			(8 * 1024 * 1024) /* max: Unit: KBytes */
#define NAND_TOTAL_BLOCK_NUM            (NAND_VOLUME >> 12) /* max: 4MB block */

#define FS_PAGE_SIZE			(4 * 1024)
#define PM_PER_NANDPAGE_SHIFT           (2)

#define PMT_TOTAL_CLUSTER_NUM           (NAND_VOLUME >> 14) /*max: 16MB*/

#define PMT_INDICATOR_PAGE_SHIFT	(10)
#define PMT_INDICATOR_DIRTY_SHIFT	(1)
#define PMT_INDICATOR_CACHED_SHIFT	(3)
#define PMT_LEB_PAGE_INDICATOR_PAGE_SHIFT	(12)
#define PMT_PAGE_SHIFT			(12)
#define PMT_PART_SHIFT			(12)
#define PMT_DATACACHE_BUFF_NUM_SHIFT	(4)
#define PMT_CACHE_NUM			(4) /* Max: 7 */
#define PMT_BLOCK_NUM			(5) /* for 8GB data, origin 3*/

#define CONFIG_START_BLOCK		(0)
#define REPLAY_BLOCK			(2)
#define PMT_START_BLOCK			(3)
#define DATA_START_BLOCK		(PMT_START_BLOCK + PMT_BLOCK_NUM)
#define MTKFTL_MAX_DATA_NUM_PER_PAGE	(1 << PMT_PART_SHIFT)

#define FTL_GC_RESERVED_PEB             (10)
/* reserved for ftl*/
#define NAND_RESERVED_FTL	(NAND_RESERVED_PEB + DATA_START_BLOCK + FTL_GC_RESERVED_PEB)

/* mt_ftl error messages */
#define mt_ftl_err(fmt, ...) \
		pr_err("[MTK FTL][%s][line:%d] " fmt "\n", __func__, __LINE__, ##__VA_ARGS__)
#define mt_ftl_debug(fmt, ...) \
		pr_debug("[MTK FTL][%s][line:%d] " fmt "\n", __func__, __LINE__, ##__VA_ARGS__)
#ifdef MT_FTL_PARAM_LOCK_SUPPORT
#define MT_FTL_PARAM_LOCK(dev)		spin_lock(&dev->param_lock)
#define MT_FTL_PARAM_UNLOCK(dev)	spin_unlock(&dev->param_lock)
#else
#define MT_FTL_PARAM_LOCK(dev)
#define MT_FTL_PARAM_UNLOCK(dev)
#endif

#define PMT_LEB_PAGE_INDICATOR_SET_BLOCKPAGE(p, blk, page) \
		((p) = (((blk) << PMT_LEB_PAGE_INDICATOR_PAGE_SHIFT) | (page)))

#define PMT_LEB_PAGE_INDICATOR_GET_BLOCK(p)	((p) >> PMT_LEB_PAGE_INDICATOR_PAGE_SHIFT)
#define PMT_LEB_PAGE_INDICATOR_GET_PAGE(p)	((p) & ((1 << PMT_LEB_PAGE_INDICATOR_PAGE_SHIFT) - 1))



#define PMT_INDICATOR_SET_BLOCKPAGE(p, blk, page, dirty, cache_num) \
		((p) = (((blk) << \
		(PMT_INDICATOR_PAGE_SHIFT + PMT_INDICATOR_DIRTY_SHIFT + PMT_INDICATOR_CACHED_SHIFT)) \
		| ((page) << (PMT_INDICATOR_DIRTY_SHIFT + PMT_INDICATOR_CACHED_SHIFT)) | \
		((dirty) << PMT_INDICATOR_CACHED_SHIFT) | ((cache_num) + 1)))

#define PMT_INDICATOR_GET_BLOCK(p)	((p) >> (PMT_INDICATOR_PAGE_SHIFT + PMT_INDICATOR_DIRTY_SHIFT \
					+ PMT_INDICATOR_CACHED_SHIFT))

#define PMT_INDICATOR_GET_PAGE(p)	(((p) >> (PMT_INDICATOR_DIRTY_SHIFT + PMT_INDICATOR_CACHED_SHIFT)) \
					& ((1 << PMT_INDICATOR_PAGE_SHIFT) - 1))

#define PMT_INDICATOR_IS_INCACHE(p)			((p) & ((1 << PMT_INDICATOR_CACHED_SHIFT) - 1))
#define PMT_INDICATOR_CACHE_BUF_NUM(p)			(((p) & ((1 << PMT_INDICATOR_CACHED_SHIFT) - 1)) - 1)
#define PMT_INDICATOR_IS_DIRTY(p)			(((p) & (1 << PMT_INDICATOR_CACHED_SHIFT)) \
								>> PMT_INDICATOR_CACHED_SHIFT)
#define PMT_INDICATOR_SET_CACHE_BUF_NUM(p, num)		((p) = (((p) & (~((1 << PMT_INDICATOR_CACHED_SHIFT) - 1))) \
							| ((num) + 1)))
#define PMT_INDICATOR_SET_DIRTY(p)	((p) |= (1 << PMT_INDICATOR_CACHED_SHIFT))
#define PMT_INDICATOR_RESET_INCACHE(p)	((p) &= (~((1 << PMT_INDICATOR_CACHED_SHIFT) - 1)))
#define PMT_INDICATOR_RESET_DIRTY(p)	((p) &= (~(1 << PMT_INDICATOR_CACHED_SHIFT)))

#define PMT_SET_BLOCKPAGE(p, blk, page) \
		((p) = (((blk) << PMT_PAGE_SHIFT) | (page)))

#define META_PMT_SET_DATA(p, data_size, part, cache_num) \
		((p) = (((data_size) << (PMT_PART_SHIFT + PMT_DATACACHE_BUFF_NUM_SHIFT)) \
			| ((part) << PMT_DATACACHE_BUFF_NUM_SHIFT) \
			| ((cache_num) + 1)))

#define PMT_GET_BLOCK(p)	((p) >> PMT_PAGE_SHIFT)
#define PMT_GET_PAGE(p)		((p) & ((1 << PMT_PAGE_SHIFT) - 1))
#define PMT_GET_DATASIZE(p)	((p) >> (PMT_PAGE_SHIFT + PMT_DATACACHE_BUFF_NUM_SHIFT))
#define PMT_GET_PART(p)		(((p) >> PMT_DATACACHE_BUFF_NUM_SHIFT) & ((1 << PMT_PART_SHIFT) - 1))
#define PMT_GET_DATACACHENUM(p)	(((p) & ((1 << PMT_DATACACHE_BUFF_NUM_SHIFT) - 1)) - 1)
#define PMT_SET_DATACACHE_BUF_NUM(p, num)	((p) = (((p) & (~((1 << PMT_DATACACHE_BUFF_NUM_SHIFT) - 1))) \
						| ((num) + 1)))
#define PMT_IS_DATA_INCACHE(p)	((p) & ((1 << PMT_DATACACHE_BUFF_NUM_SHIFT) - 1))
#define PMT_RESET_DATA_INCACHE(p)	((p) &= (~((1 << PMT_DATACACHE_BUFF_NUM_SHIFT) - 1)))

#define BIT_UPDATE(p, size)			((p) += (size))
#define PAGE_GET_DATA_NUM(p)			((p) & 0x7FFFFFFF)
#define PAGE_BEEN_READ(p)			((p) & 0x80000000)
#define PAGE_SET_READ(p)			((p) |= 0x80000000)

struct mt_ftl_data_header {
	sector_t sector;
	unsigned int offset_len;	/* offset(2bytes):length(2bytes) */
};

struct mt_ftl_gc_info {
	int src_leb;
	int page_src;
	int dst_leb;
	int page_dst;
	int offset_src;
	int offset_dst;
};

struct mt_ftl_valid_data {
	struct mt_ftl_gc_info info;
	int valid_data_num;
	int valid_data_offset;
	int valid_header_offset;
	char *valid_buffer;
};

struct mt_ftl_param {

	/* Indicate next page used data for up page. */
	/* MSB(1) is set for data in cache or not(0) discard */
	unsigned int u4NextPageOffsetIndicator;  /* 4 bytes */

	/* Indicate next used replay page. */
	unsigned int u4NextReplayOffsetIndicator;  /* 4 bytes */

	/* Indicate next used leb/page. leb(20bits):Page(12bits) */
	unsigned int u4NextLebPageIndicator;  /* 4 bytes */

	/* Indicate current used leb/page for PMT indicator. leb(20bits):Page(12bits) */
	/* Leb From 3 to PMT_BLOCK_NUM + 3 - 1 */
	unsigned int u4CurrentPMTLebPageIndicator;  /* 4 bytes */

	/* Indicate next free Leb or invalid number for no free leb.
	 * It is at most desc->vol->ubi->volumes[0]->reserved_pebs - 1 */	/* TODO: volume number need to change */
	unsigned int u4NextFreeLebIndicator;  /* 4 bytes */
	/* Leb From 3 to PMT_BLOCK_NUM + 3 - 1 */
	unsigned int u4NextFreePMTLebIndicator;  /* 4 bytes */

	/* Reserved leb for Garbage Collection */
	unsigned int u4GCReserveLeb; /* 4 bytes */
	/* Leb From 3 to PMT_BLOCK_NUM + 3 - 1 */
	unsigned int u4GCReservePMTLeb; /* 4 bytes */

	/* Compressor handler */
	struct crypto_comp *cc;

	/* Page Mapping Table Indicator, used to indicate PMT position in NAND.
	 * (Block_num(18bits):Page_num(10bits):Dirty(1bit):CachedBuffer(3bits))*/
	unsigned int *u4PMTIndicator;   /* 2K bytes */
	/* TODO: need to adjust and consider to use information from ubi */

	/* Page Mapping Table in cache. */
	/* Block_num(20bits):Page_num(12bits) */
	unsigned int *u4PMTCache;   /* 16K * 4 */
	/* Data_size(16bits):Part_num(12bits):CachedBuffer(4bits) */
	unsigned int *u4MetaPMTCache;   /* 16K * 4 */
	int *i4CurrentPMTClusterInCache;
	unsigned int *u4ReadPMTCache;   /* 16K */
	unsigned int *u4ReadMetaPMTCache;   /* 16K */
	int i4CurrentReadPMTClusterInCache;

	/* Block Invalid Table. Store invalid page amount of a block
	 * Unit of content: bytes */
	unsigned int *u4BIT;   /* 8K */

	/* Block Data Cache Buffer, collect data and store to NAND flash when it is full */
	unsigned char *u1DataCache;	/* 16K */

	/* Data information, including address (4bytes), page offset (2bytes) and data length (2bytes) */
	struct mt_ftl_data_header *u4Header;
	struct mt_ftl_data_header *u4ReadHeader;
	unsigned int u4DataNum;

	/* Replay Block Record */
	unsigned int *replay_blk_rec;
	int replay_blk_index;

	/* Page buffers */
	unsigned int *general_page_buffer;	/* 16K */
	unsigned int *replay_page_buffer;	/* 16K */
	unsigned int *commit_page_buffer;	/* 16K */
	unsigned int *gc_page_buffer;	/* 16K */
	unsigned char *cmpr_page_buffer;	/* 16K */
	unsigned int *tmp_page_buffer;	/* 16K */
};

#if 0 /* TO DO*/
struct mt_ftl_blk_list {
	struct list_head free_list;
	int lnum;
};
#endif

struct mt_ftl_blk {
	struct ubi_volume_desc *desc;
	struct mt_ftl_param *param;
	int ubi_num;
	int vol_id;
	int refcnt;
	int leb_size;
	int min_io_size;
	int pm_per_io;
	int dev_blocks;

	struct gendisk *gd;
	struct request_queue *rq;

	struct workqueue_struct *wq;
	struct work_struct work;

	struct mutex dev_mutex;
	spinlock_t queue_lock;
	struct list_head list;

	enum { STATE_EMPTY, STATE_CLEAN, STATE_DIRTY } cache_state;
	void *cache;
	int cache_leb_num;
	int sync;
	int flush;
#if 0 /* TO DO*/
	struct task_struct *bgt_thread;
	struct list_head blk_list;
	spinlock_t blk_lock;
	int thread_enabled;
	int free_count;
#endif
#ifdef MT_FTL_PARAM_LOCK_SUPPORT
	spinlock_t param_lock;
#endif
};

enum mt_replay_state {
	REPLAY_EMPTY,
	REPLAY_LAST_BLK,
	REPLAY_FREE_BLK,
	REPLAY_CLEAN_BLK,
	REPLAY_END
};


enum {
	MT_FTL_PROFILE_WRITE_ALL,
	MT_FTL_PROFILE_WRITE_COPYTOCACHE,
	MT_FTL_PROFILE_WRITE_UPDATEPMT,
	MT_FTL_PROFILE_UPDATE_PMT_MODIFYPMT,
	MT_FTL_PROFILE_UPDATE_PMT_FINDCACHE_COMMITPMT,
	MT_FTL_PROFILE_COMMIT_PMT,
	MT_FTL_PROFILE_UPDATE_PMT_DOWNLOADPMT,
	MT_FTL_PROFILE_WRITE_COMPRESS,
	MT_FTL_PROFILE_WRITE_WRITEPAGE,
	MT_FTL_PROFILE_WRITE_PAGE_WRITEOOB,
	MT_FTL_PROFILE_WRITE_PAGE_GETFREEBLK,
	MT_FTL_PROFILE_GETFREEBLOCK_GETLEB,
	MT_FTL_PROFILE_GC_FINDBLK,
	MT_FTL_PROFILE_GC_CPVALID,
	MT_FTL_PROFILE_GC_DATA_READOOB,
	MT_FTL_PROFILE_GC_DATA_READ_UPDATE_PMT,
	MT_FTL_PROFILE_GC_DATA_WRITEOOB,
	MT_FTL_PROFILE_GC_REMAP,
	MT_FTL_PROFILE_GETFREEBLOCK_PUTREPLAY_COMMIT,
	MT_FTL_PROFILE_COMMIT,
	MT_FTL_PROFILE_WRITE_PAGE_RESET,
	MT_FTL_PROFILE_READ_ALL,
	MT_FTL_PROFILE_READ_GETPMT,
	MT_FTL_PROFILE_READ_DATATOCACHE,
	MT_FTL_PROFILE_READ_DATATOCACHE_TEST1,
	MT_FTL_PROFILE_READ_DATATOCACHE_TEST2,
	MT_FTL_PROFILE_READ_DATATOCACHE_TEST3,
	MT_FTL_PROFILE_READ_ADDRNOMATCH,
	MT_FTL_PROFILE_READ_DECOMP,
	MT_FTL_PROFILE_READ_COPYTOBUFF,
	MT_FTL_PROFILE_TOTAL_PROFILE_NUM
};

#ifdef MT_FTL_PROFILE
static char *mtk_ftl_profile_message[MT_FTL_PROFILE_TOTAL_PROFILE_NUM] = {
	"MT_FTL_PROFILE_WRITE_ALL",
	"MT_FTL_PROFILE_WRITE_COPYTOCACHE",
	"MT_FTL_PROFILE_WRITE_UPDATEPMT",
	"MT_FTL_PROFILE_UPDATE_PMT_MODIFYPMT",
	"MT_FTL_PROFILE_UPDATE_PMT_FINDCACHE_COMMITPMT",
	"MT_FTL_PROFILE_COMMIT_PMT",
	"MT_FTL_PROFILE_UPDATE_PMT_DOWNLOADPMT",
	"MT_FTL_PROFILE_WRITE_COMPRESS",
	"MT_FTL_PROFILE_WRITE_WRITEPAGE",
	"MT_FTL_PROFILE_WRITE_PAGE_WRITEOOB",
	"MT_FTL_PROFILE_WRITE_PAGE_GETFREEBLK",
	"MT_FTL_PROFILE_GETFREEBLOCK_GETLEB",
	"MT_FTL_PROFILE_GC_FINDBLK",
	"MT_FTL_PROFILE_GC_CPVALID",
	"MT_FTL_PROFILE_GC_DATA_READOOB",
	"MT_FTL_PROFILE_GC_DATA_READ_UPDATE_PMT",
	"MT_FTL_PROFILE_GC_DATA_WRITEOOB",
	"MT_FTL_PROFILE_GC_REMAP",
	"MT_FTL_PROFILE_GETFREEBLOCK_PUTREPLAY_COMMIT",
	"MT_FTL_PROFILE_COMMIT",
	"MT_FTL_PROFILE_WRITE_PAGE_RESET",
	"MT_FTL_PROFILE_READ_ALL",
	"MT_FTL_PROFILE_READ_GETPMT",
	"MT_FTL_PROFILE_READ_DATATOCACHE",
	"MT_FTL_PROFILE_READ_DATATOCACHE_TEST1",
	"MT_FTL_PROFILE_READ_DATATOCACHE_TEST2",
	"MT_FTL_PROFILE_READ_DATATOCACHE_TEST3",
	"MT_FTL_PROFILE_READ_ADDRNOMATCH",
	"MT_FTL_PROFILE_READ_DECOMP",
	"MT_FTL_PROFILE_READ_COPYTOBUFF",
};
#endif	/* PROFILE */


int mt_ftl_create(struct mt_ftl_blk *dev);
int mt_ftl_remove(struct mt_ftl_blk *dev);
int mt_ftl_write(struct mt_ftl_blk *dev, char *buffer, sector_t address, int len);
int mt_ftl_read(struct mt_ftl_blk *dev, char *buffer, sector_t address, int len);
int mt_ftl_commit(struct mt_ftl_blk *dev);

int mt_ftl_blk_create(struct ubi_volume_desc *desc);
int mt_ftl_blk_remove(struct ubi_volume_info *vi);
int mt_ftl_discard(struct mt_ftl_blk *dev, unsigned long sector, unsigned nr_sects);


int mt_ftl_updatePMT(struct mt_ftl_blk *dev, u32 cluster, int sec_offset, int leb, int offset, int part,
		u32 cmpr_data_size, bool replay, bool commit);
int mt_ftl_commit_indicators(struct mt_ftl_blk *dev);
int mt_ftl_commitPMT(struct mt_ftl_blk *dev, bool replay, bool commit);
int mt_ftl_commit(struct mt_ftl_blk *dev);	/* Temporary */

#endif /* !__LINUX_MTFTL_H__ */
