struct UsrData{
	char usr_id[16];
	char usr_pwd[16];
	char usr_nickname[16];
	int age;
};

struct CmdInfo
{
	int cmd_id;
	//1:登陆.提交昵称
	//2:开设战局
	//3:加入战局
	
	//11:查询战局
	//12:查询在线人线
	//13:退出登陆
	char data[1000];
	/* data */
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

struct GameList
{
	GameInfo info[10];
};

struct UserList
{
	UserInfo info[10];
};