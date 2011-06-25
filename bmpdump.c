/***********************************************************/
/* bitmap データ ダンププログラム                          */
/*                                                         */
/* 4, 8, 24, 32 bit Bitmap に対応                          */
/*                                                         */
/* 出力データ                                              */
/* R(赤)値 G(緑)値 B(青)値                                 */
/*                                                         */
/* 2000.08.03                                              */
/* kondo - at - kk.iij4u.or.jp                             */
/*                                                         */
/* 2011.06.25                                              */
/* http://www.kk.iij4u.or.jp/~kondo/bmp/                   */
/* から拝借したソースをcolor LCD Shieldでそのまま使える    */
/* 形式でdumpする様にちょこっと改造                        */
/* Twitter: ytsuboi                                        */
/***********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void usage(char *cmdname)
{
  fprintf(stderr,
          "\n"
          "ビットマップファイル ---> テキストダンプ プログラム\n"
          "Usage : %s bmp_file [text_file]\n"
          "  bmp_file  : ビットマップファイル\n"
          "              (無圧縮 4, 8, 24, 32bit のみ対応)\n"
          "  text_file : 指定しない場合 \"bmp_file.txt\" が出力されます\n"
          "\n"
          "  出力形式 : 各色 0～255\n"
          "\tRed Green Blue\n"
          "\tRed Green Blue\n"
          "\t :    :    :\n"
          "\n"
          "  画像のサイズによってダンプファイルがかなり大きくなるので\n"
          "  注意してください\n" "\n", cmdname);
  exit(1);
}

/*----------------*/
/* ファイル名取得 */
/*----------------*/
int getbasename(char *dest, char *src)
{
  int i, start, end, ret;

  i = -1;
  start = 0;
  end = 0;

  // ファイル名のはじめと終わりを検出
  while (src[++i]) {
    if (src[i] == '\\' || src[i] == ':') {
      start = i + 1;
      end = 0;
    }
    if (src[i] == '.') {
      end = i;
    }
  }
  if (end == 0) {
    end = i;
  }
  // ファイル名が有る場合
  if (start < end) {
    for (i = 0; i < end; i++) {
      dest[i] = src[i];
    }
    dest[i] = '\0';
    ret = 1;
  } 
  else {
    dest[0] = '\0';
    ret = 0;
  }

  return ret;
}

/*----------------------------*/
/* ファイルヘッダ部 (14 Byte) */
/*----------------------------*/
int fileheader(FILE * fp, long *offset)
{
  int count = 0;
  long var_long;
  short var_short;
  char s[10];

  // BITMAP 認識文字 "BM"
  if (fread(s, 2, 1, fp) == 1) {
    if (memcmp(s, "BM", 2) == 0) {
      printf("[BM] BITMAP file\n");
    } 
    else {
      fprintf(stderr, "%s : Not a BITMAP file\n", s);
      exit(1);
    }
    count += 2;
  }
  printf("  [BITMAPFILEHEADER]\n");
  // ファイルサイズ
  if (fread(&var_long, 4, 1, fp) == 1) {
    printf("  Size          : %ld [Byte]\n", var_long);
    count += 4;
  }
  // 予約領域 0
  if (fread(&var_short, 2, 1, fp) == 1) {
    count += 2;
  }
  // 予約領域 0
  if (fread(&var_short, 2, 1, fp) == 1) {
    count += 2;
  }
  // ファイルの先頭から画像データまでの位置
  if (fread(&var_long, 4, 1, fp) == 1) {
    printf("  OffBits       : %ld [Byte]\n", var_long);
    *offset = var_long;
    count += 4;
  }
  return count;
}

