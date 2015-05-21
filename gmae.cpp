#include "MyDirectX.h";

struct SPRITE
{
	int x, y;
	int movex, movey;
	int frame, columns;
	int width, height;
	int startframe, endframe;
	int starttime, delay;
	int direction;
	int startY; // this is the start value (reset value of each sprite, if needed)
	bool explodes; //for meteor, used also for person
	SPRITE()
	{
		x = y = 0;
		movex = movey = 0;
		frame = 0; columns = 1;
		width = height = 0;
		startframe = endframe = 0;
		starttime = delay = 0;
		direction = 1;
		startY = 0;
		explodes = false;
	}
};

const string APPTITLE = "Meteor Shower";
const int SCREENWIDTH = 1024;
const int SCREENHEIGHT = 576;
//LPDIRECTINPUT8 dinput;


//Direct X Required Global Variables.
extern CSoundManager *dsound;
LPD3DXSPRITE sprite_obj;
LPDIRECT3DSURFACE9 back_img = NULL;
LPDIRECT3DTEXTURE9 person_img;
LPDIRECT3DTEXTURE9 meteor_img;
LPDIRECT3DTEXTURE9 missile_img;
LPD3DXFONT pFont = 0;

//handle sounds...
CSound *sound_explode;
CSound *sound_shot;
CSound *sound_die;
CSound *sound_break;

const int cellwidth = 32;
const int cellheight = 32;
const int BULLET_NUMBERS = 60;
const int METEOR_NUMBERS = 20;
const int cellrows = SCREENHEIGHT / cellheight; //15
const int cellcolumns = SCREENWIDTH / cellwidth; // 20
int bulletWidth = 25;
int bulletHeight = 95;
int meteorWidth = 50;
int meteorHeight = 50;
int nextBulletIndex = 0;
bool win = false;
RECT infoArea;

int score = 0;
int level = 0;
int max_score;

SPRITE bullets[BULLET_NUMBERS];
SPRITE person;
SPRITE meteors[METEOR_NUMBERS];

//To See if a Meteor is hit by a missile
bool checkForImpact(SPRITE *bull)
{
	// define stone rectangle to intersect with missle rectangles
	RECT rect1;
	rect1.left = bull->x + 1;
	rect1.top = bull->y + 1;
	rect1.right = bull->x + bull->width - 1;
	rect1.bottom = bull->y + bull->height - 1;

	// check if any metoer has been hit by a missle
	RECT dest;
	RECT rect2;
	for (int i = 0; i < BULLET_NUMBERS; i++)
	{
		rect2.left = bullets[i].x + 1;
		rect2.top = bullets[i].y + 1;
		rect2.right = bullets[i].x + bullets[i].width - 1;
		rect2.bottom = bullets[i].y + bullets[i].height - 1;

		// check if this metoer has been hit by some missile...
		if (IntersectRect(&dest, &rect1, &rect2)){
			// missle has hit a metoer, explode it
			bull->explodes = false;
			//explore meteor
			meteors[i].explodes = true;
			// update score...
			score += 1;
			// cheack if palyer reached score...
			if (score >= max_score)
				win = true;
			return true;
		}
	}
	//this is a miss!
	return false;
}

bool LoadPerson()
{
	D3DXIMAGE_INFO info;
	HRESULT result;

	//load the person sprite
	person_img = LoadTexture("fisga.bmp", D3DCOLOR_XRGB(255, 0, 255));
	if (person_img == NULL)
		return false;
	//set the person's properties
	result = D3DXGetImageInfoFromFile("fisga.bmp", &info);
	if (result != D3D_OK)
		return false;

	person.width = info.Width;
	person.height = info.Height / 4;
	person.x = (SCREENWIDTH - person.width) / 2;
	person.y = 600;
	person.movex = 0; 
	person.movey = -1;// since person can only move on x axis in our game
	person.endframe = 1;

	return true;
}

bool LoadMeteorImage()
{
	D3DXIMAGE_INFO info;
	HRESULT result;

	meteor_img = LoadTexture("bottle_break_sequence.bmp", D3DCOLOR_XRGB(255, 0, 255));
	if (meteor_img == NULL)
		return false;
	//set the meteors's properties
	result = D3DXGetImageInfoFromFile("bottle_break_sequence.bmp", &info);
	if (result != D3D_OK)
		return false;
	meteorWidth = info.Width / 6;
	meteorHeight = info.Height;

	return true;
}

