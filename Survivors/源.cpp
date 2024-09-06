#include <graphics.h>
#include <string>
#include <vector>

//删除了玩家速度,玩家各项信息
int idx_current_anim = 0;
const int PLAYER_ANIM_NUM = 6;

int WINDOW_WIDTH = 1280;
int WINDOW_HEIGHT = 720;

IMAGE img_player_left[PLAYER_ANIM_NUM];
IMAGE img_player_right[PLAYER_ANIM_NUM];


#pragma comment(lib,"MSIMG32.LIB")


inline void putimage_alpha(int x, int y, IMAGE* img)
{
	int w = img->getwidth();
	int h = img->getheight();
	AlphaBlend(GetImageHDC(NULL), x, y, w, h,
		GetImageHDC(img), 0, 0, w, h, { AC_SRC_OVER,0,255,AC_SRC_ALPHA });
}
class Animation
{
public:

	Animation(LPCTSTR path, int num, int interval)
	{
		interval_ms = interval;

		TCHAR path_file[256];
		for (size_t i = 0; i < num; i++)
		{
			_stprintf_s(path_file, path, i);

			IMAGE* frame = new IMAGE();
			loadimage(frame, path_file);//把资源注入内存当中
			frame_list.push_back(frame);
		}
	}
	~Animation()
	{
		for (size_t i = 0; i < frame_list.size(); i++)
		{
			delete frame_list[i];
		}
	}

	void Play(int x, int y, int delta)
	{
		timer += delta;
		if (timer >= interval_ms)
		{
			idx_frame = (idx_frame + 1) % frame_list.size();
			timer = 0;
		}

		putimage_alpha(x, y, frame_list[idx_frame]);
	}

private:
	int timer = 0;			//动画计时器
	int idx_frame = 0;		//动画帧索引
	int interval_ms = 0;
	std::vector<IMAGE*>  frame_list;

};


class Player
{
public:
	Player()
	{
		loadimage(&img_shadow, _T("img/shadow_player.png"));

		anim_left = new Animation(_T("img/player_left_%d.png"), 6, 45);
		anim_right = new Animation(_T("img/player_right_%d.png"), 6, 45);
	}

	~Player()
	{
		delete anim_left;
		delete anim_right;
	}

	void ProcessEvent(const ExMessage& msg)
	{
		switch (msg.message)
		{
		case WM_KEYDOWN:
			switch (msg.vkcode)
			{
			case VK_UP:
				is_move_up = 1;
				break;
			case VK_DOWN:
				is_move_down = 1;
				break;
			case VK_LEFT:
				is_move_left = 1;
				break;
			case VK_RIGHT:
				is_move_right = 1;
				break;
			}break;
		case WM_KEYUP:
			switch (msg.vkcode)
			{
			case VK_UP:
				is_move_up = 0;
				break;
			case VK_DOWN:
				is_move_down = 0;
				break;
			case VK_LEFT:
				is_move_left = 0;
				break;
			case VK_RIGHT:
				is_move_right = 0;
				break;
			}break;
		}

	}

	void Move()
	{
		dir_x = is_move_right - is_move_left;
		dir_y = is_move_down - is_move_up;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir != 0)
		{
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			position.x += (int)(PLAYER_SPEED * normalized_x);
			position.y += (int)(PLAYER_SPEED * normalized_y);
		}

		if (position.x < 0) position.x = 0;
		if (position.y < 0) position.y = 0;
		if (position.x + PLAYER_WIDTH > WINDOW_WIDTH) position.x = WINDOW_WIDTH - PLAYER_WIDTH;
		if (position.y + PLAYER_HEIGHT > WINDOW_HEIGHT) position.y = WINDOW_HEIGHT - PLAYER_HEIGHT;
	}

	void Draw(int delta)
	{
		int pos_shadow_x = position.x + (PLAYER_WIDTH / 2 - SHADOW_WIDTH / 2);
		int pos_shadow_y = position.y + PLAYER_HEIGHT - 8;
		putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);

		static bool facing_left = 0;
		if (dir_x < 0)
			facing_left = 1;
		else if (dir_x > 0)
			facing_left = 0;

		if (facing_left)
			anim_left->Play(position.x, position.y, delta);
		else
			anim_right->Play(position.x, position.y, delta);
	}

	const POINT& Getposition()const
	{
		return position;
	}
private:
	const int PLAYER_SPEED = 2;
	const int PLAYER_WIDTH = 80;
	const int PLAYER_HEIGHT = 80;
	const int SHADOW_WIDTH = 32;
private:
	IMAGE img_shadow;
	Animation* anim_left;
	Animation* anim_right;
	POINT position{ 500,500 };

	bool is_move_up = 0;
	bool is_move_down = 0;
	bool is_move_left = 0;
	bool is_move_right = 0;

	int dir_x;
	int dir_y;

};

class Bullet
{
public:
	POINT position{ 0,0 };
public:
	Bullet() = default;
	~Bullet() = default;

