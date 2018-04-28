#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <jpeglib.h>

#define IMAGE_HEIGHT 1
#define IMAGE_WIDTH  2
#define IMAGE_BPP    3
#define IMAGE_SIZE   4

typedef unsigned char BYTE;

/* -----------------------------------------------
 * JPEGエラー管理構造体
 */
struct JpegErrorManager
{
  struct jpeg_error_mgr fieldPublic;
  jmp_buf setJmpBuffer;
};
typedef struct JpegErrorManager* JpegErrorManagerPtr;


/* -----------------------------------------------
 * JPEGエラージャンプ関数
 * エラーが発生したときにsetjmp関数に戻る
 */
void exitByLongJump(j_common_ptr pInfo)
{
  JpegErrorManagerPtr pError = (JpegErrorManagerPtr) pInfo->err;
  (*pInfo->err->output_message) (pInfo);
  longjmp(pError->setJmpBuffer, 1);
}


/* -----------------------------------------------
 * JPEG画像読み込み関数
 */
int readJpeg(char *filename,       /* 画像ファイル名 */
	     BYTE **imageBuffer)   /* 画像データ先頭ポインタへのポインタ */
{
  /* 構造体・変数の定義 */
  struct jpeg_decompress_struct cInfo;
  struct JpegErrorManager jpegError;
  JSAMPARRAY rowBuffer;
  FILE *infile = NULL;
  int imageSize = 0;
  int rowStride = 0;
  int pt = 0;

  /* ファイルを開く */
  if ((infile = fopen(filename, "rb")) == NULL)
    {
      fprintf(stderr, "can't open %s\n", filename);
      return 0;
    }
	
  /* JPEG エラー処理の設定 */
  cInfo.err = jpeg_std_error(&jpegError.fieldPublic);
  jpegError.fieldPublic.error_exit = exitByLongJump;
  if (setjmp(jpegError.setJmpBuffer))
    {
      jpeg_destroy_decompress(&cInfo);
      fclose(infile);
      return 0;
    }

  /* JPEGデータの展開オブジェクト生成 */
  jpeg_create_decompress(&cInfo);
	
  /* データ入力の設定（ファイルから） */
  jpeg_stdio_src(&cInfo, infile);

  /* ヘッダー情報の読み込み */
  (void) jpeg_read_header(&cInfo, TRUE);

  /* 画像データ展開の開始 */
  (void) jpeg_start_decompress(&cInfo);

  /* 画像保存領域の確保 */
  rowStride   = cInfo.output_width * cInfo.output_components;
  imageSize   = rowStride * cInfo.output_height;
  *imageBuffer = (JSAMPLE *)malloc(imageSize);
  rowBuffer = (*cInfo.mem->alloc_sarray)
    ((j_common_ptr) &cInfo, JPOOL_IMAGE, rowStride, 1);

  /* 展開された画像データを一行ずつ読み出す */
  pt = 0;
  while (cInfo.output_scanline < cInfo.output_height)
    {
      (void) jpeg_read_scanlines(&cInfo, rowBuffer, 1);
      memcpy(*imageBuffer+pt, *rowBuffer, rowStride);
      pt += rowStride;
    }
	
  /* 画像データ展開処理の終了 */
  (void) jpeg_finish_decompress(&cInfo);
    
  /* 終了 */
  jpeg_destroy_decompress(&cInfo);
  fclose(infile);
  return imageSize;
}


/* -----------------------------------------------
 * JPEGヘッダ情報取得関数
 */
