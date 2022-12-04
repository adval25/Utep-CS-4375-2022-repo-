/*

  MyFS: a tiny file-system written for educational purposes

  MyFS is 

  Copyright 2018 by

  University of Alaska Anchorage, College of Engineering.

  Contributors: Christoph Lauter
                Chandra Boyle
                Devin Boyle and
                Derek Crain

  and based on 

  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall myfs.c implementation.c `pkg-config fuse --cflags --libs` -o myfs

*/

/* The filesystem you implement must support all the 13 operations
   stubbed out below. There need no be support for access rights,
   links, symbolic links. There needs to be support for access and
   modification times and information for statfs.

   The filesystem must run in memory, using the memory of size 
   fssize pointed to by fsptr. The memory comes from mmap and 
   is backed with a file if a backup-file is indicated. When
   the filesystem is unmounted, the memory is written back to 
   that backup-file. When the filesystem is mounted again from
   the backup-file, the same memory appears at the newly mapped
   in virtual address. The filesystem datastructures hence must not
   store any pointer directly to the memory pointed to by fsptr; it
   must rather store offsets from the beginning of the memory region.

   When a filesystem is mounted for the first time, the whole memory
   region of size fssize pointed to by fsptr reads as zero-bytes. When
   a backup-file is used and the filesystem is mounted again, certain
   parts of the memory, which have previously been written, may read
   as non-zero bytes. The size of the memory region is at least 2048
   bytes.

   CAUTION:

   * You MUST NOT use any global variables in your program for reasons
   due to the way FUSE is designed.

   You can find ways to store a structure containing all "global" data
   at the start of the memory region representing the filesystem.

   * You MUST NOT store (the value of) pointers into the memory region
   that represents the filesystem. Pointers are virtual memory
   addresses and these addresses are ephemeral. Everything will seem
   okay UNTIL you remount the filesystem again.

   You may store offsets/indices (of type size_t) into the
   filesystem. These offsets/indices are like pointers: instead of
   storing the pointer, you store how far it is away from the start of
   the memory region. You may want to define a type for your offsets
   and to write two functions that can convert from pointers to
   offsets and vice versa.

   * You may use any function out of libc for your filesystem,
   including (but not limited to) malloc, calloc, free, strdup,
   strlen, strncpy, strchr, strrchr, memset, memcpy. However, your
   filesystem MUST NOT depend on memory outside of the filesystem
   memory region. Only this part of the virtual memory address space
   gets saved into the backup-file. As a matter of course, your FUSE
   process, which implements the filesystem, MUST NOT leak memory: be
   careful in particular not to leak tiny amounts of memory that
   accumulate over time. In a working setup, a FUSE process is
   supposed to run for a long time!

   It is possible to check for memory leaks by running the FUSE
   process inside valgrind:

   valgrind --leak-check=full ./myfs --backupfile=test.myfs ~/fuse-mnt/ -f

   However, the analysis of the leak indications displayed by valgrind
   is difficult as libfuse contains some small memory leaks (which do
   not accumulate over time). We cannot (easily) fix these memory
   leaks inside libfuse.

   * Avoid putting debug messages into the code. You may use fprintf
   for debugging purposes but they should all go away in the final
   version of the code. Using gdb is more professional, though.

   * You MUST NOT fail with exit(1) in case of an error. All the
   functions you have to implement have ways to indicated failure
   cases. Use these, mapping your internal errors intelligently onto
   the POSIX error conditions.

   * And of course: your code MUST NOT SEGFAULT!

   It is reasonable to proceed in the following order:

   (1)   Design and implement a mechanism that initializes a filesystem
         whenever the memory space is fresh. That mechanism can be
         implemented in the form of a filesystem handle into which the
         filesystem raw memory pointer and sizes are translated.
         Check that the filesystem does not get reinitialized at mount
         time if you initialized it once and unmounted it but that all
         pieces of information (in the handle) get read back correctly
         from the backup-file. 

   (2)   Design and implement functions to find and allocate free memory
         regions inside the filesystem memory space. There need to be 
         functions to free these regions again, too. Any "global" variable
         goes into the handle structure the mechanism designed at step (1) 
         provides.

   (3)   Carefully design a data structure able to represent all the
         pieces of information that are needed for files and
         (sub-)directories.  You need to store the location of the
         root directory in a "global" variable that, again, goes into the 
         handle designed at step (1).
          
   (4)   Write __myfs_getattr_implem and debug it thoroughly, as best as
         you can with a filesystem that is reduced to one
         function. Writing this function will make you write helper
         functions to traverse paths, following the appropriate
         subdirectories inside the file system. Strive for modularity for
         these filesystem traversal functions.

   (5)   Design and implement __myfs_readdir_implem. You cannot test it
         besides by listing your root directory with ls -la and looking
         at the date of last access/modification of the directory (.). 
         Be sure to understand the signature of that function and use
         caution not to provoke segfaults nor to leak memory.

   (6)   Design and implement __myfs_mknod_implem. You can now touch files 
         with 

         touch foo

         and check that they start to exist (with the appropriate
         access/modification times) with ls -la.

   (7)   Design and implement __myfs_mkdir_implem. Test as above.

   (8)   Design and implement __myfs_truncate_implem. You can now 
         create files filled with zeros:

         truncate -s 1024 foo

   (9)   Design and implement __myfs_statfs_implem. Test by running
         df before and after the truncation of a file to various lengths. 
         The free "disk" space must change accordingly.

   (10)  Design, implement and test __myfs_utimens_implem. You can now 
         touch files at different dates (in the past, in the future).

   (11)  Design and implement __myfs_open_implem. The function can 
         only be tested once __myfs_read_implem and __myfs_write_implem are
         implemented.

   (12)  Design, implement and test __myfs_read_implem and
         __myfs_write_implem. You can now write to files and read the data 
         back:

         echo "Hello world" > foo
         echo "Hallo ihr da" >> foo
         cat foo

         Be sure to test the case when you unmount and remount the
         filesystem: the files must still be there, contain the same
         information and have the same access and/or modification
         times.

   (13)  Design, implement and test __myfs_unlink_implem. You can now
         remove files.

   (14)  Design, implement and test __myfs_unlink_implem. You can now
         remove directories.

   (15)  Design, implement and test __myfs_rename_implem. This function
         is extremely complicated to implement. Be sure to cover all 
         cases that are documented in man 2 rename. The case when the 
         new path exists already is really hard to implement. Be sure to 
         never leave the filessystem in a bad state! Test thoroughly 
         using mv on (filled and empty) directories and files onto 
         inexistant and already existing directories and files.

   (16)  Design, implement and test any function that your instructor
         might have left out from this list. There are 13 functions 
         __myfs_XXX_implem you have to write.

   (17)  Go over all functions again, testing them one-by-one, trying
         to exercise all special conditions (error conditions): set
         breakpoints in gdb and use a sequence of bash commands inside
         your mounted filesystem to trigger these special cases. Be
         sure to cover all funny cases that arise when the filesystem
         is full but files are supposed to get written to or truncated
         to longer length. There must not be any segfault; the user
         space program using your filesystem just has to report an
         error. Also be sure to unmount and remount your filesystem,
         in order to be sure that it contents do not change by
         unmounting and remounting. Try to mount two of your
         filesystems at different places and copy and move (rename!)
         (heavy) files (your favorite movie or song, an image of a cat
         etc.) from one mount-point to the other. None of the two FUSE
         processes must provoke errors. Find ways to test the case
         when files have holes as the process that wrote them seeked
         beyond the end of the file several times. Your filesystem must
         support these operations at least by making the holes explicit 
         zeros (use dd to test this aspect).

   (18)  Run some heavy testing: copy your favorite movie into your
         filesystem and try to watch it out of the filesystem.

*/