bool LoadMeteorsArray()
{
	for (int i = 0; i < METEOR_NUMBERS; i++)
	{
		meteors[i].width = meteorWidth;
		meteors[i].height = meteorHeight;
		meteors[i].x = (rand() % cellcolumns) * meteors[0].width;
		meteors[i].y = -((rand() % cellrows) * meteors[0].height) + infoArea.bottom;
		meteors[i].startY = meteors[i].y;
		meteors[i].movex = 0;
		meteors[i].columns = 6;
		meteors[i].endframe = 5;
	}
	return true;
}

bool LoadMissileImage()
{
	D3DXIMAGE_INFO info;
	HRESULT result;

	//load the missile's sprite
	missile_img = LoadTexture("_missile.png", D3DCOLOR_XRGB(255, 0, 255));
	if (missile_img == NULL)
		return false;
	//set the missile's properties
	result = D3DXGetImageInfoFromFile("_missile.png", &info);
	if (result != D3D_OK)
		return false;
	bulletWidth = info.Width;
	bulletHeight = info.Height;

	return true;
}

bool LoadMissileArray()
{
	for (int i = 0; i < BULLET_NUMBERS; i++)
	{
		meteors[i].width = bulletWidth;
		meteors[i].height = bulletHeight;
		meteors[i].x = 0;
		meteors[i].y = SCREENHEIGHT - person.height - meteors[i].height / 2;
		meteors[i].startY = meteors[i].y;
		meteors[i].movex = 0;
		meteors[i].columns = 1;
		meteors[i].endframe = 0;
	}
	return true;
}

//Update functions//

void UpdatePerson()
{
	if (Key_Down(DIK_RIGHT))
	{
		person.movex = 10; //Control the speed person to the right
		person.movey = 0;
		person.x += person.movex; // only move when key pressed
		// person.y += person.movey; don't move on yy
	}
	if (Key_Down(DIK_LEFT))
	{
		person.movex = -10; //Control the speed person to the left
		person.movey = 0;
		person.x += person.movex; // only move when key pressed
		//person.y += person.movey; don't move on yy
	}
	//check if person is moving out of bounds...
	if (person.x < 0)
		person.x = 0;
	if (person.x > SCREENWIDTH - person.width)
		person.x = SCREENWIDTH - person.width;
}

void UpdateMeteorsArray()
{
	for (int i = 0; i < BULLET_NUMBERS; i++){
		if (meteors[i].explodes){
			PlaySound(sound_break);
			meteors[i].frame++;
			if (meteors[i].frame >= meteors[i].columns)
			{
				meteors[i].y = meteors[i].startY;
				meteors[i].explodes = false;
			}
		}
		else
		{
			meteors[i].movey = 1; //From here the speed for bottles can be increase
			meteors[i].y += meteors[0].movey;
			//meteors[i].frame = 0;
			if ((meteors[i].y + 2 * meteors[i].height) >= SCREENHEIGHT){
				PlaySound(sound_break);
				meteors[i].explodes = true;
				//GAME OVER CODE
				score -= 2;
			}
		}
	}
}

void UpdateMissiles()
{
	nextBulletIndex = -1;
	for (int i = 0; i < BULLET_NUMBERS; i++){
		if (bullets[i].explodes){
			if (bullets[i].y <= 0){
				bullets[i].explodes = false; // this one is now available
				bullets[i].y = bullets[i].startY;
			}
			else{
				// check for hit on bottles
				if (!checkForImpact(&bullets[i]))
					// If not hit, update positon
					bullets[i].y -= 5; //From here the speed of the stonen can be raise.
			}
		}
		else {
			if (nextBulletIndex < 0)
				nextBulletIndex = i;
		}
	}
}

void UpdateScore(){
	char buffer[100];
	if (!win)
		sprintf(buffer, "SCORE: %d", score);
	else
		sprintf(buffer, "You WIN, press ESC to exit...");
	LPCSTR str = buffer;
	pFont->DrawTextA(NULL, str, -1, &infoArea, DT_SINGLELINE | DT_VCENTER, D3DCOLOR_ARGB(0xff, 0xff, 0xff, 0xff));
}


