#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <math.h> //abs fonksiyonu icin yaptik(c de fabs var bu kutuphane icinde floatlari felanda duzgun yapiyor)
#include <unistd.h>
#include <time.h>
#include "player.h"
#include "vector.h"
#include "map.h"
#include "weapon.h"
#include "raycaster.h"

#define DIRLENGTH 1.0   // sakin degistirme bu degeri buna guvenerek matematiksel hesaplamalar yapiliyor(1 olmasina gore)
#define PLANELENGTH 1.0 // fov acisini degistirmek icin bunu degistirebiliriz, bunun boyutuna gore fov acisi degisecek
#define MAXWEAPON 10
#define FPS 60
#define FRAME_TIME_US (1000000 / FPS) // Mikro saniye cinsinden hedef süre
// STRUCTS

int main()
{
  character player;
  // player dir ile plane dir dik olucak ondan dirx 1 ise planex = 0 olacak, burdan sonra plane.y ye koyduğum değer benim fov-umu belirleyecek 1 olursa mesela 90 derece fov olur, 0.5 olursa 60 derece gibi gibi. Şuan fov 90.
  player.dir.x = DIRLENGTH;
  player.dir.y = 0;
  player.plane.x = 0;
  player.plane.y = PLANELENGTH;
  player.position.x = 1.5;
  player.position.y = 4.5;
  int currentWeapon = 2;
  // burda bir mantik hatirlatmasi karakter aslinda map[y][x] icinde hareket edecek, neden boyle cunki normal matematikte
  // x yatay y dikey eksendir fakat arraylerde tam tersi ve biz obur turlu yaparsak isin icinden cikamayiz butun mat hesaplamalarinda sunda bunda
  downloadWeapon();
  printf("\e[8;45;190t");
  fflush(stdout);
  initscr();
  cbreak();
  noecho();              // Klavyede basılan tuşları terminale yazı olarak yazma
  nodelay(stdscr, TRUE); // getch() fonksiyonu tuş beklemesin, oyunu dondurmasın
  keypad(stdscr, TRUE);  // Yön tuşları (oklar) çalışabilsin
  curs_set(0);           // Yanıp sönen terminal imlecini (cursor) gizle
  start_color();         // Renk motorunu çalıştır!
  // init_pair(Palet_No, Yazı_Rengi, Arka_Plan_Rengi);
  init_pair(1, COLOR_RED, COLOR_BLACK);    // 1 Numaralı Palet: Kırmızı yazı, Siyah arka plan
  init_pair(2, COLOR_GREEN, COLOR_BLACK);  // 2 Numaralı Palet: Yeşil yazı, Siyah arka plan
  init_pair(3, COLOR_YELLOW, COLOR_BLACK); // 3 Numaralı Palet: Sarı yazı, Siyah arka plan
  init_pair(4, COLOR_CYAN, COLOR_BLACK);   // 4 Numaralı Palet: Açık Mavi yazı, Siyah arka plan (Duvarlar için harika olur)

  mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
  printf("\033[?1003h\n");
  fflush(stdout);
  while (COLS < WIDTH || LINES < HEIGHT)
  {
    clear();
    // Ekranın tam ortasına uyarı yazdırıyoruz
    mvprintw(LINES / 2, (COLS - 45) / 2, "PLEASE INCREASE THE SCREEN SIZE!");
    mvprintw((LINES / 2) + 1, (COLS - 45) / 2, "REQUIRED: %dx%d | PRESENT: %dx%d", WIDTH, HEIGHT, COLS, LINES);
    mvprintw((LINES / 2) + 3, (COLS - 45) / 2, "PRESS 'q' FOR EXIT");

    refresh();
    int key = getch();
    if (key == 'q')
    {
      endwin();
      return 0; // Oyunu kapat
    }
    usleep(10000);
  }

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
        MEVENT event; // Burdaki event ekrandaki farenin hangi pozisyonda olduğunu ve hangi tuşa basıldığını tutacak yapı
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
          if (event.bstate & BUTTON1_PRESSED || event.bstate & BUTTON1_CLICKED)
          {
            shooting = 15;
          }
        }
      }
    }
    // 2. GÜNCELLEME (Update)
    // Oyun mantığını burada işlet (örneğin, karakter hareketi, çarpışma kontrolü, yapay zeka güncellemeleri vb.)
    werase(stdscr);

    // 3. ÇİZİM (Render)
    // Oyun dünyasını ekrana çiz (örneğin, karakterler, duvar
    renderFrame(&player);

    printWeapon(&weapons[currentWeapon]);
    if (shooting)
    {
      shoot(&weapons[currentWeapon]);
      shooting--;
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
  cleanMemory();
  return 0;
}