#include "myfs_helper.h"

sz_blk blkalloc(void *fsptr, sz_blk count, blkset *buf)
{
	fsheader *fshead=fsptr;
	blkset freeoff=fshead->freelist;
	freereg *prev=NULL;
	sz_blk alloct=0;
	
	while(alloct<count && freeoff!=NULLOFF){
		freereg fhead=*(freereg*)B2P(freeoff);
		sz_blk freeblk=0;
		while(freeblk<(fhead.size) && alloct<count){
			buf[alloct++]=freeoff+freeblk++;
		}memset(B2P(freeoff),0,freeblk*BLKSZ);
		if(freeblk==(fhead.size)){
			freeoff=fhead.next;
			if(prev!=NULL) prev->next=fhead.next;
			else fshead->freelist=fhead.next;
		}else{
			fhead.size-=freeblk;
			freeoff+=freeblk;
			if(prev!=NULL) prev->next=freeoff;
			else fshead->freelist=freeoff;
			prev=(freereg*)B2P(freeoff);
			*prev=fhead;
		}
	}fshead->free-=alloct;
	return alloct;
}

void swap(blkset *A, blkset *B){ blkset t=*A; *A=*B; *B=t; }
void filter(blkset *heap, size_t dex, size_t len)
{
	size_t ch0=2*dex+1,ch1=2*(dex+1);
	
	if(ch0<len){
		if(heap[ch0]>heap[dex] && (ch1>=len || heap[ch0]>=heap[ch1])){
			swap(&heap[dex],&heap[ch0]);
			filter(heap,ch0,len);
		}else if(ch1<len && heap[ch1]>heap[dex]){
			swap(&heap[dex],&heap[ch1]);
			filter(heap,ch1,len);
		}
	}
}
void offsort(blkset *data,size_t len)
{
	size_t i=len;
	
	while(i) filter(data,--i,len);
	while(len){
		swap(data,&data[--len]);
		filter(data,0,len);
	}
}
sz_blk blkfree(void *fsptr, sz_blk count, blkset *buf)
{
	fsheader *fshead=fsptr;
	blkset freeoff=fshead->freelist;
	freereg *fhead;
	sz_blk freect=0;
	
	offsort(buf,count);
	while(freect<count && *buf<(fshead->ntsize)){
		*(buf++)=NULLOFF; count--;
	}if(freect<count && ((freeoff==NULLOFF && *buf<fshead->size) || *buf<freeoff)){
		fhead=(freereg*)B2P(freeoff=*buf);
		fhead->next=fshead->freelist;
		fshead->freelist=freeoff;
		if((freeoff+(fhead->size=1))==fhead->next){
			freereg *tmp=B2P(fhead->next);
			fhead->size+=tmp->size;
			fhead->next=tmp->next;
		}buf[freect++]=NULLOFF;
	}while(freect<count && buf[freect]<fshead->size){
		fhead=(freereg*)B2P(freeoff);
		if(buf[freect]>=(freeoff+fhead->size)){
			if(fhead->next!=NULLOFF && buf[freect]>=fhead->next){
				freeoff=fhead->next;
				continue;
			}if(buf[freect]==(freeoff+fhead->size)){
				fhead->size++;
			}else{
				freereg *tmp=B2P(freeoff=buf[freect]);
				tmp->next=fhead->next;
				tmp->size=1;
				fhead->next=freeoff;
				fhead=tmp;
			}if((freeoff+fhead->size)==fhead->next){
				freereg *tmp=B2P(fhead->next);
				fhead->size+=tmp->size;
				fhead->next=tmp->next;
			}buf[freect++]=NULLOFF;
		}else{
			(buf++)[freect]=NULLOFF; count--;
		}
	}while(freect<count){
		(buf++)[freect]=NULLOFF; count--;
	}fshead->free+=freect;
	return freect;
}

nodei newnode(void *fsptr)
{
	fsheader *fshead=fsptr;
	inode *nodetbl=O2P(fshead->nodetbl);
	size_t nodect=fshead->ntsize*NODES_BLOCK-1;
	nodei i=0;
	while(++i<nodect){
		if(nodetbl[i].nlinks==0 && nodetbl[i].blocks[0]==NULLOFF) return i;
	}return NONODE;
}

