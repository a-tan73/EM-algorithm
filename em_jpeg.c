/*
 * EMアルゴリズムで画像を領域分けするプログラム
 *  第一引数：入力ファイルパス
 *  第二引数：出力ファイルパス
 *  第三引数：クラス数
 */
#include <stdio.h>
#include <math.h>
#include "jpegio.h"

#define pi 3.14159265359 // パイ
#define REPEAT_NUM 20    // 計算回数
#define C0 -1 / 96
#define H0 1 / 9

// 構造体
typedef struct
{
  int r;
  int g;
  int b;
} rgb_t;

int era = 0;

// 関数プロトタイプ宣言
double like(double, double, double);
double probability(double, double *, double *, double *, int, int);
void em_alg(double *, double *, double *, double *, int, int);
double return_invance(rgb_t, int);

int main(int argc, char **argv)
{
  char *filename = argv[1];  // 入力ファイルパス
  char *filename2 = argv[2]; // 出力ファイルパス
  char *class_num = argv[3]; // クラス数
  BYTE *data = NULL;
  int imageSize = 0;
  int imageWidth = 0;
  int imageHeight = 0;
  int imageBpp = 0;

  int CLASS_NUM = atoi(class_num);
  int DATA_NUM = 3;
  double ave[CLASS_NUM];
  double dis[CLASS_NUM];
  double rat[CLASS_NUM];
  int i, j;

  //invance
  rgb_t rgb;
  double temp;

  // 表示色
  rgb_t color[12];
  color[0].r = 255;
  color[0].g = 0;
  color[0].b = 0;
  color[1].r = 255;
  color[1].g = 127;
  color[1].b = 0;
  color[2].r = 255;
  color[2].g = 255;
  color[2].b = 0;
  color[3].r = 127;
  color[3].g = 255;
  color[3].b = 0;
  color[4].r = 0;
  color[4].g = 255;
  color[4].b = 0;
  color[5].r = 0;
  color[5].g = 255;
  color[5].b = 127;
  color[6].r = 0;
  color[6].g = 255;
  color[6].b = 255;
  color[7].r = 0;
  color[7].g = 127;
  color[7].b = 255;
  color[8].r = 0;
  color[8].g = 0;
  color[8].b = 255;
  color[9].r = 127;
  color[9].g = 0;
  color[9].b = 255;
  color[10].r = 255;
  color[10].g = 0;
  color[10].b = 255;
  color[11].r = 255;
  color[11].g = 0;
  color[11].b = 127;

  // 画像読み込み
  imageSize = readJpeg(filename, &data);
  if (imageSize == 0)
  {
    printf("no image¥n");
    return -1;
  }
  imageWidth = readHeader(filename, IMAGE_WIDTH);
  imageHeight = readHeader(filename, IMAGE_HEIGHT);
  imageBpp = readHeader(filename, IMAGE_BPP);

  //invance
  double i_data[imageSize / 3];

  for (i = 0; i < imageSize / 3; i++)
  {
    rgb.r = data[i * 3];
    rgb.g = data[i * 3 + 1];
    rgb.b = data[i * 3 + 2];

    temp = return_invance(rgb, 2);

    i_data[i] = (temp / pi + 1.0 / 2.0) * 2.55;
  }

  //luminance
  for (i = 0; i < imageSize / 3; i++)
  {
    i_data[i] =
        (0.298912 * data[i * 3] +
         0.586611 * data[i * 3 + 1] +
         0.114478 * data[i * 3 + 2]) /
        100;
  }

  //hue
  /*
  for(i = 0; i < imageSize / 3; i++){
    i_data[i] = ( atan( sqrt(3) * (data[i*3+1] - data[i*3+2])
		      / (2.0 * data[i*3] - data[i*3+1] - data[i*3+2]) ) / pi + 1.0 / 2.0 ) * 2.55;
    if(data[i*3] == data[i*3+1] && data[i*3+1] == data[i*3+2]){
      i_data[i] = 0.0;
    }
    printf("%5d %lf %d %d %d¥n", i, i_data[i], data[i*3], data[i*3+1], data[i*3+2]);
  }
  */

  DATA_NUM = i;

  // EMアルゴリズム実行
  for (i = 0; i < CLASS_NUM; i++)
  {
    ave[i] = (2.50 / CLASS_NUM) * (i + 1);
    dis[i] = 0.05;
    rat[i] = 1.0 / CLASS_NUM;
  }
  em_alg(i_data, ave, dis, rat, DATA_NUM, CLASS_NUM);

  /**
  // チェック
  for (i = 0; i < CLASS_NUM; i++)
  {
    printf("%lf %lf %lf¥n", ave[i], dis[i], rat[i]);
  }
  printf("¥n");
  **/

  // クラス分けした画素に色を付ける
  for (i = 0; i < DATA_NUM; i++)
  {
    for (j = 0; j < CLASS_NUM; j++)
    {
      if (probability(i_data[i], ave, dis, rat, j, CLASS_NUM) >= 0.5)
      {
        data[i * 3] = color[j].r;
        data[i * 3 + 1] = color[j].g;
        data[i * 3 + 2] = color[j].b;
      }
    }
  }

  // 画像出力
  writeJpeg(filename2, data, imageWidth, imageHeight, imageBpp, 100);
  free(data);

  return 0;
}