/*------------------------------------------------------------------*/
/* 情報ヘッダ部 (12 Byte -> OS/2 Bitmap, 40 Byte -> Windows Bitmap) */
/*------------------------------------------------------------------*/
int infoheader(FILE * fp,
               short *infosize,
               long *width, long *height,
               long *x_coodinate, long *y_coodinate,
               short *BitCount, 
               long *ClrUsed)
{
  int count = 0;
  long var_long, compress = 0;
  short var_short;

  // BITMAPINFOHEADER のサイズ
  if (fread(&var_long, 4, 1, fp) == 1) {
    count += 4;
    *infosize = var_long;
  }

  printf("  [BITMAPINFOHEADER]\n");
  // OS/2 Bitmap
  if (*infosize == 12) {
    // 画像データの幅
    if (fread(&var_short, 2, 1, fp) == 1) {
      printf("  Width         : %d [pixel]\n", var_short);
      *width = var_short;
      count += 2;
    }
    // 画像データの高さ 
    if (fread(&var_short, 2, 1, fp) == 1) {
      printf("  Height        : %d [pixel]\n", var_short);
      *height = var_short;
      count += 2;
    }
    // プレーン数 (1のみ) 
    if (fread(&var_short, 2, 1, fp) == 1) {
      count += 2;
    }
    // 1画素あたりのビット数 (1, 4, 8, 24, 32) 
    if (fread(&var_short, 2, 1, fp) == 1) {
      printf("  BitCount      : %d [bit]\n", var_short);
      *BitCount = var_short;
      count += 2;
    }
  }
  // Windows BMP 
  else if (*infosize == 40) {
    // 画像データの幅 
    if (fread(&var_long, 4, 1, fp) == 1) {
      printf("  Width         : %ld [pixel]\n", var_long);
      *width = var_long;
      count += 4;
    }
    // 画像データの高さ
    if (fread(&var_long, 4, 1, fp) == 1) {
      printf("  Height        : %ld [pixel]\n", var_long);
      *height = var_long;
      count += 4;
    }
    // プレーン数 (1のみ) 
    if (fread(&var_short, 2, 1, fp) == 1) {
      count += 2;
    }
    // 1画素あたりのビット数 (1, 4, 8, 24, 32) 
    if (fread(&var_short, 2, 1, fp) == 1) {
      printf("  BitCount      : %d [bit]\n", var_short);
      *BitCount = var_short;
      count += 2;
    }
    // 圧縮方式  0 : 無圧縮 
    //           1 : BI_RLE8 8bit RunLength 圧縮 
    //           2 : BI_RLE4 4bit RunLength 圧縮 
    if (fread(&var_long, 4, 1, fp) == 1) {
      printf("  Compression   : %ld\n", var_long);
      compress = var_long;
      count += 4;
    }
    // 画像データのサイズ 
    if (fread(&var_long, 4, 1, fp) == 1) {
      printf("  SizeImage     : %ld [Byte]\n", var_long);
      count += 4;
    }
    // 横方向解像度 (Pixel/meter) 
    if (fread(&var_long, 4, 1, fp) == 1) {
      printf("  XPelsPerMeter : %ld [pixel/m]\n", var_long);
      *x_coodinate = var_long;
      count += 4;
    }
    // 縦方向解像度 (Pixel/meter) 
    if (fread(&var_long, 4, 1, fp) == 1) {
      printf("  YPelsPerMeter : %ld [pixel/m]\n", var_long);
      *y_coodinate = var_long;
      count += 4;
    }
    // 使用色数 
    if (fread(&var_long, 4, 1, fp) == 1) {
      printf("  ClrUsed       : %ld [color]\n", var_long);
      *ClrUsed = var_long;
      count += 4;
    }
    // 重要な色の数 0の場合すべての色 
    if (fread(&var_long, 4, 1, fp) == 1) {
      count += 4;
    }
  } 
  else {
    fprintf(stderr, "Bitmap Info Header error\n");
  }

  if (compress != 0) {
    fprintf(stderr, "圧縮ビットマップには対応していません\n");
    exit(1);
  }
  if (*BitCount == 4 || *BitCount == 8 || *BitCount == 24 || *BitCount == 32) {
    ;
  } 
  else {
    fprintf(stderr, "%d ビット色には対応していません\n", *BitCount);
    exit(1);
  }
  return count;
}

/*------------------*/
/* 使用色数カウント */
/*------------------*/
long usedcolor(short bits, long color)
{
  if (color != 0) {
    return color;
  } 
  else {
    return 1 << bits;
  }
}

/*-------------*/
/* OS/2 bitmap */
/*-------------*/
int rgbtriple(FILE * fp,
              long used,
              unsigned char *red,
              unsigned char *green, 
              unsigned char *blue)
{
  long i;
  int count = 0;

  // ビットの並びは B G R 
  for (i = 0; i < used; i++) {
    blue[i] = fgetc(fp);
    green[i] = fgetc(fp);
    red[i] = fgetc(fp);
    count++;
  }

  return count;
}

/*----------------*/
/* Windows bitmap */
/*----------------*/
int rgbquad(FILE * fp,
            long used,
            unsigned char *red, 
            unsigned char *green, 
            unsigned char *blue)
{
  long i;
  int receive, count = 0;

  // ビットの並びは B G R 予約 
  for (i = 0; i < used; i++) {
    blue[i] = fgetc(fp);
    green[i] = fgetc(fp);
    red[i] = fgetc(fp);
    receive = fgetc(fp);
    count++;
  }
  return count;
}