int readHeader(char *filename,   /* 画像ファイル名 */
	       int infoType)     /* 取得する情報の種類 */
{
  /* 構造体・変数の定義 */
  struct jpeg_decompress_struct cInfo;
  struct JpegErrorManager jpegError;
  JSAMPARRAY rowBuffer;
  FILE *infile = NULL;
  int returnValue = 0;
  int rowStride = 0;

  /* ファイルを開く */
  if ((infile = fopen(filename, "rb")) == NULL)
    {
      fprintf(stderr, "can't open %s\n", filename);
      return 0;
    }
	
  /* JPEG エラー処理の設定 */
  cInfo.err = jpeg_std_error(&jpegError.fieldPublic);
  jpegError.fieldPublic.error_exit = exitByLongJump;
  if (setjmp(jpegError.setJmpBuffer))
    {
      jpeg_destroy_decompress(&cInfo);
      fclose(infile);
      return 0;
    }

  /* JPEGデータの展開オブジェクト生成 */
  jpeg_create_decompress(&cInfo);
	
  /* データ入力の設定（ファイルから） */
  jpeg_stdio_src(&cInfo, infile);

  /* ヘッダー情報の読み込み */
  (void) jpeg_read_header(&cInfo, TRUE);

  /* 画像データ展開の開始 */
  (void) jpeg_start_decompress(&cInfo);

  /* 画像保存領域の確保 */
  switch (infoType)
    {
    case IMAGE_HEIGHT:
      returnValue = cInfo.output_height;
      break;
    case IMAGE_WIDTH:
      returnValue = cInfo.output_width;
      break;
    case IMAGE_BPP:
      returnValue = cInfo.output_components;
      break;
    case IMAGE_SIZE:
      returnValue = cInfo.output_height
	* cInfo.output_width * cInfo.output_components;
      break;
    }
  rowStride   = cInfo.output_width * cInfo.output_components;
  rowBuffer = (*cInfo.mem->alloc_sarray)
    ((j_common_ptr) &cInfo, JPOOL_IMAGE, rowStride, 1);

  while (cInfo.output_scanline < cInfo.output_height)
    {
      (void) jpeg_read_scanlines(&cInfo, rowBuffer, 1);
    }
	
  /* 画像データ展開処理の終了 */
  (void) jpeg_finish_decompress(&cInfo);
    
  /* 終了 */
  jpeg_destroy_decompress(&cInfo);
  fclose(infile);
  return returnValue;
}


/* -----------------------------------------------
 * JPEG画像出力関数
 */
int writeJpeg(char *filename,      /* ファイル名 */
	      BYTE *imageBuffer,   /* 画像データ配列の先頭ポインタ */
	      int width,           /* 縱の画素数 */
	      int height,          /* 横の画素数 */
	      int bpp,             /* １画素のバイト数 */
	      int quality)         /* JPEG出力画質(1-100) */
{
  /* 構造体・変数の定義 */
  struct jpeg_compress_struct cInfo;
  struct jpeg_error_mgr jpegError;
  JSAMPROW rowPointer[1];
  FILE *outfile = NULL;
  int rowStride = 0;
  
  /* JPEG エラー処理の設定 */
  cInfo.err = jpeg_std_error(&jpegError);
  
  /* JPEGデータの圧縮オブジェクト生成 */
  jpeg_create_compress(&cInfo);
  
  /* ファイルを開く */
  if ((outfile = fopen(filename, "wb")) == NULL)
    {
      fprintf(stderr, "can't open %s\n", filename);
      return 0;
    }
  
  /* データ出力の設定（ファイルへ書き出し） */
  jpeg_stdio_dest(&cInfo, outfile);
  
  /* ヘッダー情報の設定 */
  cInfo.image_width = width;
  cInfo.image_height = height;
  cInfo.input_components = bpp;
  cInfo.in_color_space = JCS_RGB;
  jpeg_set_defaults(&cInfo);
  jpeg_set_quality(&cInfo, quality, TRUE);

  /* 画像データ圧縮処理の開始 */
  (void) jpeg_start_compress(&cInfo, TRUE);

  /* 画像データを一行ずつ出力する */
  rowStride   = width * bpp;
  while (cInfo.next_scanline < cInfo.image_height) {
    rowPointer[0] = & imageBuffer[cInfo.next_scanline * rowStride];
    (void) jpeg_write_scanlines(&cInfo, rowPointer, 1);
  }
	
  /* 画像データ圧縮処理の終了 */
  (void) jpeg_finish_compress(&cInfo);
    
  /* 終了 */
  fclose(outfile);
  jpeg_destroy_compress(&cInfo);
  return 1;
}