	void Draw() const
	{
		setlinecolor(RGB(255, 155, 50));
		setfillcolor(RGB(200, 75, 10));
		fillcircle(position.x, position.y, RADIUS);
	}

private:
	const int RADIUS = 10;
};

class Enemy
{
public:
	Enemy()
	{
		loadimage(&img_shadow, _T("img/shadow_enemy.png"));
		anim_left = new Animation(_T("img/enemy_left_%d.png"), 6, 45);
		anim_right = new Animation(_T("img/enemy_right_%d.png"), 6, 45);

		enum class SpawnEdge
		{
			Up = 0,
			Down,
			Left,
			Right
		};

		//将敌人放置在地图外边界处的随机位置
		SpawnEdge edge = (SpawnEdge)(rand() % 4);
		switch (edge)
		{
		case SpawnEdge::Up:
			position.x = rand() % WINDOW_WIDTH;
			position.y = -FRAME_HEIGHT;
			break;
		case SpawnEdge::Down:
			position.x = rand() % WINDOW_WIDTH;
			position.y = WINDOW_HEIGHT;
			break;
		case SpawnEdge::Left:
			position.x = -FRAME_HEIGHT;
			position.y = rand() % WINDOW_HEIGHT;
			break;
		case SpawnEdge::Right:
			position.x = WINDOW_WIDTH;
			position.y = rand() % WINDOW_HEIGHT;
			break;
		default:break;
		}
	}

	bool CheckBulletCollision(const Bullet& bullet)
	{
		return false;
	}
	bool CheckPlayerCollision(const Player& player)
	{
		return false;
	}

	void Move(const Player& player)
	{
		const POINT& player_position = player.Getposition();
		int dir_x = player_position.x - position.x;
		int dir_y = player_position.y - position.y;
		if (dir_x < 0) facing_left = 1;
		else facing_left = 0;

		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir != 0)
		{
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			position.x += (int)(SPEED * normalized_x);
			position.y += (int)(SPEED * normalized_y);
		}
	}


	~Enemy()
	{
		delete anim_left;
		delete anim_right;
	}

	void Draw(int delta)
	{
		int pos_shadow_x = position.x + (FRAME_WIDTH / 2 - SHADOW_WIDTH / 2);
		int pos_shadow_y = position.y + FRAME_HEIGHT - 8;
		putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);

		if (facing_left)
			anim_left->Play(position.x, position.y, delta);
		else
			anim_right->Play(position.x, position.y, delta);
	}

private:
	const int SPEED = 2;
	const int FRAME_WIDTH = 80;
	const int FRAME_HEIGHT = 80;
	const int SHADOW_WIDTH = 48;

private:
	IMAGE img_shadow;
	Animation* anim_left;
	Animation* anim_right;
	POINT position = { 0,0 };
	bool facing_left = false;
};




//动画的表达类

//Animation anim_left_player(_T("img/player_left_%d.png"), 6, 45);
//Animation anim_right_player(_T("img/player_right_%d.png"), 6, 45);

//void LoadAnimation()//把动画资源找到并且储存到内存中
//{
//	for (size_t i= 0; i < PLAYER_ANIM_NUM; i++)
//	{
//		std::wstring path = L"img/player_left_" + std::to_wstring(i) + L".png";
//		loadimage(&img_player_left[i], path.c_str());
//	}
//
//	for (size_t i = 0; i < PLAYER_ANIM_NUM; i++)
//	{
//		std::wstring path = L"img/player_right_" + std::to_wstring(i) + L".png";
//		loadimage(&img_player_right[i], path.c_str());
//	}
//}

void TryGenerateEnemy(std::vector<Enemy*>& enemy_list)//敌人生成函数
{
	const int INTERVAL = 100;
	static int counter = 0;
	if ((++counter) % INTERVAL == 0)
		enemy_list.push_back(new Enemy());
}

int main()
{


	initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);

	bool running = 1;

	ExMessage msg;//创建一个叫msg的ExMessage结构体,就是外部输入的信息

	IMAGE img_background;//创建一个叫 背景 的IMAGE型变量

	Player player;//创建一个player
	std::vector<Enemy*>enemy_list;
	/*bool is_move_up = 0;
	bool is_move_down = 0;
	bool is_move_left = 0;
	bool is_move_right = 0;*/


	loadimage(&img_background, _T("img/background.png"));


	BeginBatchDraw();


	while (running)
	{
		DWORD start_time = GetTickCount();

		while (peekmessage(&msg))
		{
			player.ProcessEvent(msg);
		}
		player.Move();
		TryGenerateEnemy(enemy_list);
		for (Enemy* enemy : enemy_list)
			enemy->Move(player);

		cleardevice();

		putimage(0, 0, &img_background);
		player.Draw(1000 / 144);
		for (Enemy* enemy : enemy_list)
			enemy->Draw(1000 / 144);

		FlushBatchDraw();

		//保证是144帧数
		DWORD end_time = GetTickCount();
		DWORD delta_time = end_time - start_time;
		if (delta_time < 1000 / 144)
		{
			Sleep(1000 / 144 - delta_time);
		}
	}
	EndBatchDraw();
	return 0;
}