/*----------------*/
/* テキストダンプ */
/*----------------*/
long dump32(FILE * bmp,
            FILE * txt,
            long offset,
            long width,
            long height,
            short bits,
            unsigned char *red, 
            unsigned char *green, 
            unsigned char *blue)
{
  int r, g, b, alpha;
  long i, j, line, position, count = 0;

  // 4byte 境界にあわせる 
  line = (width * bits) / 8;
  if ((line % 4) != 0) {
    line = ((line / 4) + 1) * 4;
  }

  for (i = 0; i < height; i++) {
    // 行の先頭位置 
    position = offset + line * (height - (i + 1));
    fseek(bmp, position, SEEK_SET);

    // 値の読み込み 
    for (j = 1; j <= width; j++) {
      // 32 bit 色 
      if (bits == 32) {
        b = fgetc(bmp);
        g = fgetc(bmp);
        r = fgetc(bmp);
        alpha = fgetc(bmp);
        fprintf(txt, "%d,%d,%d, ", r, g, b);
        count++;
      }
      // 24 bit 色 
      else if (bits == 24) {
        b = fgetc(bmp);
        g = fgetc(bmp);
        r = fgetc(bmp);
        fprintf(txt, "%d,%d,%d, ", r, g, b);
        count++;
      }
      // 8 bit 色 
      else if (bits == 8) {
        int index;

        index = fgetc(bmp);
        fprintf(txt, "%d,%d,%d, ", red[index], green[index], blue[index]);
        count++;
      }

     if (fmod(j,10)==0) { fprintf(txt, "\n"); }

    }
    fprintf(txt, "\n");
  }
  return count;
}

long dump4(FILE * bmp,
           FILE * txt,
           long offset,
           long width,
           long height,
           short bits,
           unsigned char *red, 
           unsigned char *green, 
           unsigned char *blue)
{
  long i, j, line, position, count = 0;

  // 4byte 境界にあわせる 
  line = (width * bits) / 8;
  if ((line % 4) != 0) {
    line = ((line / 4) + 1) * 4;
  }

  for (i = 0; i < height; i++) {
    // 行の先頭位置 
    position = offset + line * (height - (i + 1));
    fseek(bmp, position, SEEK_SET);
    // 値の読み込み 
    for (j = 0; j < (int) ((double) width / 2.0 + 0.5); j++) {
      int index, high, low;

      index = fgetc(bmp);
      // 上位 4bit 
      high = index >> 4;
      // 下位 4bit 
      low = index & 15;
      fprintf(txt, "%d\t%d\t%d\n%d\t%d\t%d\n",
              red[high], green[high], blue[high],
              red[low], green[low], blue[low]);
      count += 2;
    }
    fprintf(txt, "\n");
  }
  return count;
}

/*------------*/
/* ダンプ処理 */
/*------------*/
void dumpprocess(char *bmpfile, char *textfile)
{
  short infosize, bits;
  long used = 0, color = 0, offset, width, height, xreso, yreso;
  unsigned char red[256], green[256], blue[256];
  FILE *fp1, *fp2;

  if ((fp1 = fopen(bmpfile, "rb")) == NULL) {
    fprintf(stderr, "Cannot open file : %s\n", bmpfile);
    exit(1);
  }
  // ヘッダ情報 BITMAPFILEHEADER 
  fileheader(fp1, &offset);
  // ヘッダ情報 BITMAPINFOHEADER 
  infoheader(fp1, &infosize, &width, &height, &xreso, &yreso, &bits, &color);
  color = usedcolor(bits, color);

  // OS/2 Bitmap 
  if (infosize == 12) {
    if (bits == 1 || bits == 4 || bits == 8) {
      used = rgbtriple(fp1, color, red, green, blue);
    }
    printf("[OS/2 bitmap] --- %d bit %ld color\n", bits, used);
  }
  // Windows Bitmap 
  else if (infosize == 40) {
    if (bits == 1 || bits == 4 || bits == 8) {
      used = rgbquad(fp1, color, red, green, blue);
    }
    printf("[Windows bitmap] --- %d bit %ld color\n", bits, used);
  } 
  else if (infosize == 108) {
    printf("[other bitmap]\n");
  }
  else {
    fprintf(stderr, "BITMAP INFOHEADER error\n");
  }

  if ((fp2 = fopen(textfile, "w")) == NULL) {
    fprintf(stderr, "Cannot open file : %s\n", textfile);
    exit(1);
  }
  // テキスト出力 
  if (bits == 32 || bits == 24 || bits == 8) {
    printf("%ld [pixels]\n",
           dump32(fp1, fp2, offset, width, height, bits, red, green, blue));
  }
  else if (bits == 4) {
    printf("%ld [pixels]\n",
           dump4(fp1, fp2, offset, width, height, bits, red, green, blue));
  }
  else {
    fprintf(stderr, "%d bit 色には対応していません\n", bits);
    exit(1);
  }

  fclose(fp1);
  fclose(fp2);
}

/**************/
/* メイン関数 */
/**************/
int main(int argc, char *argv[])
{
  char txtfile[255], bmpfile[255];

  if (argc < 2) {
    usage(argv[0]);
  }
  else if (argc == 2) {
    char basename[255];

    strcpy(bmpfile, argv[1]);
    getbasename(basename, argv[1]);
    sprintf(txtfile, "%s.txt", basename);
  } 
  else if (argc == 3) {
    strcpy(bmpfile, argv[1]);
    strcpy(txtfile, argv[2]);
  } 
  else {
    usage(argv[0]);
  }

  dumpprocess(bmpfile, txtfile);

  return 0;
}

