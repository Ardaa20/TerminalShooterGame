#include <stdio.h>
#include <ncurses.h>
#include <math.h> //abs fonksiyonu icin yaptik(c de fabs var bu kutuphane icinde floatlari felanda duzgun yapiyor)
#include <unistd.h>
#include <time.h>
#include "player.h"
#include "vector.h"
#include "map.h"

#define DIRLENGTH 1.0   // sakin degistirme bu degeri buna guvenerek matematiksel hesaplamalar yapiliyor(1 olmasina gore)
#define PLANELENGTH 1.0 // fov acisini degistirmek icin bunu degistirebiliriz, bunun boyutuna gore fov acisi degisecek

#define FPS 200
#define FRAME_TIME_US (1000000 / FPS) // Mikro saniye cinsinden hedef süre
// STRUCTS

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
  mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
  printf("\033[?1000l");
  printf("\033[?1003h\n");
  fflush(stdout);

  // 1. GİRİŞ (Input)
  // Kullanıcıdan gelen girdileri oku (klavye, fare vb.)
  // main game loop
  rayCasting ray;
  int lastMouseX = -1;
  int gameRunning = 1;
  struct timespec start, end;
  long diff_ms;
  while (gameRunning)
  {

    // 1. GİRİŞ (Input)
    // Kullanıcıdan gelen girdileri oku (klavye, fare vb.)
    clock_gettime(CLOCK_MONOTONIC, &start);
    int key;
    while ((key = getch()) != ERR)
    {

      movePlayer(&player, key);

      if (key == 'q')
      { // 'q' tuşuna basınca çık
        gameRunning = 0;
      }

      // Fare hareketlerini işlemek için
      if (key == KEY_MOUSE)
      {
        MEVENT event;
        if (getmouse(&event) == OK)
        {
          if (lastMouseX == -1)
            lastMouseX = event.x;
          int deltaX = event.x - lastMouseX;
          if (deltaX != 0)
          {
            rotatePlayer(&player, deltaX);
          }
          lastMouseX = event.x; // Konumu güncelle
        }
      }
    }
    // 2. GÜNCELLEME (Update)
    // Oyun mantığını burada işlet (örneğin, karakter hareketi, çarpışma kontrolü, yapay zeka güncellemeleri vb.)
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

      printMap(ray.perpWallDist, ray.side, x);
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
  printf("\033[?1003l\n"); // Fare izlemeyi KAPAT (Sonundaki harf küçük L)
  fflush(stdout);
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
