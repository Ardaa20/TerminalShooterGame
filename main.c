#include <stdio.h>
#include <ncurses.h>
#include <math.h> //abs fonksiyonu icin yaptik(c de fabs var bu kutuphane icinde floatlari felanda duzgun yapiyor)
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#define MapSize 20
#define HEIGHT 45
#define WIDTH 190
#define DIRLENGTH 1.0   // sakin degistirme bu degeri buna guvenerek matematiksel hesaplamalar yapiliyor(1 olmasina gore)
#define PLANELENGTH 1.0 // fov acisini degistirmek icin bunu degistirebiliriz, bunun boyutuna gore fov acisi degisecek
#define ROTSPEED 0.07
#define PLAYERSPEED 0.05
#define MAXWEAPON 10
#define FPS 60
#define FRAME_TIME_US (1000000 / FPS) // Mikro saniye cinsinden hedef süre
// STRUCTS
typedef struct {
    int width;
    int height;
    char **ascii_art; 
} Weapon;

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
void downloadWeapon();
void cleanMemory();
void printWeapon(int weapon);
void shoot();

Weapon weapons[MAXWEAPON];
int weaponCount = 0;
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
  int currentWeapon=2;
  // burda bir mantik hatirlatmasi karakter aslinda map[y][x] icinde hareket edecek, neden boyle cunki normal matematikte
  // x yatay y dikey eksendir fakat arraylerde tam tersi ve biz obur turlu yaparsak isin icinden cikamayiz butun mat hesaplamalarinda sunda bunda
  downloadWeapon();
  initscr();
  cbreak();
  noecho();              // Klavyede basılan tuşları terminale yazı olarak yazma
  nodelay(stdscr, TRUE); // getch() fonksiyonu tuş beklemesin, oyunu dondurmasın
  keypad(stdscr, TRUE);  // Yön tuşları (oklar) çalışabilsin
  curs_set(0);           // Yanıp sönen terminal imlecini (cursor) gizle
  mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
  printf("\033[?1003h\n");
  fflush(stdout);

  // 1. GİRİŞ (Input)
  // Kullanıcıdan gelen girdileri oku (klavye, fare vb.)
  // main game loop
  rayCasting ray;
  int lastMouseX = -1;
  int gameRunning = 1;
  VectorDouble nextPlayerPosition;
  nextPlayerPosition.x = player.position.x;
  nextPlayerPosition.y = player.position.y;
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
      if (key == 'w')
      {
        nextPlayerPosition.x += PLAYERSPEED * player.dir.x;
        nextPlayerPosition.y += PLAYERSPEED * player.dir.y;
      }
      if (key == 's')
      {
        nextPlayerPosition.x -= PLAYERSPEED * player.dir.x;
        nextPlayerPosition.y -= PLAYERSPEED * player.dir.y;
      }
      if (key == 'a')
      {
        nextPlayerPosition.x += PLAYERSPEED * player.dir.y;
        nextPlayerPosition.y -= PLAYERSPEED * player.dir.x;
      }
      if (key == 'd')
      {
        nextPlayerPosition.x -= PLAYERSPEED * player.dir.y;
        nextPlayerPosition.y += PLAYERSPEED * player.dir.x;
      }
      if (map[(int)nextPlayerPosition.y][(int)nextPlayerPosition.x] == 0)
      {
        player.position.x = nextPlayerPosition.x;
        player.position.y = nextPlayerPosition.y;
      }
      nextPlayerPosition.x = player.position.x;
      nextPlayerPosition.y = player.position.y;
      if (key == 'q')
      { // 'q' tuşuna basınca çık
        gameRunning = 0;
      }
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
            double rotSpeed = deltaX * ROTSPEED;
            double oldDirX = player.dir.x;
            player.dir.x = player.dir.x * cos(rotSpeed) - player.dir.y * sin(rotSpeed);
            player.dir.y = oldDirX * sin(rotSpeed) + player.dir.y * cos(rotSpeed);
            double oldPlaneX = player.plane.x;
            player.plane.x = player.plane.x * cos(rotSpeed) - player.plane.y * sin(rotSpeed);
            player.plane.y = oldPlaneX * sin(rotSpeed) + player.plane.y * cos(rotSpeed);
          }
          lastMouseX = event.x; // Konumu güncelle
        }
      }
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
    printWeapon(currentWeapon);
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
  cleanMemory();
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
  char *shades = "@#%*+=-:. ";
  int shadeIndex = (int)wallDist * 1;
  if (shadeIndex > 9)
    shadeIndex = 8;
  drawWith = *(shades + shadeIndex);
  if (side == 1 && shadeIndex < 9)
  {
    drawWith = *(shades + shadeIndex + 1);
  };
  /* tavan istersen
  int i = 0;
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
     mvaddch(endPoint,whichCol,'~');
     endPoint++;
  }
  */
};

void downloadWeapon() {
    FILE *file = fopen("./weapon_image/weapon1.txt", "r");
    if (file == NULL) {
        printf("ERROR: FILE NOT FOUND!\n");
        return;
    }
    int width, height;
    while (weaponCount < MAXWEAPON && fscanf(file, "w:%d h:%d", &width, &height) == 2) {
        int ch;
        while ((ch = fgetc(file)) != '\n' && ch != EOF);

        weapons[weaponCount].width = width;
        weapons[weaponCount].height = height;
        weapons[weaponCount].ascii_art = (char**) malloc(height * sizeof(char *));
        
        for (int i = 0; i < height; i++) {
            weapons[weaponCount].ascii_art[i] = (char*) malloc(width * sizeof(char));
            int j = 0;
            int current_char;
            while ((current_char = fgetc(file)) != '\n' && current_char != EOF) {
                if (current_char != '\r' && j < width) {
                    weapons[weaponCount].ascii_art[i][j] = current_char;
                    j++;
                }
            }
            while (j < width) {
                weapons[weaponCount].ascii_art[i][j] = ' ';
                j++;
            }
        }
        weaponCount++;
    }
    fclose(file);
}

void cleanMemory() {
    for (int s = 0; s < weaponCount; s++) {
        int h = weapons[s].height;
        for (int i = 0; i < h; i++) {
            free(weapons[s].ascii_art[i]);
        }
        free(weapons[s].ascii_art);
    }
}

void printWeapon(int weapon) {
    if (weaponCount == 0) return;

    int w = weapons[weapon].width;
    int h = weapons[weapon].height;

    int widthStart = (WIDTH / 2) - (w / 2);
    int heightStart = HEIGHT - h;

    for(int i = 0; i < h; i++) {
        for(int j = 0; j < w; j++) {
            char c = weapons[weapon].ascii_art[i][j];
            if(c!=' ') mvaddch(heightStart + i, widthStart + j, c);
        }
    }
}

