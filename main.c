#include <stdio.h>
#include <ncurses.h>
#include <math.h> //abs fonksiyonu icin yaptik(c de fabs var bu kutuphane icinde floatlari felanda duzgun yapiyor)
#include <unistd.h>
#include <time.h>
#define MapSize 20
#define HEIGHT 45
#define WIDTH 190
#define DIRLENGTH 1.0   // sakin degistirme bu degeri buna guvenerek matematiksel hesaplamalar yapiliyor(1 olmasina gore)
#define PLANELENGTH 1.0 // fov acisini degistirmek icin bunu degistirebiliriz, bunun boyutuna gore fov acisi degisecek
#define ROTSPEED 0.07
#define PLAYERSPEED 0.05

#define FPS 200
#define FRAME_TIME_US (1000000 / FPS) // Mikro saniye cinsinden hedef süre
// STRUCTS
typedef struct
{
  float x;
  float y;
} Vector;

typedef struct
{
  double x;
  double y;
} VectorDouble;

typedef struct
{
  Vector position;
  Vector moveSpeed;
  Vector dir;
  Vector plane;
} character;

typedef struct
{
  VectorDouble rayDir;    // asil olaylardan yon
  VectorDouble deltaDist; // atlama mesafesi
  VectorDouble sideDist;  // ilk cizgiye uzaklik
  int mapX;
  int mapY;           // haritadaki anlik konum
  int stepX;          // sag sol
  int stepY;          // asagi yukari
  int hit;            // hit the wall
  int side;           // X duvarı mı, Y duvarı mı? (0 veya 1)
  float perpWallDist; // fisheye cozumu burda eger siteyi okursan(yani fazladan bir plane felan ypk bizde bura zaten cozuyor(hatta tam bura degilde onceden yaptigimiz
                      // matematiksel islemler cozuyor))
} rayCasting;

// function
void DDA(rayCasting *ray);
void print(double wallDist, int side, int whichCol);