int nodevalid(void *fsptr, nodei node)
{
	fsheader *fshead=(fsheader*)fsptr;
	inode *nodetbl=O2P(fshead->nodetbl);
	
	if(node<0 || node>=(fshead->ntsize*NODES_BLOCK-1)) return NODEI_BAD;
	if(nodetbl[node].nlinks==0 ||(nodetbl[node].mode!=DIRMODE && nodetbl[node].mode!=FILEMODE)) return NODEI_GOOD;
	return NODEI_LINKD;
}

void loadpos(void *fsptr, fpos *pos, nodei node)
{
	fsheader *fshead=fsptr;
	inode *nodetbl=O2P(fshead->nodetbl);
	
	if(pos==NULL) return;
	if(nodevalid(fsptr,node)<NODEI_GOOD){
		pos->node=NONODE;
		return;
	}pos->node=node;
	pos->nblk=0;
	pos->opos=0;
	pos->dpos=0;
	pos->oblk=NULLOFF;
	pos->dblk=nodetbl[node].blocks[0];
	pos->data=pos->dblk*BLKSZ;
}

sz_blk advance(void *fsptr, fpos *pos, sz_blk blks)
{
	fsheader *fshead=fsptr;
	inode *nodetbl=O2P(fshead->nodetbl);
	sz_blk adv=0;
	size_t unit=1;
	
	if(pos==NULL || pos->node==NONODE || pos->dblk==NULLOFF) return 0;
	if(nodetbl[pos->node].mode==DIRMODE) unit=sizeof(direntry);
	
	if(pos->data==NULLOFF){
		if(pos->dpos*unit==BLKSZ) pos->opos--;
	}pos->dpos=0;
	while(blks){
		blkdex opos=pos->opos+1;
		if(pos->oblk==NULLOFF){
			if(opos==OFFS_NODE){
				if((pos->oblk=nodetbl[pos->node].blocklist)==NULLOFF) break;
				offblock *offs=B2P(pos->oblk);
				pos->dblk=offs->blocks[opos=0];
			}else{
				if(nodetbl[pos->node].blocks[opos]==NULLOFF) break;
				pos->dblk=nodetbl[pos->node].blocks[opos];
			}
		}else{
			offblock *offs=B2P(pos->oblk);
			if(opos==OFFS_BLOCK){
				if(offs->next==NULLOFF) break;
				pos->oblk=offs->next;
				offs=(offblock*)B2P(pos->oblk);
				pos->dblk=offs->blocks[opos=0];
			}else{
				if(offs->blocks[opos]==NULLOFF) break;
				pos->dblk=offs->blocks[opos];
			}
		}pos->opos=opos;
		adv++; blks--;
	}pos->data=pos->dblk*BLKSZ;
	pos->nblk+=adv;
	return adv;
}

size_t seek(void *fsptr, fpos *pos, size_t off)
{
	fsheader *fshead=fsptr;
	inode *nodetbl=O2P(fshead->nodetbl);
	size_t adv=0, bck=0, unit=1;
	sz_blk blks;
	
	if(pos==NULL || pos->node==NONODE || pos->data==NULLOFF) return 0;
	if(nodetbl[pos->node].mode==DIRMODE) unit=sizeof(direntry);
	
	if((blks=(off+pos->dpos)*unit/BLKSZ)>0){
		off=(off+pos->dpos)%(BLKSZ/unit);
		bck=pos->dpos;
		if((adv=advance(fsptr,pos,blks))<blks){
			off=BLKSZ/unit;
		}adv*=BLKSZ/unit;
	}while(pos->data!=NULLOFF && off>0){
		pos->dpos++;
		if((pos->nblk*BLKSZ/unit+pos->dpos)==nodetbl[pos->node].size){
			if(pos->dpos==BLKSZ/unit) pos->opos++;
			pos->data=NULLOFF;
		}else{
			pos->data=pos->dblk*BLKSZ+pos->dpos*unit;
			adv++; off--;
		}
	}return (adv-bck);
}

int frealloc(void *fsptr, nodei node, size_t size)
{
	fsheader *fshead=fsptr;
	inode *nodetbl=O2P(fshead->nodetbl);
	fpos pos;
	ssize_t blkdiff;
	sz_blk blksize;
	
	loadpos(fsptr,&pos,node);
	if(pos.node==NONODE || nodetbl[pos.node].mode==DIRMODE) return -1;
	
	blksize=CLDIV(size,BLKSZ);
	blkdiff=blksize-nodetbl[node].nblocks;
	if(blkdiff<0){
		sz_blk fct=0,adv=0;
		offblock *offs;
		if(blksize<=OFFS_NODE){
			fct=OFFS_NODE-blksize;
			advance(fsptr,&pos,blksize);
			if(fct!=0){
				adv=advance(fsptr,&pos,fct);
				blkfree(fsptr,fct,&(nodetbl[node].blocks[blksize]));
			}blkfree(fsptr,1,&(nodetbl[node].blocklist));
		}else{
			advance(fsptr,&pos,blksize-1);
			offs=(offblock*)B2P(pos.oblk);
			advance(fsptr,&pos,1);
			if(pos.opos>0){
				fct=OFFS_BLOCK-pos.opos;
				adv=advance(fsptr,&pos,fct);
				blkfree(fsptr,fct,&(offs->blocks[OFFS_BLOCK-fct]));
			}offs->next=NULLOFF;
		}while(adv==fct){
			offs=(offblock*)B2P(pos.oblk);
			blkset prev=pos.oblk;
			fct=OFFS_BLOCK;
			adv=advance(fsptr,&pos,fct);
			blkfree(fsptr,fct,offs->blocks);
			blkfree(fsptr,1,&prev);
		}
	}else if(size>nodetbl[node].size){
		seek(fsptr,&pos,nodetbl[node].size);
		if(pos.dblk!=NULLOFF && pos.dpos<BLKSZ){
			memset(O2P(pos.dblk*BLKSZ+pos.dpos),0,BLKSZ-pos.dpos);
			pos.opos++;
		}if(blkdiff>0){
			offblock *offs;
			blkset *tblks;
			sz_blk noblks,alloct;
			
			if(pos.oblk==NULLOFF){
				noblks=(blkdiff+pos.opos+(OFFS_BLOCK-OFFS_NODE)-1)/OFFS_BLOCK;
			}else{
				noblks=(blkdiff+pos.opos-1)/OFFS_BLOCK;
			}
			
			if((tblks=(blkset*)malloc((blkdiff+noblks)*sizeof(blkset)))==NULL) return -1;
			if(blkalloc(fsptr,blkdiff+noblks,tblks)<(blkdiff+noblks)){
				blkfree(fsptr,blkdiff+noblks,tblks);
				free(tblks);
				return -1;
			}
			
			nodetbl[node].size=nodetbl[node].nblocks*BLKSZ;
			seek(fsptr,&pos,BLKSZ);
			alloct=0;
			while(alloct<(noblks+blkdiff)){
				if(pos.oblk==NULLOFF){
					if(pos.opos==OFFS_NODE){
						nodetbl[node].blocklist=tblks[alloct++];
						offs=(offblock*)B2P(nodetbl[node].blocklist);
						pos.opos=0; pos.oblk=nodetbl[node].blocklist;
						offs->blocks[0]=tblks[alloct];
					}else{
						nodetbl[node].blocks[pos.opos]=tblks[alloct];
					}
				}else{
					offs=(offblock*)B2P(pos.oblk);
					if(pos.opos==OFFS_BLOCK){
						offs->next=tblks[alloct++];
						pos.oblk=offs->next;
						offs=(offblock*)B2P(offs->next);
						pos.opos=0;
					}offs->blocks[pos.opos]=tblks[alloct];
				}alloct++;
				nodetbl[node].size+=BLKSZ;
				nodetbl[node].nblocks++;
				pos.opos++;
			}free(tblks);
		}
	}nodetbl[node].nblocks=blksize;
	nodetbl[node].size=size;
	return 0;
}

