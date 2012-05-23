
#include "dna_sharemem.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef linux
	
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

/*
int shm_show_log = 1;
#define SHM_LOG_MSG(s)  do { if (shm_show_log) fprintf(stderr,s); } while(0)
#define SHM_LOG_BILL(s) do { if (shm_show_log) fprintf(stderr,s); } while(0)
*/

#define SHM_LOG_MSG(s)  do { strcat(shm_info->log, s); } while(0)

char * print_shm_head(char *str)
{
	sprintf(str, "  pid        key    shm_id");
	return str;
}

char * format_shm_info(char *str, struct stShmInfo *shm_info)
{
	sprintf(str, "%5u 0x%08X %9d", 
			getpid(), shm_info->key, shm_info->shm_id);
	return str;
}

int InitKeyByFile(struct stShmInfo *shm_info, const char *pathname, int proj_id)
{
	key_t key = ftok(pathname, proj_id);
	if (key == -1)
		return -1;
	shm_info->key = key;
	return 0;
}

char * GetShm(struct stShmInfo *shm_info, bool *newCreated)
{
	shm_info->log[0] = 0;
	*newCreated = true;

	shm_info->shm_id = -1;
	shm_info->shm_addr = NULL;

	char msg[1024];
	char shamemem_info[256]; 
	*shamemem_info = 0;

	SHM_LOG_MSG("\n");

	print_shm_head(shamemem_info);
	sprintf(msg, "[%s]\n", shamemem_info);
	SHM_LOG_MSG(msg);

	format_shm_info(shamemem_info, shm_info);
	
	if (shm_info->key < 0) {
		sprintf(msg, "[%s] sharemem key error, %d\n", shamemem_info, shm_info->key);
		SHM_LOG_MSG(msg);
		return NULL;
	}
	
	sprintf(msg, "[%s] allocating shm of %d bytes.\n", shamemem_info, shm_info->size);
	SHM_LOG_MSG(msg);
	
	// IPC_EXCL: ??IPC_CREATһ??ʹ??ʱ?????Ѿ????ڶ???ʱ???򷵻?ʧ?ܣ???ֹ?ؼ????ظ??? 
	shm_info->shm_id = shmget(shm_info->key, shm_info->size, IPC_CREAT|IPC_EXCL|0666);
	format_shm_info(shamemem_info, shm_info);
	if (shm_info->shm_id < 0) {
		if (errno != EEXIST) {
			sprintf(msg, "[%s] alloc shm failed. %d: %s\n", shamemem_info, errno, strerror(errno));
			SHM_LOG_MSG(msg);
			return NULL;
		}
		
		sprintf(msg, "[%s] attaching shm due to it already exists. %d: %s\n", shamemem_info, errno, strerror(errno));
		SHM_LOG_MSG(msg);

		shm_info->shm_id = shmget(shm_info->key, shm_info->size, 0666);
		format_shm_info(shamemem_info, shm_info);
		if (shm_info->shm_id < 0) {
			sprintf(msg, "[%s] touching shm due to attach shm failed. %d: %s\n", shamemem_info, errno, strerror(errno));
			SHM_LOG_MSG(msg);

			shm_info->shm_id = shmget(shm_info->key, 0, 0666);
			format_shm_info(shamemem_info, shm_info);

			if (shm_info->shm_id < 0) {
				sprintf(msg, "[%s] touch shm failed. %d: %s\n", shamemem_info, errno, strerror(errno));
				SHM_LOG_MSG(msg);
				return NULL;
			}

			sprintf(msg, "[%s] removing existed shm. \n", shamemem_info);
			SHM_LOG_MSG(msg);

			if (shmctl(shm_info->shm_id, IPC_RMID, NULL)) {
				sprintf(msg, "[%s] remove shm failed. %d: %s\n", shamemem_info, errno, strerror(errno));
				SHM_LOG_MSG(msg);
				return NULL;
			}

			sprintf(msg, "[%s] reallocating shm. \n", shamemem_info);
			SHM_LOG_MSG(msg);

			shm_info->shm_id = shmget(shm_info->key, shm_info->size, IPC_CREAT|IPC_EXCL|0666);
			format_shm_info(shamemem_info, shm_info);

			if (shm_info->shm_id < 0) {
				sprintf(msg, "[%s] realloc shm failed. %d: %s\n", shamemem_info, errno, strerror(errno));
				SHM_LOG_MSG(msg);
				return NULL;
			}

			sprintf(msg, "[%s] realloc shm succeed. \n", shamemem_info);
			SHM_LOG_MSG(msg);
		}
		else {
			*newCreated = false;
			sprintf(msg, "[%s] attach shm succeed.\n", shamemem_info);
			SHM_LOG_MSG(msg);
		}
	}
	else {
		sprintf(msg, "[%s] alloc shm succeed. \n", shamemem_info);
		SHM_LOG_MSG(msg);
	}
	
	time_t now; time(&now);
	sprintf(msg, "[%s] time %d\n", shamemem_info, (int)now);
	SHM_LOG_MSG(msg);
	
	//shmctl(shm_info->shm_id, SHM_LOCK, NULL);
	shm_info->shm_addr = (char *)shmat(shm_info->shm_id, NULL, 0);

	sprintf(msg, "[%s] shmat: [0x%08x, 0x%08x]. len: %d. \n", 
		shamemem_info, (int)shm_info->shm_addr, (int)shm_info->shm_addr+shm_info->size, shm_info->size);
	SHM_LOG_MSG(msg);
	return shm_info->shm_addr;
}

int DelShm(struct stShmInfo *shm_info)
{
	shm_info->log[0] = 0;
//	int result = 0;
	char msg[1024];
	char shamemem_info[256]; 

//	shm_info->size = 0;
//	shm_info->shm_addr = NULL;
	
	SHM_LOG_MSG("\n");
	
	print_shm_head(shamemem_info);
	sprintf(msg, "[%s]\n", shamemem_info);
	SHM_LOG_MSG(msg);
	
	format_shm_info(shamemem_info, shm_info);

	if (shm_info->shm_id >= 0) {
		sprintf(msg, "[%s] removing shm by shm_id %d \n", shamemem_info, shm_info->shm_id);
		SHM_LOG_MSG(msg);
	}
	else if (shm_info->key >= 0) {
		sprintf(msg, "[%s] removing shm by key 0x%08X \n", shamemem_info, shm_info->key);
		SHM_LOG_MSG(msg);
		
		sprintf(msg, "[%s] touching shm \n", shamemem_info);
		SHM_LOG_MSG(msg);

		shm_info->shm_id = shmget(shm_info->key, 0, 0666);
		format_shm_info(shamemem_info, shm_info);

		if (shm_info->shm_id < 0) {
			sprintf(msg, "[%s] touch shm failed. %d: %s\n", shamemem_info, errno, strerror(errno));
			SHM_LOG_MSG(msg);
			return -1;
		}
	}
	else {
		sprintf(msg, "[%s] invalid shm, shm_id %d, key 0x%08X \n", shamemem_info, shm_info->shm_id, shm_info->key);
		SHM_LOG_MSG(msg);
		return -1;
	}
	
	if (shmctl(shm_info->shm_id, IPC_RMID, NULL)) {
		sprintf(msg, "[%s] remove shm failed. %d: %s\n", shamemem_info, errno, strerror(errno));
		SHM_LOG_MSG(msg);
		return -1;
	}
	
	sprintf(msg, "[%s] remove shm succeed. \n", shamemem_info);
	SHM_LOG_MSG(msg);
	
	return 0;
}


#endif // end of #ifdef linux
