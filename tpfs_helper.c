#include "tpfs.h"	

/*
 * get_dirent updates the dirent pointer passed to the dirent for the given path
 * It returns -1 incase of error
 * 0 if the dirent is not a directory
 * 1 if the dirent is a directory	
 */

int get_dirent(DIRENT *dirent,char *path){
	#ifdef DEBUG
		printf("get_dirent: %s\n",path);
	#endif
	if(strcmp(path,"/")==0){
		dirent = &dirent_g[0];
		return 1;
	}
	else{
		char *token = strtok(path,"/");
		DIRENT *parent = &dirent_g[0];
		while(token != NULL){
				#ifdef DEBUGx2
					printf("get_dirent: %s\n",parent->file_name);
				#endif
				int flag = 0;
				for(int i = 0; i < parent->dirent_c; i++){
					#ifdef DEBUGx2
						printf("child no: %d\n",i);
					#endif
					if(freemap_g->dirent_free[parent->dirent_l[i]] == 1){															//the block matched has been free'd, so invalid and return
						return -1;
					}
					DIRENT *child = &dirent_g[parent->dirent_l[i]];
					#ifdef DEBUGx2
						printf("Child_Name: %s\n",child->file_name);
					#endif
					if(child->file_name != NULL && strcmp(child->file_name,token)==0){
						parent = child;
						flag = 1;
					}
				}
				if(flag == 0){
					printf("Invalid path\n");
					return -1;
				}
				token = strtok(NULL,"/");
		}
		dirent = parent;
		if(inode_g[parent->inode_num].is_dir == true ){
			return 1;
		}
		else{
			return 0;
		}
	}
}




int get_inode(INODE *inode, char *path){
	#ifdef DEBUG
		printf("get_inode: %s\n",path);
	#endif
	DIRENT *temp;
	int dircheck = get_dirent(temp,path);
	if(dircheck == 0 || dircheck == 1){
		inode = &inode_g[temp->inode_num];							//something is wrong
		return dircheck;
	}
	else{
		return -1;
	}
}




/*
 *Return char string of the new path(only the new filename)
 */

char* get_dirent_parent(DIRENT *dirent, char *path){
	#ifdef DEBUG
		printf("get_dirent_parent: %s\n",path);
	#endif
	char *new = (char *)malloc(sizeof(char)*strlen(path));
	int i = strlen(path)-1;
	int r = 0;
	while(path[i] != '/'){
		new[r++] = path[i];
		i--;
	}
	new = realloc(new,(sizeof(char)*r+1));
	new[r+1] = '\0'
	int new_len = i-r;
	char *parent_path = (char *)malloc(sizeof(char)*new_len+1);
	for(int j = 0; j < new_len; j++){
		parent_path[j] = path[j];
	}
	parent_path[new_len] = '\0';
	if(parent_path[0] != '/'){
		parent_path = realloc(parent_path,sizeof(char)*2);
		parent_path[0] = '/';
		parent_path[1] = '\0';
	}
	int dircheck = get_dirent(dirent, parent_path);
	return new;
}



int get_inode_free(){
	for(i = 0; i < INODE_MAX; i++){
		if(freemap_g->inode_free[i] == 1){
			return i;
		}
	}
	if(i == INODE_MAX){
		return -1;
	}
}



int get_dirent_free(){
	for(i = 0; i < DIRENT_MAX; i++){
		if(freemap_g->dirent_free[i] == 1){
			return i;
		}
	}
	if(i == DIRENT_MAX){
		return -1;
	}
}



int get_data_blk_free(){
	for(i = 0; i < INODE_MAX; i++){
		if(freemap_g->datablk_free[i] == 1){
			return i;
		}
	}
	if(i == INODE_MAX){
		return -1;
	}
}


/*
 *
 *
 *Persistence helpers
 *
 *
 */

int inode_initialise(FILE *fp){
	if(fp != NULL){																			//pers_tpfs found, initialise inode_g with the contents of the pers file
		FILE *temp_fp = fp;
		fseek(temp_fp,INODE_OFF,SEEK_SET);
		fread(inode_g,sizeof(INODE),INODE_MAX,temp_fp);
	}
																											// else dont do shit
}

int dirent_initialise(FILE *fp){
	if(fp != NULL){
		FILE *temp_fp = fp;
		fseek(temp_fp,DIRENT_OFF,SEEK_SET);
		fread(dirent_g,sizeof(DIRENT),DIRENT_MAX,temp_fp);
	}
																											//again if not present dont do shit
}

int datablk_initialise(FILE *fp){
	if(fp != NULL){
		FILE *temp_fp = fp;
		fseek(temp_fp,DATABLK_OFF,SEEK_SET);
		fread(datablk_g,sizeof(DATA_BlOCK),DATA_BLOCKS,temp_fp);
	}
																											//dont do shit
}

int freemap_initialise(FILE *fp){
	if(fp != NULL){
		FILE *temp_fp = fp;
		fseek(temp_fp,FREEMAP_OFF,SEEK_SET);
		fread(freemap_g,sizeof(FREEMAP),1,temp_fp);
	}
																											//actually don't do shit here also
																											//calloc is initialising this for us already xD
}


/*
 *
 * all of these write will write individual entry passed to the file
 *
 */

int inode_write(FILE *fp, int offset, INODE *buff){
	FILE *temp_fp = fp;
	int seek_offset = INODE_OFF+offset;
	fseek(temp_fp,seek_offset,SEEK_SET);
	fwrite(buff,sizeof(INODE),1,temp_fp);
}

int dirent_write(FILE *fp, int offset, DIRENT *buff){
	FILE *temp_fp = fp;
	int seek_offset = DIRENT_OFF+offset;
	fseek(temp_fp,seek_offset,SEEK_SET);
	fwrite(buff,sizeof(DIRENT),1,temp_fp);
}

int datablk_write(FILE *fp, int offset,DATA_BLOCK *buff){
	FILE *temp_fp = fp;
	int seek_offset = DATABLK_OFF+offset;
	fseek(temp_fp,seek_offset,SEEK_SET);
	fwrite(buff,sizeof(DATA_BLOCK),1,temp_fp);
}


int freemap_write(FILE *fp){
	FILE *temp_fp = fp;
	int seek_offset = FREEMAP_OFF;
	fseek(temp_fp,seek_offset,SEEK_SET);
	fwrite(freemap_g,sizeof(FREEMAP),1,temp_fp);
}



/*
 *
 * This function should write whole of the filesystem
 *
 */


int tpfs_to_disk(FILE *fp){
	FILE *temp_fp = fp;
	int seek_offset = 0;
	fseek(temp_fp,seek_offset,SEEK_SET);
	fwrite(TPFS,sizeof(char),TPFS_SIZEm,temp_fp);
}


