void namepathset(char *name, const char *path)
{
	size_t len=0;
	if(name==path) return;
	while(path[len]!='/' && path[len]!='\0'){
		name[len]=path[len];
		if(++len==NAMELEN-1) break;
	}name[len]='\0';
}
int namepatheq(char *name, const char *path)
{
	size_t len=0;
	if(name==path) return 1;
	while(path[len]!='/' && path[len]!='\0'){
		if(name[len]=='\0' || name[len]!=path[len]) return 0;
		if(++len==NAMELEN) return 1;
	}return (name[len]=='\0');
}

nodei dirmod(void *fsptr, nodei dir, const char *name, nodei node, const char *rename)
{
	fsheader *fshead=fsptr;
	inode *nodetbl=O2P(fshead->nodetbl);
	blkset oblk=NULLOFF, prevo=NULLOFF;
	blkset dblk=nodetbl[dir].blocks[0];
	direntry *df, *found=NULL;
	blkdex block=0, entry=0;
	
	if(nodevalid(fsptr,dir)<NODEI_LINKD || nodetbl[dir].mode!=DIRMODE) return NONODE;
	if(node!=NONODE && rename==NULL && nodevalid(fsptr,node)<NODEI_GOOD) return NONODE;
	if(*name=='\0' || (rename!=NULL && node==NONODE && *rename=='\0')) return NONODE;
	
	while(dblk!=NULLOFF){
		df=(direntry*)B2P(dblk);
		while(entry<FILES_DIR){
			if(df[entry].node==NONODE) break;
			if(node==NONODE && rename!=NULL && namepatheq(df[entry].name,rename)){
				return NONODE;
			}if(namepatheq(df[entry].name,name)){
				if(rename!=NULL) found=&df[entry];
				else{
					if(node==NONODE) return df[entry].node;
					else return NONODE;
				}
			}entry++;
		}if(entry<FILES_DIR) break;
		block++; entry=0;
		if(oblk==NULLOFF){
			if(block==OFFS_NODE){
				if((oblk=nodetbl[dir].blocklist)==NULLOFF){
					dblk=NULLOFF;
				}else{
					offblock *offs=B2P(oblk);
					dblk=offs->blocks[block=0];
				}
			}else dblk=nodetbl[dir].blocks[block];
		}else{
			offblock *offs=B2P(oblk);
			if(block==OFFS_BLOCK){
				if(offs->next==NULLOFF) dblk=NULLOFF;
				else{
					prevo=oblk;
					oblk=offs->next;
					offs=(offblock*)B2P(oblk);
					dblk=offs->blocks[block=0];
				}
			}else dblk=offs->blocks[block];
		}
	}if(node==NONODE){
		if(rename!=NULL && found!=NULL){
			namepathset(found->name,rename);
			return found->node;
		}return NONODE;
	}if(rename!=NULL){
		if(found==NULL) return NONODE;
		node=found->node;
		if(nodetbl[node].mode==DIRMODE && nodetbl[node].nlinks==1 && nodetbl[node].size>0) return NONODE;
		if(dblk!=NULLOFF) entry--;
		else{
			if(oblk==NULLOFF){
				dblk=nodetbl[dir].blocks[--block];
			}else{
				offblock *offs=B2P(oblk);
				dblk=offs->blocks[--block];
			}entry=FILES_DIR-1;
		}df=(direntry*)B2P(dblk);
		found->node=df[entry].node;
		namepathset(found->name,df[entry].name);
		df[entry].node=NONODE;
		if(entry==0){
			if(oblk==NULLOFF){
				blkfree(fsptr,1,&(nodetbl[dir].blocks[block]));
			}else{
				offblock *offs=B2P(oblk);
				blkfree(fsptr,1,&(offs->blocks[block]));
				if(block==0){
					if(prevo==NULLOFF){
						blkfree(fsptr,1,&(nodetbl[dir].blocklist));
					}else{
						offs=(offblock*)B2P(prevo);
						blkfree(fsptr,1,&(offs->next));
					}
				}
			}nodetbl[dir].nblocks--;
		}nodetbl[dir].size--;
		//update dir node times?
		nodetbl[node].nlinks--;
		return node;
	}if(dblk==NULLOFF){
		offblock *offs;
		if(oblk==NULLOFF){
			if(block==OFFS_NODE){
				if(blkalloc(fsptr,1,&oblk)==0){
					return NONODE;
				}if(blkalloc(fsptr,1,&dblk)==0){
					blkfree(fsptr,1,&oblk);
					return NONODE;
				}nodetbl[dir].blocklist=oblk;
				offs=(offblock*)B2P(oblk);
				offs->blocks[0]=dblk;
				offs->blocks[1]=NULLOFF;
				offs->next=NULLOFF;
			}else{
				if(blkalloc(fsptr,1,&dblk)==0){
					return NONODE;
				}nodetbl[dir].blocks[block]=dblk;
				if(block<OFFS_NODE-1) nodetbl[dir].blocks[block+1]=NULLOFF;
			}
		}else{
			offs=(offblock*)B2P(oblk);
			if(block==OFFS_BLOCK){
				if(blkalloc(fsptr,1,&oblk)==0){
					return NONODE;
				}if(blkalloc(fsptr,1,&dblk)==0){
					blkfree(fsptr,1,&oblk);
					return NONODE;
				}offs->next=oblk;
				offs=(offblock*)B2P(oblk);
				offs->next=NULLOFF;
				block=0;
			}else{
				if(blkalloc(fsptr,1,&dblk)==0){
					return NONODE;
				}
			}offs->blocks[block]=dblk;
			offs->blocks[1]=NULLOFF;
		}nodetbl[dir].nblocks++;
		df=(direntry*)B2P(dblk);
	}nodetbl[dir].size++;
	df[entry].node=node;
	namepathset(df[entry].name,name);
	nodetbl[node].nlinks++;
	if(++entry<FILES_DIR) df[entry].node=NONODE;
	return node;
}

