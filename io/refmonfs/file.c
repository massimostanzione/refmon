#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/timekeeping.h>
#include <linux/time.h>
#include <linux/buffer_head.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uio.h>

#include "singlefilefs.h"
#include "../../misc/fsutils.h"
#include "../files.h"


ssize_t onefilefs_read(struct file * filp, char __user * buf, size_t len, loff_t * off) {

    struct buffer_head *bh = NULL;
    struct inode * the_inode = filp->f_inode;
    uint64_t file_size = the_inode->i_size;
    int ret;
    loff_t offset;
    int block_to_read;//index of the block to be read from device

    printk("%s: read operation called with len %ld - and offset %lld (the current file size is %lld)",REFMONFS_MODNAME, len, *off, file_size);

    //this operation is not synchronized 
    //*off can be changed concurrently 
    //add synchronization if you need it for any reason

    //check that *off is within boundaries
    if (*off >= file_size)
        return 0;
    else if (*off + len > file_size)
        len = file_size - *off;

    //determine the block level offset for the operation
    offset = *off % DEFAULT_BLOCK_SIZE; 
    //just read stuff in a single block - residuals will be managed at the applicatin level
    if (offset + len > DEFAULT_BLOCK_SIZE)
        len = DEFAULT_BLOCK_SIZE - offset;

    //compute the actual index of the the block to be read from device
    block_to_read = *off / DEFAULT_BLOCK_SIZE + DEFAULT_INO_SIZE; //the value 2 accounts for superblock and file-inode on device
    
    printk("%s: read operation must access block %d of the device",REFMONFS_MODNAME, block_to_read);

    bh = (struct buffer_head *)sb_bread(filp->f_path.dentry->d_inode->i_sb, block_to_read);
    if(!bh){
	return -EIO;
    }
    ret = copy_to_user(buf,bh->b_data + offset, len);
    *off += (len - ret);
    brelse(bh);

    return len - ret;

}

/**
 * Notice: the signature is compliant with the ".write_iter" operation handle
 * instead of the ".write" one that was suggested in the running examples, see
 * the struct declaration below for further details.
 */
static ssize_t onefilefs_write_async(struct kiocb *iocb, struct iov_iter *iter)
{
    struct buffer_head *bh = NULL;
    struct inode *the_inode = iocb->ki_filp->f_inode;
    uint64_t file_size = the_inode->i_size;
    int block_to_write;
    loff_t offset;
    size_t out_size, copied;
    char *buf;
    int ret = 0;

    printk("%s: write (async) operation called with count %zu - and offset %lld (the current file size is %lld)",
           REFMONFS_MODNAME, iov_iter_count(iter), iocb->ki_pos, file_size);


    offset = file_size;
    out_size = iov_iter_count(iter);

    out_size += 1;

    loff_t block_offset = offset % DEFAULT_BLOCK_SIZE;

    block_to_write = offset / DEFAULT_BLOCK_SIZE + DEFAULT_INO_SIZE;

    printk("%s: (async-)write operation must access block %d of the device",
           REFMONFS_MODNAME, block_to_write);

    bh = sb_bread(iocb->ki_filp->f_path.dentry->d_inode->i_sb, block_to_write);
    if (bh == NULL) {
        pr_err("%s: error while trying to read buffer head for the indexed block",
               REFMONFS_MODNAME);
        return -EIO;
    }

    buf = kmalloc(out_size, GFP_KERNEL);
    if (buf == NULL) {
        pr_err("%s: error while trying to kmalloc", REFMONFS_MODNAME);
        brelse(bh);
        return -ENOMEM;
    }

    copied = copy_from_iter(buf, out_size - 1, iter);
    if (copied != out_size - 1) {
        pr_err("%s: failed to copy %zu bytes from iov_iter\n", REFMONFS_MODNAME, out_size - 1);
        kfree(buf);
        brelse(bh);
        return -EFAULT;
    }

    buf[out_size - 1] = '\n';

    memcpy(bh->b_data + block_offset, buf, out_size);
    mark_buffer_dirty(bh);

    if (offset + out_size > file_size)
        i_size_write(the_inode, offset + out_size);

    iocb->ki_pos = offset + out_size;

    brelse(bh);
    kfree(buf);
    return out_size;
}


struct dentry *onefilefs_lookup(struct inode *parent_inode, struct dentry *child_dentry, unsigned int flags) {

    struct onefilefs_inode *FS_specific_inode;
    struct super_block *sb = parent_inode->i_sb;
    struct buffer_head *bh = NULL;
    struct inode *the_inode = NULL;

    printk("%s: running the lookup inode-function for name %s",REFMONFS_MODNAME,child_dentry->d_name.name);

    if(!strcmp(child_dentry->d_name.name, REFMON_FILE_LOG)){

	
	//get a locked inode from the cache 
        the_inode = iget_locked(sb, 1);
        if (!the_inode)
       		 return ERR_PTR(-ENOMEM);

	//already cached inode - simply return successfully
	if(!(the_inode->i_state & I_NEW)){
		return child_dentry;
	}


	//this work is done if the inode was not already cached
	inode_init_owner(the_inode, NULL, S_IFREG );
	the_inode->i_mode = S_IFREG | S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP | S_IXUSR | S_IXGRP | S_IXOTH;
        the_inode->i_fop = &onefilefs_file_operations;
	the_inode->i_op = &onefilefs_inode_ops;

	//just one link for this file
	set_nlink(the_inode,1);

	//now we retrieve the file size via the FS specific inode, putting it into the generic inode
    	bh = (struct buffer_head *)sb_bread(sb, SINGLEFILEFS_INODES_BLOCK_NUMBER );
    	if(!bh){
		iput(the_inode);
		return ERR_PTR(-EIO);
    	}
	FS_specific_inode = (struct onefilefs_inode*)bh->b_data;
	the_inode->i_size = FS_specific_inode->file_size;
        brelse(bh);

        d_add(child_dentry, the_inode);
	dget(child_dentry);

	//unlock the inode to make it usable 
    	unlock_new_inode(the_inode);

	return child_dentry;
    }

    return NULL;

}

//look up goes in the inode operations
const struct inode_operations onefilefs_inode_ops = {
    .lookup = onefilefs_lookup,
};

// Notice: there is no need to declare differences between LK versions,
//       unlike what is valid for procfiles (see procio.c)
const struct file_operations onefilefs_file_operations = {
    .owner = THIS_MODULE,
    .read = onefilefs_read,

    /**
     * NOTICE: write is implemented as ".write_iter" instead of ".write" as
     * initially suggested in the running example, in order to make it
     * compliant with the asynchronous nature of the deferred work
     */
    .write_iter = onefilefs_write_async,
};