/*
 * 確率密度関数
 * (1/√2πσ) * e^(-(x-μ)^2/2σ^2) 
 */
double like(double data, double ave, double dis)
{
  double a = sqrt(2 * pi * dis);
  double b = -1 * (data - ave) * (data - ave);
  double c = 2 * dis;

  return (1 / a) * exp(b / c);
}

/*
 * あるクラスに所属する確率を計算する関数
 */
double probability(double data, double *ave, double *dis, double *rat, int class, int CLASS_NUM)
{
  int i;
  double a, b; // a/b に分ける

  a = rat[class] * like(data, ave[class], dis[class]);
  b = 0;
  for (i = 0; i < CLASS_NUM; i++)
  {
    b += rat[i] * like(data, ave[i], dis[i]);
  }

  return a / b;
}

/*
 * EMアルゴリズム実行
 */
void em_alg(double *data, double *ave, double *dis, double *rat, int DATA_NUM, int CLASS_NUM)
{
  double probab_sum_a, probab_sum_b;
  int i, j, k;

  for (k = 0; k < REPEAT_NUM; k++)
  {
    for (i = 0; i < CLASS_NUM; i++)
    {
      // Σの計算
      probab_sum_a = 0;
      probab_sum_b = 0;
      for (j = 0; j < DATA_NUM; j++)
      {
        probab_sum_a += probability(data[j], ave, dis, rat, i, CLASS_NUM) * data[j];
        probab_sum_b += probability(data[j], ave, dis, rat, i, CLASS_NUM);
      }
      // 平均の算出
      ave[i] = probab_sum_a / probab_sum_b;
    }

    for (i = 0; i < CLASS_NUM; i++)
    {
      // Σの計算
      probab_sum_a = 0;
      probab_sum_b = 0;
      for (j = 0; j < DATA_NUM; j++)
      {
        probab_sum_a += probability(data[j], ave, dis, rat, i, CLASS_NUM) * (data[j] - ave[i]) * (data[j] - ave[i]);
        probab_sum_b += probability(data[j], ave, dis, rat, i, CLASS_NUM);
      }
      // 分散の算出
      dis[i] = probab_sum_a / probab_sum_b;
      // 割合の算出
      rat[i] = probab_sum_b / DATA_NUM;
    }
  }
  return;
}

// invance
double return_invance(rgb_t rgb, int cmd)
{
  double e0;
  double e1;
  double e2;
  double h;
  double c;
  double hd;

  e0 = 0.06 * rgb.r + 0.63 * rgb.g + 0.27 * rgb.b;
  e1 = 0.30 * rgb.r + 0.04 * rgb.g - 0.35 * rgb.b;
  e2 = 0.34 * rgb.r - 0.60 * rgb.g + 0.17 * rgb.b;

  h = e1 / e2;
  c = e1 / e0;

  h = atan(h);
  c = atan(c);

  if (c >= atan(C0) && h >= 0)
    hd = 0.5 * h - 0.5 * pi;
  else if (c >= atan(C0) && h < 0)
    hd = 0.5 * h;
  else if (c < atan(C0) && h >= 0)
    hd = 0.5 * h;
  else if (c < atan(C0) && h < 0)
    hd = 0.5 * h + 0.5 * pi;
  else
    hd = pi / 2.0;

  switch (cmd)
  {
  case 0:
    return h;
    break;
  case 1:
    return c;
    break;
  case 2:
    return hd;
    break;
  default:
    return -1;
  }
}