nodei path2node(void *fsptr, const char *path, const char **child)
{
	nodei node=0;
	size_t sub=1, ch=1;
	
	if(path[0]!='/') return NONODE;
	
	while(path[sub=ch]!='\0'){
		while(path[ch]!='\0'){
			if(path[ch++]=='/') break;
		}if(child!=NULL && path[ch]=='\0'){
			*child=&path[sub];
			break;
		}if((node=dirmod(fsptr,node,&path[sub],NONODE,NULL))==NONODE)
			return NONODE;
	}return node;
}

void fsinit(void *fsptr, size_t fssize)
{
	fsheader *fshead=fsptr;
	freereg *fhead;
	inode *nodetbl;
	struct timespec creation;
	
	if(fshead->size==fssize/BLKSZ) return;
	
	fshead->ntsize=(BLOCKS_FILE*(1+NODES_BLOCK)+fssize/BLKSZ)/(1+BLOCKS_FILE*NODES_BLOCK);
	fshead->nodetbl=sizeof(inode);
	fshead->freelist=fshead->ntsize;
	fshead->free=fssize/BLKSZ-fshead->ntsize;
	
	fhead=(freereg*)B2P(fshead->freelist);
	fhead->size=fshead->free;
	fhead->next=NULLOFF;
	
	nodetbl=(inode*)O2P(fshead->nodetbl);
	memset(nodetbl,0,fshead->ntsize*BLKSZ-sizeof(inode));
	timespec_get(&creation,TIME_UTC);
	nodetbl[0].mode=DIRMODE;
	nodetbl[0].ctime=creation;
	nodetbl[0].mtime=creation;
	nodetbl[0].nlinks=1;
	
	fshead->size=fssize/BLKSZ;
}

/*Implementation Details
	Filesystem layout
		[ global header | root inode | ... inodes ... ] [ node table blocks ]... [ data blocks ]...
	File layout
		node{ first n data offsets }->first offset block{ next m data offsets }->...
	Directory layout
		{ file0[node,name] file1[node,name] ... } ... { file_n[node,name] file_n+1[node,name] ... }
	
	Block sizes were chosen to be 1024 bytes, as this is a common block size, is smaller than the page size, and is big enough to contain most small files
	Names are given a fixed length to reduce complexity, and the length is such that a directory entry is 256 bytes
		when/if a longer name is given, the name will be truncated to the fixed length
	The number of Inodes allocated to the filesystem is calculated so there are at least as many nodes as there
		is space for the number of 4k files that can fit after the node table
	Inodes store the same data for files as for directories, only sizes are interpreted differently,
		and the mode is set appropriately to distinguish between them
	No empty offset, data, or directory blocks are allocated, empty dirs and files of size 0 have 0 blocks
	Free blocks are stored in a linked list and grouped into contiguous regions
	Testing was done similarly to HW3, using a separate file to test helper functions before working with FUSE
	Valgrind was used to check for memory leaks and seemed to find none, though some were reported and appear to
		result from FUSE
	Certain functions were implemented inefficiently near the deadline and perfom noticably slowly under load,
		notably frealloc, as used for truncates
	The helper functions are implemented in the separate file myfs_helper.c, and filesystem types and definintions
		are in myfs_helper.h
	A makefile was made to build the project, with targets default(fuse version), debug(for gdb), and test(fstst.c)
*/

/* FUSE Function Implementations */

/* Implements an emulation of the stat system call on the filesystem 
   of size fssize pointed to by fsptr. 
   
   If path can be followed and describes a file or directory 
   that exists and is accessable, the access information is 
   put into stbuf. 

   On success, 0 is returned. On failure, -1 is returned and 
   the appropriate error code is put into *errnoptr.

   man 2 stat documents all possible error codes and gives more detail
   on what fields of stbuf need to be filled in. Essentially, only the
   following fields need to be supported:

   st_uid      the value passed in argument
   st_gid      the value passed in argument
   st_mode     (as fixed values S_IFDIR | 0755 for directories,
                                S_IFREG | 0755 for files)
   st_nlink    (as many as there are subdirectories (not files) for directories
                (including . and ..),
                1 for files)
   st_size     (supported only for files, where it is the real file size)
   st_atim
   st_mtim

*/
int __myfs_getattr_implem(void *fsptr, size_t fssize, int *errnoptr,
                          uid_t uid, gid_t gid,
                          const char *path, struct stat *stbuf) {
	fsheader *fshead=fsptr;
	inode *nodetbl;
	nodei node;
	size_t unit=1;
	
	fsinit(fsptr,fssize);
	nodetbl=(inode*)O2P(fshead->nodetbl);
	
	if((node=path2node(fsptr,path,NULL))==NONODE){
		*errnoptr=ENOENT;
		return -1;
	}if(nodetbl[node].mode==DIRMODE) unit=sizeof(direntry);
	
	stbuf->st_uid=uid;
	stbuf->st_gid=gid;
	stbuf->st_mode=nodetbl[node].mode;
	stbuf->st_size=nodetbl[node].size*unit;
	stbuf->st_nlink=nodetbl[node].nlinks;
	stbuf->st_atim=nodetbl[node].atime;
	stbuf->st_mtim=nodetbl[node].mtime;
	stbuf->st_ctim=nodetbl[node].ctime;
	return 0;
}

