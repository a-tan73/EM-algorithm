/*
 * EMアルゴリズムのサンプルプログラム
 * 入力されたデータをクラス分けする
 */
#include <stdio.h>
#include <math.h>

#define pi 3.14159265359 // 円周率

#define DATA_NUM 10   // 入力データ数
#define CLASS_NUM 2   // クラス分けする数
#define REPEAT_NUM 20 // 計算回数（多いほど精度があがる）

//関数プロトタイプ宣言
double like(double, double, double);
double probability(double, double *, double *, double *, int);
int calcMaxClass(double *);

int main(void)
{
  double data[DATA_NUM] = {1.48, 1.53, 1.58, 1.72, 1.89, 2.01, 1.51, 1.85, 1.66, 1.80}; // 入力データ
  double ave[CLASS_NUM] = {1.0, 2.0};                                                   // 各クラスの推定平均値
  double dis[CLASS_NUM] = {0.05, 0.05};                                                 // 各クラスの推定分散値
  double rat[CLASS_NUM] = {0.5, 0.5};                                                   // 各クラスの推定比率

  double probab_sum_a, probab_sum_b; // 尤もらしさ

  // 計算回数分ループ
  for (int k = 0; k < REPEAT_NUM; k++)
  {
    // クラス毎に計算
    for (int i = 0; i < CLASS_NUM; i++)
    {
      // Σの計算
      probab_sum_a = 0;
      probab_sum_b = 0;
      for (int j = 0; j < DATA_NUM; j++)
      {
        probab_sum_a += probability(data[j], ave, dis, rat, i) * data[j];
        probab_sum_b += probability(data[j], ave, dis, rat, i);
      }
      // 平均の算出
      ave[i] = probab_sum_a / probab_sum_b;
    }

    for (int i = 0; i < CLASS_NUM; i++)
    {
      // Σの計算
      probab_sum_a = 0;
      probab_sum_b = 0;
      for (int j = 0; j < DATA_NUM; j++)
      {
        probab_sum_a += probability(data[j], ave, dis, rat, i) * (data[j] - ave[i]) * (data[j] - ave[i]);
        probab_sum_b += probability(data[j], ave, dis, rat, i);
      }
      // 分散の算出
      dis[i] = probab_sum_a / probab_sum_b;
      // 割合の算出
      rat[i] = probab_sum_b / DATA_NUM;
    }
  }

  // 結果の出力
  for (int i = 0; i < CLASS_NUM; i++)
  {
    printf("クラス%d　平均値：%lf　", i + 1, ave[i]);
    printf("分散値%lf　", dis[i]);
    printf("割合　%lf\n", rat[i]);
  }
  printf("\n");
  for (int i = 0; i < DATA_NUM; i++)
  {
    double inputNum[CLASS_NUM];
    for (int j = 0; j < CLASS_NUM; j++)
    {
      inputNum[j] = probability(data[i], ave, dis, rat, j);
    }
    int belongClass = calcMaxClass(inputNum);

    printf("入力データ：%lf  クラス：%d\n", data[i], belongClass + 1);
  }

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
double probability(double data, double *ave, double *dis, double *rat, int class)
{
  double a = rat[class] * like(data, ave[class], dis[class]);
  double b = 0;
  for (int i = 0; i < CLASS_NUM; i++)
  {
    b += rat[i] * like(data, ave[i], dis[i]);
  }
  return a / b;
}

/*
 * 入力された値のうち、一番大きい値の順序を返す関数
 */
int calcMaxClass(double inputNum[CLASS_NUM])
{
  double maxNum = 0;
  int maxRow = 0;
  for (int i = 0; i < CLASS_NUM; i++)
  {
    if (maxNum < inputNum[i])
    {
      maxNum = inputNum[i];
      maxRow = i;
    }
  }
  return maxRow;
}