bool Game_Init(HWND hwnd)
{
	//initialize Direct3D
	if (!Direct3D_Init(hwnd, SCREENWIDTH, SCREENHEIGHT, false))
	{
		MessageBox(hwnd, "Error initializing Direct3D", APPTITLE.c_str(), 0);
		return false;
	}

	//initialize DirectInput
	if (!DirectInput_Init(hwnd))
	{
		MessageBox(hwnd, "Error initializing DirectInput", APPTITLE.c_str(), 0);
		return false;
	}

	//initialize DirectSound
	if (!DirectSound_Init(hwnd))
	{
		MessageBox(hwnd, "Error initializing DirectSound", APPTITLE.c_str(), 0);
		return false;
	}

	// Create D3DX Font
	HFONT hFont = (HFONT)GetStockObject(SYSTEM_FONT);
	HRESULT r = D3DXCreateFontA(d3ddev, 16, 10, BOLD_FONTTYPE, MAP_COMPOSITE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, "Verdana", &pFont);

	// Define rectangle where to show score...
	infoArea.top = 0;
	infoArea.left = 0;
	infoArea.right = SCREENWIDTH;
	infoArea.bottom = 36;

	// Load background image
	back_img = LoadSurface("sky.bmp");
	if (!back_img) {
		MessageBox(hwnd, "Error loading background", "Error", 0);
		return false;
	}

	LoadPerson();
	LoadMeteorImage();
	LoadMeteorsArray();
	LoadMissileImage();
	LoadMissileArray();

	//load explode wave file
	//sound_break = LoadSound("bottle_break.wav");
	//if (sound_break == NULL)   return 0;

	//load hit wave file
	//sound_shot = LoadSound("shot_whipe.wav");
	//if (sound_shot == NULL)  return 0;
	return true;
}
void Game_Run(HWND hwnd)
{

	static bool canShoot = true;
	static int shootTick = 0;
	//make sure the Direct3D device is valid
	if (!d3ddev) return;
	DirectInput_Update();
	int start = GetTickCount();
	if (GetTickCount() - start >= 30/* && !win*/)
	{
		//reset timing
		start = GetTickCount();
		UpdatePerson();
		UpdateMeteorsArray();
		UpdateMissiles();
		//UpdateScore();
	}

	if (GetTickCount() - shootTick >= 500){
		// allow shoot if not allowed
		if (!canShoot)
			canShoot = true;

	}

	// Shoot stone if can be done
	if (canShoot && !win){
		// start check sequence
		if (person.explodes){
			if (nextBulletIndex >= 0){
				if (Key_Down(DIK_SPACE)){
					// enable stone to be visible and detectable
					bullets[nextBulletIndex].explodes = true;
					// reset stone yy coordinate
					bullets[nextBulletIndex].y = bullets[nextBulletIndex].startY;
					// set stone x coordinate
					bullets[nextBulletIndex].x = (int)(person.x + person.width / 2 - bullets[nextBulletIndex].width / 2);
					// the person is now unloaded with stone
					person.frame = 0;
					person.explodes = false;
					canShoot = false;
					// reset canShoot tick value
					shootTick = GetTickCount();
					// Play shoot sound
					PlaySound(sound_shot);
				}
			}
		}
		else if (nextBulletIndex >= 0){
			// person is now loaded
			person.frame = 1;
			person.explodes = true;
		}
	}


	if (d3ddev->BeginScene())
	{
		// Draw background
		DrawSurface(backbuffer, 0, 0, back_img);
		// needed to draw sprites
		sprite_obj->Begin(D3DXSPRITE_ALPHABLEND);
		// draw person image at correct position
		Sprite_Draw_Frame(person_img, person.x, person.y, person.frame,
			person.width, person.height, person.columns);
		// Draw all available bottles in scenario
		for (int i = 0; i < METEOR_NUMBERS; i++)
			Sprite_Draw_Frame(meteor_img, meteors[i].x, meteors[i].y, meteors[i].frame,
			meteors[i].width, meteors[i].height, meteors[i].columns);
		// Draw throunw stones
		for (int i = 0; i < BULLET_NUMBERS; i++){
			if (bullets[i].explodes)
				Sprite_Draw_Frame(missile_img, bullets[i].x, bullets[i].y, bullets[i].frame,
				bullets[i].width, bullets[i].height, bullets[i].columns);
		}
		sprite_obj->End();
		UpdateScore();
		d3ddev->EndScene();
	}


	d3ddev->Present(NULL, NULL, NULL, NULL);

	//escape key exits
	if (Key_Down(DIK_ESCAPE))
		gameover = true;
	if (score<0)
	{
		gameover = true;
		MessageBox(hwnd, "Good Luck Next Time", "The Bottles Won, TRY Harder", 0);
	}
}

void Game_End()
{

	if (back_img) back_img->Release();
	if (person_img) person_img->Release();
	if (meteor_img) meteor_img->Release();
	if (missile_img) missile_img->Release();
	if (pFont) pFont->Release();
	/*if (explode_image) explode_image->Release();
	if (enemy_image) enemy_image->Release();
	if (die_image) die_image->Release();*/
	if (sound_break != NULL)	delete sound_break;
	if (sound_shot != NULL)	delete sound_shot;
	//if (infoArea != NULL) delete infoArea;

	DirectSound_Shutdown();
	DirectInput_Shutdown();
	Direct3D_Shutdown();

}