int map[MapSize][MapSize] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1},
    {1, 1, 1, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},
    {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

int main()
{
  character player;
  player.dir.x = DIRLENGTH;
  player.dir.y = 0;
  player.plane.x = 0;
  player.plane.y = PLANELENGTH;
  player.position.x = 1.5;
  player.position.y = 4.5;
  // burda bir mantik hatirlatmasi karakter aslinda map[y][x] icinde hareket edecek, neden boyle cunki normal matematikte
  // x yatay y dikey eksendir fakat arraylerde tam tersi ve biz obur turlu yaparsak isin icinden cikamayiz butun mat hesaplamalarinda sunda bunda

  initscr();
  cbreak();
  noecho();              // Klavyede basılan tuşları terminale yazı olarak yazma
  nodelay(stdscr, TRUE); // getch() fonksiyonu tuş beklemesin, oyunu dondurmasın
  keypad(stdscr, TRUE);  // Yön tuşları (oklar) çalışabilsin
  curs_set(0);           // Yanıp sönen terminal imlecini (cursor) gizle
  // main game loop
  rayCasting ray;
  bool gameRunning = true;
  struct timespec start, end;
  long diff_ms;
  while (gameRunning)
  {
    // 1. GİRİŞ (Input)
    // Kullanıcıdan gelen girdileri oku (klavye, fare vb.)
    clock_gettime(CLOCK_MONOTONIC, &start);
    int key = getch();
    if (key == KEY_LEFT)
    {
      double oldDirX = player.dir.x;
      player.dir.x = player.dir.x * cos(-ROTSPEED) - player.dir.y * sin(-ROTSPEED);
      player.dir.y = oldDirX * sin(-ROTSPEED) + player.dir.y * cos(-ROTSPEED);
      double oldPlaneX = player.plane.x;
      player.plane.x = player.plane.x * cos(-ROTSPEED) - player.plane.y * sin(-ROTSPEED);
      player.plane.y = oldPlaneX * sin(-ROTSPEED) + player.plane.y * cos(-ROTSPEED);
    }
    if (key == KEY_RIGHT)
    {
      double oldDirX = player.dir.x;
      player.dir.x = player.dir.x * cos(ROTSPEED) - player.dir.y * sin(ROTSPEED);
      player.dir.y = oldDirX * sin(ROTSPEED) + player.dir.y * cos(ROTSPEED);
      double oldPlaneX = player.plane.x;
      player.plane.x = player.plane.x * cos(ROTSPEED) - player.plane.y * sin(ROTSPEED);
      player.plane.y = oldPlaneX * sin(ROTSPEED) + player.plane.y * cos(ROTSPEED);
    }
    if (key == 'w')
    {
      player.position.x += PLAYERSPEED * player.dir.x;
      player.position.y += PLAYERSPEED * player.dir.y;
    }
    if (key == 's')
    {
      player.position.x -= PLAYERSPEED * player.dir.x;
      player.position.y -= PLAYERSPEED * player.dir.y;
    }
    if (key == 'a')
    {
      player.position.x += PLAYERSPEED * player.dir.y;
      player.position.y -= PLAYERSPEED * player.dir.x;
    }
    if (key == 'd')
    {
      player.position.x -= PLAYERSPEED * player.dir.y;
      player.position.y += PLAYERSPEED * player.dir.x;
    }
    if (key == 'q')
    { // 'q' tuşuna basınca çık
      gameRunning = false;
    }
    // 2. GÜNCELLEME (Update)
    // Oyun mantığını burada işlet (taşların hareketi, portal kontrolü vb.)
    werase(stdscr);
    for (int x = 0; x < WIDTH; x++)
    {
      double cameraX = 2 * x / (double)WIDTH - 1; // x-coordinate in camera space
      ray.rayDir.x = player.dir.x + player.plane.x * cameraX;
      ray.rayDir.y = player.dir.y + player.plane.y * cameraX;
      ray.mapX = (int)player.position.x;
      ray.mapY = (int)player.position.y;
      ray.deltaDist.x = (ray.rayDir.x == 0) ? 1e30 : fabs(1 / ray.rayDir.x);
      ray.deltaDist.y = (ray.rayDir.y == 0) ? 1e30 : fabs(1 / ray.rayDir.y);
      ray.hit = 0;
      if (ray.rayDir.x < 0)
      {
        ray.stepX = -1;
        ray.sideDist.x = (player.position.x - ray.mapX) * ray.deltaDist.x;
      }
      else
      {
        ray.stepX = 1;
        ray.sideDist.x = (ray.mapX + 1.0 - player.position.x) * ray.deltaDist.x;
      }
      if (ray.rayDir.y < 0)
      {
        ray.stepY = -1;
        ray.sideDist.y = (player.position.y - ray.mapY) * ray.deltaDist.y;
      }
      else
      {
        ray.stepY = 1;
        ray.sideDist.y = (ray.mapY + 1.0 - player.position.y) * ray.deltaDist.y;
      }
      DDA(&ray);
      if (ray.side == 0)
        ray.perpWallDist = (ray.sideDist.x - ray.deltaDist.x);
      else
        ray.perpWallDist = (ray.sideDist.y - ray.deltaDist.y);
      print(ray.perpWallDist, ray.side, x);
    }
    wnoutrefresh(stdscr);
    doupdate();
    // 4. ZAMANLAMA (Timing)
    // --- KRONOMETREYİ DURDUR ---
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Ne kadar süre harcadık? (Mikro saniye cinsinden hesapla)
    long elapsed_us = (end.tv_sec - start.tv_sec) * 1000000 +
                      (end.tv_nsec - start.tv_nsec) / 1000;

    // --- ASIL FPS AYARI BURASI ---
    if (elapsed_us < FRAME_TIME_US)
    {
      // Eğer işimiz hedef süreden erken bittiyse, aradaki fark kadar uyu
      usleep(FRAME_TIME_US - elapsed_us);
    }
  }
  // finish the game
  endwin();
  return 0;
}

void DDA(rayCasting *ray)
{
  while (ray->hit == 0)
  {
    if (ray->sideDist.x < ray->sideDist.y)
    {
      ray->sideDist.x += ray->deltaDist.x;
      ray->mapX += ray->stepX;
      ray->side = 0;
    }
    else
    {
      ray->sideDist.y += ray->deltaDist.y;
      ray->mapY += ray->stepY;
      ray->side = 1;
    }
    // Check if ray has hit a wall
    if (map[ray->mapY][ray->mapX] > 0)
      ray->hit = 1;
  }
};

void print(double wallDist, int side, int whichCol)
{
  int height = (int)(HEIGHT / wallDist);
  int startPoint = HEIGHT / 2 - height / 2;
  int endPoint = HEIGHT / 2 + height / 2;
  if (startPoint < 0)
    startPoint = 0;
  if (endPoint >= HEIGHT)
    endPoint = HEIGHT - 1;
  char drawWith;
  if (side == 0)
    drawWith = '#';
  else
    drawWith = '@';
  int i = 0;

  /* tavan istersen
  while(i<startPoint){
    mvaddch(i,whichCol,'_');
    i++;
  }
  */
  while (startPoint <= endPoint)
  {
    mvaddch(startPoint, whichCol, drawWith);
    startPoint++;
  }
  /*yer istersen
  endPoint++;
  while(endPoint<=HEIGHT-1){
     mvaddch(endPoint,whichCol,'-');
     endPoint++;
  }
  */
};
