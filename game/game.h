#define HASH_ROW 7
#define HASH_COL 10000
#define LIST_USER_LEN 200
#define LIST_GAME_LEN 200

static int prime_list[HASH_ROW] = {99991, 99989, 99971, 99961, 99929, 99923, 99907};

/**
 * 在线用户
 */
struct CHashUserData
{
    typedef unsigned long KeyType;
    typedef const unsigned long ConstKeyType;
    static unsigned int GetUintHash(const KeyType & key)
    {
        return key;
    }

    static int * GetRowSizeList()
    {
        return prime_list;
    }

    const KeyType & GetKey() const 
    {
        return id;
    }

    bool SameKey(const KeyType & key) const
    {
        return key == id;
    }

    bool IsEmpty() const
    {
        return id==0;
    }

    void SetEmpty()
    {
        id = 0;
    }


    unsigned long id;
    char nickName[50];
    int score;
    int status;
    int gameMessageStatus;
    char gameMessage[50];
    /* data */
};
/**
 * 在线用户
 */
struct CHashGameData
{
    typedef unsigned long KeyType;
    typedef const unsigned long ConstKeyType;
    static unsigned int GetUintHash(const KeyType & key)
    {
        return key;
    }

    static int * GetRowSizeList()
    {
        return prime_list;
    }

    const KeyType & GetKey() const 
    {
        return id;
    }

    bool SameKey(const KeyType & key) const
    {
        return key == id;
    }

    bool IsEmpty() const
    {
        return id==0;
    }

    void SetEmpty()
    {
        id = 0;
    }


    unsigned long id;
    unsigned long playA;
    unsigned long playACode;
    int           playAScore;
    char          playANickName[50];
    unsigned long playB;
    unsigned long playBCode;
    int           playBScore;
    char          playBNickName[50];
    int result;
    int startTime;
};

/** 
 * 出错信息 
 */
typedef struct MsgInfo{
    int code;
    char message[100];
}MsgInfo_t;



struct UsrData{
	char usr_id[16];
	char usr_pwd[16];
	char usr_nickname[16];
	int age;
};

struct PkgInfo
{
	int id;
	//1:登陆.提交昵称[done]
	//2:开设战局[done]
	//3:加入战局[done]
        //4:出拳[done]
        //5:查询战局[done]
        //6.查询用户[doing]
	
	//11:查询战局列表[done]
	//12:查询在线人线[done]
        //120:返回在线人[done]
	//13:退出登陆[done]
        //14:出错信息[doing]
        //15:测试test[doing]
	char data[5000];
	/* data */
};

struct head
{
    int cmd_id;
    unsigned uin;
};

struct GameInfo{
	char play_a[100];
	int play_a_code;
	char play_b[100];
	int play_b_code;
	int result;
	int start_time;
};

struct UserInfo
{
	char id[100];
	char nick[100];
	int score;
	int status;	
};

typedef struct GameList
{
	CHashGameData game[LIST_GAME_LEN];
} GameList_t;

typedef struct UserList
{
	CHashUserData user[LIST_USER_LEN];
        //int key[LIST_USER_LEN];
} UserList_t;