/* Implements an emulation of the readdir system call on the filesystem 
   of size fssize pointed to by fsptr. 

   If path can be followed and describes a directory that exists and
   is accessable, the names of the subdirectories and files 
   contained in that directory are output into *namesptr. The . and ..
   directories must not be included in that listing.

   If it needs to output file and subdirectory names, the function
   starts by allocating (with calloc) an array of pointers to
   characters of the right size (n entries for n names). Sets
   *namesptr to that pointer. It then goes over all entries
   in that array and allocates, for each of them an array of
   characters of the right size (to hold the i-th name, together 
   with the appropriate '\0' terminator). It puts the pointer
   into that i-th array entry and fills the allocated array
   of characters with the appropriate name. The calling function
   will call free on each of the entries of *namesptr and 
   on *namesptr.

   The function returns the number of names that have been 
   put into namesptr. 

   If no name needs to be reported because the directory does
   not contain any file or subdirectory besides . and .., 0 is 
   returned and no allocation takes place.

   On failure, -1 is returned and the *errnoptr is set to 
   the appropriate error code. 

   The error codes are documented in man 2 readdir.

   In the case memory allocation with malloc/calloc fails, failure is
   indicated by returning -1 and setting *errnoptr to EINVAL.

*/
int __myfs_readdir_implem(void *fsptr, size_t fssize, int *errnoptr,
                          const char *path, char ***namesptr) {
	fsheader *fshead=fsptr;
	inode *nodetbl;
	direntry *df;
	nodei dir;
	fpos pos;
	struct timespec access;
	size_t count=0;
	char **namelist;
	
	fsinit(fsptr,fssize);
	nodetbl=O2P(fshead->nodetbl);
	
	if((dir=path2node(fsptr,path,NULL))==NONODE){
		*errnoptr=ENOENT;
		return -1;
	}if(nodetbl[dir].mode!=DIRMODE){
		*errnoptr=ENOTDIR;
		return -1;
	}if((namelist=malloc(nodetbl[dir].size*sizeof(char*)))==NULL){
		*errnoptr=EINVAL;
		return -1;
	}
	
	timespec_get(&access,TIME_UTC);
	nodetbl[dir].atime=access;
	
	loadpos(fsptr,&pos,dir);
	while(pos.data!=NULLOFF){
		df=(direntry*)B2P(pos.dblk);
		if(df[pos.dpos].node==NONODE) break;
		namelist[count]=(char*)malloc(strlen(df[pos.dpos].name));
		if(namelist[count]==NULL){
			while(count) free(namelist[--count]);
			free(namelist);
			*errnoptr=EINVAL;
			return -1;
		}strcpy(namelist[count],df[pos.dpos].name);
		count++;
		seek(fsptr,&pos,1);
	}*namesptr=namelist;
	return count;
}

/* Implements an emulation of the mknod system call for regular files
   on the filesystem of size fssize pointed to by fsptr.

   This function is called only for the creation of regular files.

   If a file gets created, it is of size zero and has default
   ownership and mode bits.

   The call creates the file indicated by path.

   On success, 0 is returned.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The error codes are documented in man 2 mknod.

*/
int __myfs_mknod_implem(void *fsptr, size_t fssize, int *errnoptr, const char *path) {
	fsheader *fshead=fsptr;
	inode *nodetbl;
	nodei pnode, node;
	struct timespec creation;
	const char *fname;
	
	fsinit(fsptr,fssize);
	nodetbl=(inode*)O2P(fshead->nodetbl);
	
	if((pnode=path2node(fsptr,path,&fname))==NONODE){
		*errnoptr=ENOENT;
		return -1;
	}if((node=newnode(fsptr))==NONODE){
		*errnoptr=ENOSPC;
		return -1;
	}if(dirmod(fsptr,pnode,fname,node,NULL)==NONODE){
		*errnoptr=EEXIST;
		return -1;
	}
	
	timespec_get(&creation,TIME_UTC);
	nodetbl[node].mode=FILEMODE;
	nodetbl[node].ctime=creation;
	nodetbl[node].mtime=creation;
	return 0;
}

/* Implements an emulation of the unlink system call for regular files
   on the filesystem of size fssize pointed to by fsptr.

   This function is called only for the deletion of regular files.

   On success, 0 is returned.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The error codes are documented in man 2 unlink.

*/
int __myfs_unlink_implem(void *fsptr, size_t fssize, int *errnoptr, const char *path) {
	fsheader *fshead=fsptr;
	inode *nodetbl;
	nodei pnode, node;
	const char *fname;
	
	fsinit(fsptr,fssize);
	nodetbl=(inode*)O2P(fshead->nodetbl);
	
	if((pnode=path2node(fsptr,path,&fname))==NONODE){
		*errnoptr=ENOENT;
		return -1;
	}if((node=dirmod(fsptr,pnode,fname,0,""))==NONODE){
		*errnoptr=EEXIST;
		return -1;
	}if(nodetbl[node].nlinks==0){
		frealloc(fsptr,node,0);
	}return 0;
}

