/*


demo:

#include "dna_sharemem.h"

int main()
{
	struct stShmInfo shm_info;
	
	shm_info.key = -1;
	shm_info.size = 60;
	shm_info.shm_id = -1;
	shm_info.shm_addr = NULL;
	
	if (InitKeyByFile(&shm_info, "./keyfile", 'A'))
		return -1;
	
	bool new_created;
	if (GetShm(&shm_info, &new_created) == NULL)
		return -1;

	if (DelShm(&shm_info))
		return -1;

	return 0;
}

*/

#ifndef _SHAREMEM_UTILS_HPP_
#define _SHAREMEM_UTILS_HPP_

#ifdef linux

#include <stddef.h>
#include <sys/types.h>

struct stShmInfo
{
	key_t key;
	size_t size;
	
	int shm_id;
	char *shm_addr;

	char log[1024];
};

char * print_shm_head(char *str);
char * format_shm_info(char *str, struct stShmInfo *shm_info);

int InitKeyByFile(struct stShmInfo *shm_info, const char *pathname, int proj_id);
char * GetShm(struct stShmInfo *shm_info, bool *newCreated);
int DelShm(struct stShmInfo *shm_info);

#endif

#endif
