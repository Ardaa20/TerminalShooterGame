#include <stdio.h> 
#include <ncurses.h>
#include <math.h> //abs fonksiyonu icin yaptik(c de fabs var bu kutuphane icinde floatlari felanda duzgun yapiyor)
#define MapSize 20
#define HEIGHT 45
#define WIDTH 190
#define DIRLENGTH 1.0  // sakin degistirme bu degeri buna guvenerek matematiksel hesaplamalar yapiliyor(1 olmasina gore)
#define PLANELENGTH 1.0 //fov acisini degistirmek icin bunu degistirebiliriz, bunun boyutuna gore fov acisi degisecek
#define ROTSPEED 0.07
#define PLAYERSPEED 0.05
//STRUCTS
typedef struct{
  float x;
  float y;
}Vector;

typedef  struct{
  double x;
  double y;
}VectorDouble;

typedef struct{
  Vector position;
  Vector moveSpeed; 
  Vector dir;
  Vector plane;
}character;

typedef struct {
    VectorDouble rayDir;       // asil olaylardan yon
    VectorDouble deltaDist;    // atlama mesafesi
    VectorDouble sideDist;     // ilk cizgiye uzaklik
    int mapX;            
    int mapY;            // haritadaki anlik konum
    int stepX;           // sag sol 
    int stepY;           // asagi yukari
    int hit;             // hit the wall
    int side;            // X duvarı mı, Y duvarı mı? (0 veya 1)
    float perpWallDist;  // fisheye cozumu burda eger siteyi okursan(yani fazladan bir plane felan ypk bizde bura zaten cozuyor(hatta tam bura degilde onceden yaptigimiz 
                         // matematiksel islemler cozuyor))
}rayCasting;


//function
void DDA(rayCasting *ray);
void print(double wallDist,int side,int whichCol);

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
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

int main(){
  character player;
  player.dir.x=DIRLENGTH;
  player.dir.y=0;
  player.plane.x=0;
  player.plane.y=PLANELENGTH;
  player.position.x=1.5;
  player.position.y=4.5;
  // burda bir mantik hatirlatmasi karakter aslinda map[y][x] icinde hareket edecek, neden boyle cunki normal matematikte 
  // x yatay y dikey eksendir fakat arraylerde tam tersi ve biz obur turlu yaparsak isin icinden cikamayiz butun mat hesaplamalarinda sunda bunda

  initscr();
  cbreak();
  noecho();             // Klavyede basılan tuşları terminale yazı olarak yazma
  nodelay(stdscr, TRUE); // getch() fonksiyonu tuş beklemesin, oyunu dondurmasın
  keypad(stdscr, TRUE);  // Yön tuşları (oklar) çalışabilsin
  curs_set(0);          // Yanıp sönen terminal imlecini (cursor) gizle
  printf("\033[?1003h\n");
  fflush(stdout);
  mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
  //main game loop 
  rayCasting ray;
  int lastMouseX=-1;
  int gameRunning = 1;
  while(gameRunning) {
    int key = getch();
    if (key == 'w') {
      player.position.x+=PLAYERSPEED * player.dir.x;
      player.position.y+=PLAYERSPEED * player.dir.y;
    }
    if (key == 's') {
      player.position.x-=PLAYERSPEED * player.dir.x;
      player.position.y-=PLAYERSPEED * player.dir.y;
    }
    if (key == 'a'){
      player.position.x+=PLAYERSPEED * player.dir.y;
      player.position.y-=PLAYERSPEED * player.dir.x;
    }
    if (key == 'd'){
      player.position.x-=PLAYERSPEED * player.dir.y;
      player.position.y+=PLAYERSPEED * player.dir.x;
    }
    if (key == 'q') {  // 'q' tuşuna basınca çık
        gameRunning = 0; 
    }
    if (key == KEY_MOUSE)
    {
      MEVENT event;
      if (getmouse(&event) == OK)
      {
        if (lastMouseX == -1) lastMouseX = event.x;          
          int deltaX = event.x - lastMouseX; 
          if (deltaX != 0) {
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
    clear();
    for(int x = 0; x<WIDTH ; x++){
      double cameraX = 2 * x / (double) WIDTH - 1; //x-coordinate in camera space
      ray.rayDir.x = player.dir.x + player.plane.x * cameraX;
      ray.rayDir.y= player.dir.y + player.plane.y * cameraX;
      ray.mapX = (int) player.position.x;
      ray.mapY = (int) player.position.y;
      ray.deltaDist.x = (ray.rayDir.x==0) ? 1e30 : fabs(1/ray.rayDir.x);
      ray.deltaDist.y = (ray.rayDir.y==0) ? 1e30 : fabs(1/ray.rayDir.y);
      ray.hit=0;
      if (ray.rayDir.x < 0)
      {
        ray.stepX = -1;
        ray.sideDist.x = (player.position.x - ray.mapX) * ray.deltaDist.x;
      }
      else
      {
        ray.stepX = 1;
        ray.sideDist.x  = (ray.mapX + 1.0 - player.position.x) * ray.deltaDist.x;
      }
      if (ray.rayDir.y < 0)
      {
        ray.stepY  = -1;
        ray.sideDist.y  = (player.position.y - ray.mapY) * ray.deltaDist.y;
      }
      else
      {
        ray.stepY = 1;
        ray.sideDist.y  = (ray.mapY + 1.0 - player.position.y) * ray.deltaDist.y;
      }
      DDA(&ray);
      if(ray.side == 0) ray.perpWallDist = (ray.sideDist.x - ray.deltaDist.x);
      else              ray.perpWallDist = (ray.sideDist.y - ray.deltaDist.y);
      print(ray.perpWallDist,ray.side,x);
    }

    refresh();
    napms(16);
  }
  //finish the game
  printf("\033[?1003l\n"); // Fare izlemeyi KAPAT (Sonundaki harf küçük L)
  fflush(stdout);
  endwin();
  return 0;
}

void DDA(rayCasting *ray){
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
    //Check if ray has hit a wall
    if (map[ray->mapY][ray->mapX] > 0) ray->hit = 1;
  } 
};

void print(double wallDist, int side, int whichCol){
  int height = (int) (HEIGHT / wallDist);
  int startPoint = HEIGHT/2 - height/2;
  int endPoint = HEIGHT/2 + height/2;
  if(startPoint<0) startPoint = 0;
  if(endPoint>=HEIGHT) endPoint = HEIGHT-1;
  char drawWith;
  if(side==0) drawWith = '#';
  else        drawWith = '@';
  /* tavan istersen 
  int i = 0;
  while(i<startPoint){
    mvaddch(i,whichCol,'_'); 
    i++;  
  }
  */
  while(startPoint<=endPoint){
    mvaddch(startPoint,whichCol,drawWith);
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