/* Implements an emulation of the rmdir system call on the filesystem 
   of size fssize pointed to by fsptr. 

   The call deletes the directory indicated by path.

   On success, 0 is returned.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The function call must fail when the directory indicated by path is
   not empty (if there are files or subdirectories other than . and ..).

   The error codes are documented in man 2 rmdir.

*/
int __myfs_rmdir_implem(void *fsptr, size_t fssize, int *errnoptr, const char *path) {
	nodei pnode;
	const char *fname;
	
	fsinit(fsptr,fssize);
	
	if((pnode=path2node(fsptr,path,&fname))==NONODE){
		*errnoptr=ENOENT;
		return -1;
	}if(dirmod(fsptr,pnode,fname,0,"")==NONODE){
		*errnoptr=EEXIST;
		return -1;
	}return 0;
}

/* Implements an emulation of the mkdir system call on the filesystem 
   of size fssize pointed to by fsptr. 

   The call creates the directory indicated by path.

   On success, 0 is returned.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The error codes are documented in man 2 mkdir.

*/
int __myfs_mkdir_implem(void *fsptr, size_t fssize, int *errnoptr, const char *path) {
	fsheader *fshead=fsptr;
	inode *nodetbl;
	struct timespec creation;
	nodei pnode, node;
	const char *fname;
	
	fsinit(fsptr,fssize);
	nodetbl=(inode*)O2P(fshead->nodetbl);

	if((pnode=path2node(fsptr,path,&fname))==NONODE){
		*errnoptr=ENOENT;
		return -1;
	}if((node=newnode(fsptr))==NONODE){
		*errnoptr=ENOSPC;
		return -1;
	}if(dirmod(fsptr,pnode,fname,node,NULL)==NONODE){
		*errnoptr=EEXIST;
		return -1;
	}

	timespec_get(&creation,TIME_UTC);
	nodetbl[node].mode=DIRMODE;
	nodetbl[node].ctime=creation;
	nodetbl[node].mtime=creation;
	return 0;
}

/* Implements an emulation of the rename system call on the filesystem 
   of size fssize pointed to by fsptr. 

   The call moves the file or directory indicated by from to to.

   On success, 0 is returned.

   On failure, -1 is returned and *errnoptr is set appropriately.

   Caution: the function does more than what is hinted to by its name.
   In cases the from and to paths differ, the file is moved out of 
   the from path and added to the to path.

   The error codes are documented in man 2 rename.

*/
int __myfs_rename_implem(void *fsptr, size_t fssize, int *errnoptr,
                         const char *from, const char *to) {
	fsheader *fshead=fsptr;
	inode *nodetbl;
	nodei pfrom, pto, file;
	struct timespec modify;
	const char *ffrom, *fto;
	
	fsinit(fsptr,fssize);
	nodetbl=(inode*)O2P(fshead->nodetbl);
	
	if((pfrom=path2node(fsptr,from,&ffrom))==NONODE){
		*errnoptr=ENOENT;
		return -1;
	}if((pto=path2node(fsptr,to,&fto))==NONODE){
		*errnoptr=ENOENT;
		return -1;
	}if((file=dirmod(fsptr,pfrom,ffrom,NONODE,NULL))==NONODE){
		*errnoptr=ENOENT;
		return -1;
	}
	
	timespec_get(&modify,TIME_UTC);
	nodetbl[file].mtime=modify;
	
	if(pto==pfrom){
		if(dirmod(fsptr,pfrom,ffrom,NONODE,fto)==NONODE){
			*errnoptr=EEXIST;
			return -1;
		}return 0;
	}
	
	if(dirmod(fsptr,pto,fto,file,NULL)==NONODE){
		*errnoptr=EEXIST;
		return -1;
	}if(dirmod(fsptr,pfrom,ffrom,0,"")==NONODE){
		dirmod(fsptr,pto,fto,0,"");
		*errnoptr=EACCES;
		return -1;
	}return 0;
}

/* Implements an emulation of the truncate system call on the filesystem 
   of size fssize pointed to by fsptr. 

   The call changes the size of the file indicated by path to offset
   bytes.

   When the file becomes smaller due to the call, the extending bytes are
   removed. When it becomes larger, zeros are appended.

   On success, 0 is returned.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The error codes are documented in man 2 truncate.

*/
int __myfs_truncate_implem(void *fsptr, size_t fssize, int *errnoptr,
                           const char *path, off_t offset) {
	fsheader *fshead=fsptr;
	inode *nodetbl;
	nodei node;
	struct timespec modify;
	
	fsinit(fsptr,fssize);
	nodetbl=(inode*)O2P(fshead->nodetbl);
	
	if((node=path2node(fsptr,path,NULL))==NONODE){
		*errnoptr=ENOENT;
		return -1;
	}if(nodetbl[node].mode!=FILEMODE){
		*errnoptr=EISDIR;
		return -1;
	}
	
	timespec_get(&modify,TIME_UTC);
	nodetbl[node].mtime=modify;
	
	if(frealloc(fsptr,node,offset)==-1){
		*errnoptr=EPERM;
		return -1;
	}return 0;
}

/* Implements an emulation of the open system call on the filesystem 
   of size fssize pointed to by fsptr, without actually performing the opening
   of the file (no file descriptor is returned).

   The call just checks if the file (or directory) indicated by path
   can be accessed, i.e. if the path can be followed to an existing
   object for which the access rights are granted.

   On success, 0 is returned.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The two only interesting error codes are 

   * EFAULT: the filesystem is in a bad state, we can't do anything

   * ENOENT: the file that we are supposed to open doesn't exist (or a
             subpath).

   It is possible to restrict ourselves to only these two error
   conditions. It is also possible to implement more detailed error
   condition answers.

   The error codes are documented in man 2 open.

*/
int __myfs_open_implem(void *fsptr, size_t fssize, int *errnoptr, const char *path) {
	fsheader *fshead=fsptr;
	inode *nodetbl;
	nodei node;
	struct timespec access;
	
	fsinit(fsptr,fssize);
	nodetbl=(inode*)O2P(fshead->nodetbl);
	
	if((node=path2node(fsptr,path,NULL))==NONODE){
		*errnoptr=ENOENT;
		return -1;
	}
	
	timespec_get(&access,TIME_UTC);
	nodetbl[node].atime=access;
	return 0;
}

