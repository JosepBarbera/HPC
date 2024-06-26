#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

int main();
unsigned char *julia_rgb(int w, int h, float xl, float xr, float yb, float yt);
int julia_point(int w, int h, float xl, float xr, float yb, float yt, int i, int j);
void tga_write(int w, int h, unsigned char rgb[], char *filename);

/******************************************************************************/

int main()

/******************************************************************************/
/*
  Purpose:

    MAIN is the main program for JULIA_SET.

  Discussion:

    Consider points (X,Y) in a rectangular domain R = [XL,XR]x[YB,YT].

    Let Z be the complex number X+Yi, and let C be some complex constant.

    Let Z(0) = Z, Z(k+1) = Z(k)^2 + C

    The Julia set is the set of points Z in R with the property that
    the sequence of points Z(k) remain within R.

    To compute a picture of the Julia set, we choose a discrete array
    of WxH points in R.  We carry out up to 200 steps of the iteration for
    each point Z.  If 1000 < |Z| at any time, we assume Z is not in the
    Julia set.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    06 March 2017

  Parameters:

    Local, int H, W, the height and width of the region in pixels.

    Local, float XL, XR, YB, YT, the left and right X limits, the
    bottom and top Y limits, of the region.

    Local, unsigned char *RGB, will hold W*H*3 values between 0 and 255,
    specifying the pixel color values.
*/
{
  int size = 20;
  int h = 1000 * size;
  unsigned char *rgb;
  int w = 1000 * size;
  float xl = -1.5;
  float xr = +1.5;
  float yb = -1.5;
  float yt = +1.5;
  double begin = omp_get_wtime();

  printf("\n");
  printf("JULIA_SET:\n");
  printf("  C version.\n");
  printf("  Plot a version of the Julia set for Z(k+1)=Z(k)^2-0.8+0.156i\n");

  rgb = julia_rgb(w, h, xl, xr, yb, yt);

  tga_write(w, h, rgb, "julia_set.tga");
  /*
    Free memory.
  */
  free(rgb);
  /*
    Terminate.
  */
  double end = omp_get_wtime();
  double time_spent = (double)(end - begin);
  printf("\n");
  printf("JULIA_SET:\n");
  printf("Normal end of execution.\n");
  printf("Execution time %f\n", time_spent);
  return 0;
}
/******************************************************************************/

unsigned char *julia_rgb(int w, int h, float xl, float xr, float yb, float yt)

/******************************************************************************/
/*
  Purpose:

    JULIA_RGB applies JULIA to each point in the domain.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    06 March 2017

  Parameters:

    Input, int W, H, the width and height of the region in pixels.

    Input, float XL, XR, YB, YT, the left, right, bottom and top limits.

    Output, unsigned char *JULIA_SET[W*H*3], the B, G, R values,
    between 0 and 255, of a plot of the Julia set.  We want
    [0,0,255] for points in the set (red), and [255,255,255]
    for points not in the set (white).
*/
{
  int j, i, k;
  unsigned char *rgb;
  rgb = (unsigned char *)malloc(w * h * 3 * sizeof(unsigned char));
/*Setting the parallel region for each loop, each one will have the private variables (i,j,k)
and also using dynamic scheduling to distribute the work through the loops. */ 
#pragma omp parallel for private(i, j, k) schedule(dynamic) 
  for (j = 0; j < h; ++j)
  {
    for (i = 0; i < w; ++i)
    {
      int juliaValue = julia_point(w, h, xl, xr, yb, yt, i, j);

      // Calculate the index k in the rgb array for the current pixel (i,j)
      k = (j * w + i) * 3; 
      rgb[k] = 255 * (1 - juliaValue);
      rgb[k + 1] = 255 * (1 - juliaValue);
      rgb[k + 2] = 255;
    }
  }

  return rgb;
}

/******************************************************************************/

int julia_point(int w, int h, float xl, float xr, float yb, float yt, int i, int j)

/******************************************************************************/
/*
  Purpose:

    JULIA_POINT returns 1 if a point is in the Julia set.

  Discussion:

    The iteration Z(k+1) = Z(k) + C is used, with C=-0.8+0.156i.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    06 March 2017

  Parameters:

    Input, int W, H, the width and height of the region in pixels.

    Input, float XL, XR, YB, YT, the left, right, bottom and top limits.

    Input, int I, J, the indices of the point to be checked.

    Ouput, int JULIA, is 1 if the point is in the Julia set.
*/
{
  float ai;
  float ar;
  float ci = 0.156;
  float cr = -0.8;
  int k;
  float t;
  float x;
  float y;
  /*
    Convert (I,J) indices to (X,Y) coordinates.
  */
  x = ((float)(w - i - 1) * xl + (float)(i)*xr) / (float)(w - 1);

  y = ((float)(h - j - 1) * yb + (float)(j)*yt) / (float)(h - 1);
  /*
    Think of (X,Y) as real and imaginary components of
    a complex number A = x + y*i.
  */
  ar = x;
  ai = y;
  /*
    A -> A * A + C
  */
  for (k = 0; k < 200; k++)
  {
    t = ar * ar - ai * ai + cr;
    ai = ar * ai + ai * ar + ci;
    ar = t;
    /*
      if 1000 < ||A||, reject the point.
    */
    if (1000 < ar * ar + ai * ai)
    {
      return 0;
    }
  }

  return 1;
}
/******************************************************************************/

void tga_write(int w, int h, unsigned char rgb[], char *filename)

/******************************************************************************/
/*
  Purpose:

    TGA_WRITE writes a TGA or TARGA graphics file of the data.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    06 March 2017

  Parameters:

    Input, int W, H, the width and height of the image.

    Input, unsigned char RGB[W*H*3], the pixel data.

    Input, char *FILENAME, the name of the file to contain the screenshot.
*/
{
  FILE *file_unit;
  unsigned char header1[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  unsigned char header2[6] = {w % 256, w / 256, h % 256, h / 256, 24, 0};
  /*
    Create the file.
  */
  file_unit = fopen(filename, "wb");
  /*
    Write the headers.
  */
  fwrite(header1, sizeof(unsigned char), 12, file_unit);
  fwrite(header2, sizeof(unsigned char), 6, file_unit);
  /*
    Write the image data.
  */
  fwrite(rgb, sizeof(unsigned char), 3 * w * h, file_unit);
  /*
    Close the file.
  */
  fclose(file_unit);

  printf("\n");
  printf("TGA_WRITE:\n");
  printf("  Graphics data saved as '%s'\n", filename);

  return;
}
