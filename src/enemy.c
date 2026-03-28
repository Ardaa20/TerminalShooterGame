#include "enemy.h"
#include <ncurses.h> // attron, attroff ve mvaddch için gerekli

// DİKKAT: VectorDouble kullandığımız için koordinatlar ekstra { } içine alındı.
Enemy enemys[NUM_ENEMYS] = {
    {{4.5, 2.5}, 0, 'M'},
    {{16.5, 10.5}, 0, 'S'},
    {{10.5, 15.5}, 0, 'B'}};

double ZBuffer[WIDTH];

// Basit bir sıralama fonksiyonu (Düşmanları uzaktan yakına sıralar)
void sortEnemys(int amount)
{
    for (int i = 0; i < amount - 1; i++)
    {
        for (int j = 0; j < amount - i - 1; j++)
        {
            if (enemys[j].distance < enemys[j + 1].distance)
            {
                // Yer değiştir
                Enemy temp = enemys[j];
                enemys[j] = enemys[j + 1];
                enemys[j + 1] = temp;
            }
        }
    }
}

// Düşmanları ekrana basan ana fonksiyon
void drawEnemys(character *player)
{
    // 1. Adım: Tüm düşmanların oyuncuya uzaklığını hesapla
    for (int i = 0; i < NUM_ENEMYS; i++)
    {
        // Basit pisagor
        enemys[i].distance = ((player->position.x - enemys[i].position.x) * (player->position.x - enemys[i].position.x) +
                              (player->position.y - enemys[i].position.y) * (player->position.y - enemys[i].position.y));
    }

    // 2. Adım: Uzaktan yakına doğru sırala
    sortEnemys(NUM_ENEMYS);

    // 3. Adım: Her bir düşmanı ekrana yansıt ve çiz
    for (int i = 0; i < NUM_ENEMYS; i++)
    {
        if (enemys[i].distance > 80)
            continue; // Çok uzaklardaki düşmanları çizme

        // Göreceli konum alıyoruz (düşman ile oyuncu arasındaki fark)
        double EnemyX = enemys[i].position.x - player->position.x;
        double EnemyY = enemys[i].position.y - player->position.y;

        // Kamera matrisi tersi
        double invDet = 1.0 / (player->plane.x * player->dir.y - player->dir.x * player->plane.y);

        double transformX = invDet * (player->dir.y * EnemyX - player->dir.x * EnemyY);
        double transformY = invDet * (-player->plane.y * EnemyX + player->plane.x * EnemyY);

        // Düşmanın ekrandaki X merkezini bul
        int EnemyScreenX = (int)((WIDTH / 2) * (1 + transformX / transformY));

        // Düşman kameranın önündeyse çiz (arkadaysa transformY <= 0 olur)
        if (transformY > 0)
        {
            // 1. O uzaklıktaki duvarın normal boyunu hesapla
            int refWallHeight = abs((int)(HEIGHT / transformY));

            // 2. Duvarın bittiği yeri (Zemini) bul.
            int floorY = HEIGHT / 2 + refWallHeight / 2;

            // 3. Düşmanın boyunu belirle (Örn: Duvarın %75'i kadar olsun)
            int EnemyHeight = (int)(refWallHeight * 0.75);

            // 4. Ayakları zemine basacak şekilde hizala
            int drawEndY = floorY; // Bitiş noktası tam olarak zemin
            if (drawEndY >= HEIGHT)
                drawEndY = HEIGHT - 1;

            int drawStartY = floorY - EnemyHeight; // Başlangıç noktası (Kafa kısmı)
            if (drawStartY < 0)
                drawStartY = 0;

            // Genişliği ayarlama (Terminal karakterleri diktörgen olduğu için boyun yarısı kadar genişlik idealdir)
            int EnemyWidth = EnemyHeight;
            int drawStartX = -EnemyWidth / 2 + EnemyScreenX;
            if (drawStartX < 0)
                drawStartX = 0;

            int drawEndX = EnemyWidth / 2 + EnemyScreenX;
            if (drawEndX >= WIDTH)
                drawEndX = WIDTH - 1;

            // 4. Adım: Z-Buffer kontrolü yaparak ekrana bas
            for (int stripe = drawStartX; stripe < drawEndX; stripe++)
            {
                // ŞARTLAR:
                // 1. Ekranın içinde mi? (stripe > 0 && stripe < WIDTH)
                // 2. Düşman o pikseldeki duvardan YAKIN MI? (transformY < ZBuffer[stripe])
                if (stripe > 0 && stripe < WIDTH && transformY < ZBuffer[stripe])
                {
                    attron(COLOR_PAIR(1)); // Düşmanları kırmızı renkte çiz
                    for (int y = drawStartY; y < drawEndY; y++)
                    {
                        mvaddch(y, stripe, enemys[i].symbol); // Ncurses karakter basma
                    }
                    attroff(COLOR_PAIR(1)); // Çizim bitince rengi kapat
                }
            }
        }
    }
}