/* Implements an emulation of the read system call on the filesystem 
   of size fssize pointed to by fsptr.

   The call copies up to size bytes from the file indicated by 
   path into the buffer, starting to read at offset. See the man page
   for read for the details when offset is beyond the end of the file etc.
   
   On success, the appropriate number of bytes read into the buffer is
   returned. The value zero is returned on an end-of-file condition.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The error codes are documented in man 2 read.

*/
int __myfs_read_implem(void *fsptr, size_t fssize, int *errnoptr,
                       const char *path, char *buf, size_t size, off_t off) {



   fsheader *fshead=fsptr;
   inode *nodetbl;
   nodei node;
   fpos pos;
   size_t readct=0, nblks;
   struct timespec access;
   
   fsinit(fsptr,fssize);
   nodetbl=(inode*)O2P(fshead->nodetbl);
   
   if((node=path2node(fsptr,path,NULL))==NONODE){
      *errnoptr=ENOENT;
      return -1;
   }if(nodetbl[node].mode!=FILEMODE){
      *errnoptr=EISDIR;
      return -1;
   }if(size==0) return 0;

   timespec_get(&access,TIME_UTC);
   nodetbl[node].atime=access;
   
   loadpos(fsptr,&pos,node);
   seek(fsptr,&pos,off);
   while(pos.data!=NULLOFF && readct<size){
      char* blk=B2P(pos.dblk);
      buf[readct++]=blk[pos.dpos];
      seek(fsptr,&pos,1);
   }
   while(readct<size){
      loadpos(fsptr,&pos,node);
      seek(fsptr,&pos,off+readct);
      char* blk=B2P(pos.dblk);
      buf[readct++]=blk[pos.dpos];
      seek(fsptr,&pos,1);
   }




   return readct;

  return -1;
}

/* Implements an emulation of the write system call on the filesystem 
   of size fssize pointed to by fsptr.

   The call copies up to size bytes to the file indicated by 
   path into the buffer, starting to write at offset. See the man page
   for write for the details when offset is beyond the end of the file etc.
   
   On success, the appropriate number of bytes written into the file is
   returned. The value zero is returned on an end-of-file condition.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The error codes are documented in man 2 write.

*/
int __myfs_write_implem(void *fsptr, size_t fssize, int *errnoptr,
                        const char *path, const char *buf, size_t size, off_t off) {
	fsheader *fshead=fsptr;
	inode *nodetbl;
	nodei node;
	fpos pos;
	struct timespec modify;
	size_t writect=0;
	
	fsinit(fsptr,fssize);
	nodetbl=(inode*)O2P(fshead->nodetbl);
	
	if((node=path2node(fsptr,path,NULL))==NONODE){
		*errnoptr=ENOENT;
		return -1;
	}if(nodetbl[node].mode!=FILEMODE){
		*errnoptr=EISDIR;
		return -1;
	}
	
	timespec_get(&modify,TIME_UTC);
	nodetbl[node].mtime=modify;
	
	if(size==0) return 0;	
	if(off>=nodetbl[node].size){
		size_t offsize=MIN(((off+BLKSZ-1)/BLKSZ)*BLKSZ,(off+size));
		if(frealloc(fsptr,node,MIN(offsize,(off+size)))==-1){
			*errnoptr=EINVAL;
			return -1;
		}
	}loadpos(fsptr,&pos,node);
	seek(fsptr,&pos,off);
	while(pos.data!=NULLOFF && writect<size){
		char* blk=B2P(pos.dblk);
		blk[pos.dpos]=buf[writect++];
		seek(fsptr,&pos,1);
	}while(writect<size){
		if(pos.data==NULLOFF){
			size_t extsize=MIN((off+size),(nodetbl[node].nblocks+1)*BLKSZ);
			if(frealloc(fsptr,node,extsize)==-1) break;
		}loadpos(fsptr,&pos,node);
		seek(fsptr,&pos,off+writect);
		char* blk=B2P(pos.dblk);
		blk[pos.dpos]=buf[writect++];
		seek(fsptr,&pos,1);
	}return writect;
}

/* Implements an emulation of the utimensat system call on the filesystem 
   of size fssize pointed to by fsptr.

   The call changes the access and modification times of the file
   or directory indicated by path to the values in ts.

   On success, 0 is returned.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The error codes are documented in man 2 utimensat.

*/
int __myfs_utimens_implem(void *fsptr, size_t fssize, int *errnoptr,
                          const char *path, const struct timespec ts[2]) {
	fsheader *fshead=fsptr;
	inode *nodetbl;
	nodei node;
	
	fsinit(fsptr,fssize);
	nodetbl=(inode*)O2P(fshead->nodetbl);
	
	if((node=path2node(fsptr,path,NULL))==NONODE){
		*errnoptr=ENOENT;
		return -1;
	}
	
	nodetbl[node].atime=ts[0];
	nodetbl[node].mtime=ts[1];
	return 0;
}

/* Implements an emulation of the statfs system call on the filesystem 
   of size fssize pointed to by fsptr.

   The call gets information of the filesystem usage and puts in 
   into stbuf.

   On success, 0 is returned.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The error codes are documented in man 2 statfs.

   Essentially, only the following fields of struct statvfs need to be
   supported:

   f_bsize   fill with what you call a block (typically 1024 bytes)
   f_blocks  fill with the total number of blocks in the filesystem
   f_bfree   fill with the free number of blocks in the filesystem
   f_bavail  fill with same value as f_bfree
   f_namemax fill with your maximum file/directory name, if your
             filesystem has such a maximum

*/
int __myfs_statfs_implem(void *fsptr, size_t fssize, int *errnoptr,
                         struct statvfs* stbuf) {
	fsheader *fshead=fsptr;
	
	fsinit(fsptr,fssize);
	
	stbuf->f_bsize=BLKSZ;
	stbuf->f_blocks=fshead->size;
	stbuf->f_bfree=fshead->free;
	stbuf->f_bavail=fshead->free;
	stbuf->f_namemax=NAMELEN-1;
	return 0;
}
