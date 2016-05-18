/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be
 *  found  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include <assert.h>

#include "vp10/common/motion_model.h"
#include "vp10/common/blockd.h"

#define MAX_PARAMDIM 9

static const double TINY_NEAR_ZERO = 1.0E-12;


static inline double SIGN(double a, double b) {
  return ((b) >= 0 ? fabs(a) : -fabs(a));
}

static inline double PYTHAG(double a, double b) {
  double absa, absb, ct;
  absa = fabs(a);
  absb = fabs(b);

  if(absa > absb) {
    ct = absb / absa;
    return absa * sqrt(1.0 + ct * ct);
  } else {
    ct = absa / absb;
    return (absb == 0) ? 0 : absb * sqrt(1.0 + ct * ct);
  }
}

int IMIN(int a, int b) {
  return (((a) < (b)) ? (a) : (b));
}

int IMAX(int a, int b) {
  return (((a) < (b)) ? (b) : (a));
}

void MultiplyMat(double *m1, double *m2, double *res,
                 const int M1, const int N1, const int N2) {
  int timesInner = N1;
  int timesRows = M1;
  int timesCols = N2;
  double sum;

  int row, col, inner;
  for( row = 0; row < timesRows; ++row ) {
    for( col = 0; col < timesCols; ++col ) {
      sum = 0;
      for (inner = 0; inner < timesInner; ++inner )
        sum += m1[row * N1 + inner] * m2[inner * N2 + col];
      *(res++) = sum;
    }
  }
}

static void invnormalize_mat(double *T, double *iT) {
  double is = 1.0/T[0];
  double m0 = -T[2]*is;
  double m1 = -T[5]*is;
  iT[0] = is;
  iT[1] = 0;
  iT[2] = m0;
  iT[3] = 0;
  iT[4] = is;
  iT[5] = m1;
  iT[6] = 0;
  iT[7] = 0;
  iT[8] = 1;
}

static int svdcmp_(double **u, int m, int n, double w[], double **v) {
  const int max_its = 30;
  int flag, i, its, j, jj, k, l, nm;
  double anorm, c, f, g, h, s, scale, x, y, z;
  double *rv1 = (double *)malloc(sizeof(double) * (n + 1));
  g = scale = anorm = 0.0;
  for (i = 0; i < n; i++) {
    l = i + 1;
    rv1[i] = scale * g;
    g = s = scale = 0.0;
    if (i < m) {
      for (k = i; k < m; k++) scale += fabs(u[k][i]);
      if (scale) {
        for (k = i; k < m; k++) {
          u[k][i] /= scale;
          s += u[k][i] * u[k][i];
        }
        f = u[i][i];
        g = -SIGN(sqrt(s), f);
        h = f * g - s;
        u[i][i] = f - g;
        for (j = l; j < n; j++) {
          for (s = 0.0, k = i; k < m; k++) s += u[k][i] * u[k][j];
          f = s / h;
          for (k = i; k < m; k++) u[k][j] += f * u[k][i];
        }
        for (k = i; k < m; k++) u[k][i] *= scale;
      }
    }
    w[i] = scale * g;
    g = s = scale = 0.0;
    if (i < m && i != n - 1) {
      for (k = l; k < n; k++)
        scale += fabs(u[i][k]);
      if (scale) {
        for (k = l; k < n; k++) {
          u[i][k] /= scale;
          s += u[i][k] * u[i][k];
        }
        f = u[i][l];
        g = -SIGN(sqrt(s),f);
        h = f * g - s;
        u[i][l] = f - g;
        for (k = l; k < n; k++) rv1[k] = u[i][k] / h;
        for (j = l; j < m; j++) {
          for (s = 0.0, k = l; k < n; k++) s += u[j][k] * u[i][k];
          for (k = l; k < n; k++) u[j][k] += s * rv1[k];
        }
        for (k = l; k < n; k++) u[i][k] *= scale;
      }
    }
    anorm = fmax(anorm, (fabs(w[i]) + fabs(rv1[i])));
  }

  for (i = n - 1; i >= 0; i--) {
    if (i < n - 1) {
      if (g) {
        for (j = l; j < n; j++) v[j][i] = (u[i][j] / u[i][l]) / g;
        for (j = l; j < n; j++) {
          for (s = 0.0, k = l; k < n; k++) s += u[i][k] * v[k][j];
          for (k = l; k < n; k++) v[k][j] += s * v[k][i];
        }
      }
      for (j = l; j < n; j++) v[i][j] = v[j][i] = 0.0;
    }
    v[i][i] = 1.0;
    g = rv1[i];
    l = i;
  }

  for (i = IMIN(m, n) - 1; i >= 0; i--) {
    l = i + 1;
    g = w[i];
    for (j = l; j < n; j++) u[i][j] = 0.0;
    if (g) {
      g = 1.0 / g;
      for (j = l; j < n; j++) {
        for (s = 0.0, k = l; k < m; k++) s += u[k][i] * u[k][j];
        f = (s / u[i][i]) * g;
        for (k = i; k < m; k++) u[k][j] += f * u[k][i];
      }
      for (j = i; j < m; j++) u[j][i] *= g;
    } else {
      for (j = i; j < m; j++) u[j][i] = 0.0;
    }
    ++u[i][i];
  }
  for (k = n - 1; k >= 0; k--) {
    for (its = 0; its < max_its; its++) {
      flag = 1;
      for (l = k; l >= 0; l--) {
        nm = l - 1;
        if ((double)(fabs(rv1[l]) + anorm) == anorm || nm < 0) {
          flag = 0;
          break;
        }
        if ((double)(fabs(w[nm]) + anorm) == anorm) break;
      }
      if (flag) {
        c = 0.0;
        s = 1.0;
        for (i = l; i <= k; i++) {
          f = s * rv1[i];
          rv1[i] = c * rv1[i];
          if ((double)(fabs(f) + anorm) == anorm) break;
          g = w[i];
          h = PYTHAG(f, g);
          w[i] = h;
          h = 1.0 / h;
          c = g * h;
          s = -f * h;
          for (j = 0; j < m; j++) {
            y = u[j][nm];
            z = u[j][i];
            u[j][nm] = y * c + z * s;
            u[j][i] = z * c - y * s;
          }
        }
      }
      z = w[k];
      if (l == k) {
        if (z < 0.0) {
          w[k] = -z;
          for (j = 0; j < n; j++) v[j][k] = -v[j][k];
        }
        break;
      }
      if (its == max_its - 1) {
        return 1;
      }
      assert(k > 0);
      x = w[l];
      nm = k - 1;
      y = w[nm];
      g = rv1[nm];
      h = rv1[k];
      f = ((y - z) * (y + z) + (g - h) * (g + h)) / (2.0 * h * y);
      g = PYTHAG(f, 1.0);
      f = ((x - z) * (x + z) + h * ((y / (f + SIGN(g, f))) - h)) / x;
      c = s = 1.0;
      for (j = l; j <= nm; j++) {
        i = j + 1;
        g = rv1[i];
        y = w[i];
        h = s * g;
        g = c * g;
        z = PYTHAG(f, h);
        rv1[j] = z;
        c = f / z;
        s = h / z;
        f = x * c + g * s;
        g = g * c - x * s;
        h = y * s;
        y *= c;
        for (jj = 0; jj < n; jj++) {
          x = v[jj][j];
          z = v[jj][i];
          v[jj][j] = x * c + z * s;
          v[jj][i] = z * c - x * s;
        }
        z = PYTHAG(f, h);
        w[j] = z;
        if (z) {
          z = 1.0 / z;
          c = f * z;
          s = h * z;
        }
        f = c * g + s * y;
        x = c * y - s * g;
        for (jj = 0; jj < m; jj++) {
          y = u[jj][j];
          z = u[jj][i];
          u[jj][j] = y * c + z * s;
          u[jj][i] = z * c - y * s;
        }
      }
      rv1[l] = 0.0;
      rv1[k] = f;
      w[k] = x;
    }
  }
  free(rv1);
  return 0;
}

static int SVD(double *U, double *W, double *V, double *matx, int M, int N) {
  // Assumes allocation for U is MxN
  double **nrU, **nrV;
  int problem, i;

  nrU = (double **)malloc((M)*sizeof(double*));
  nrV = (double **)malloc((N)*sizeof(double*));
  problem = !(nrU && nrV);
  if (!problem) {
    problem = 0;
    for (i = 0; i < M; i++) {
      nrU[i] = &U[i * N];
    }
    for (i = 0; i < N; i++) {
      nrV[i] = &V[i * N];
    }
  }
  if (problem) {
    return 1;
  }

  /* copy from given matx into nrU */
  for (i = 0; i < M; i++) {
    memcpy(&(nrU[i][0]), matx + N * i, N * sizeof(*matx));
  }

  /* HERE IT IS: do SVD */
  if (svdcmp_(nrU, M, N, W, nrV)) {
    return 1;
  }

  /* free Numerical Recipes arrays */
  free(nrU);
  free(nrV);

  return 0;
}

static void normalizeHomography(double *pts, int n, double *T) {
  // Assume the points are 2d coordinates with scale = 1
  double *p = pts;
  double mean[2] = {0, 0};
  double msqe = 0;
  double scale;
  int i;
  for (i = 0; i < n; ++i, p+=2) {
    mean[0] += p[0];
    mean[1] += p[1];
  }
  mean[0] /= n;
  mean[1] /= n;
  for (p = pts, i = 0; i < n; ++i, p+=2) {
    p[0] -= mean[0];
    p[1] -= mean[1];
    msqe += sqrt(p[0] * p[0] + p[1] * p[1]);
  }
  msqe /= n;
  scale = sqrt(2)/msqe;
  T[0] = scale;
  T[1] = 0;
  T[2] = -scale * mean[0];
  T[3] = 0;
  T[4] = scale;
  T[5] = -scale * mean[1];
  T[6] = 0;
  T[7] = 0;
  T[8] = 1;
  for (p = pts, i = 0; i < n; ++i, p+=2) {
    p[0] *= scale;
    p[1] *= scale;
  }
}

static void denormalizeHomography(double *H, double *T1, double *T2) {
  double iT2[9];
  double H2[9];
  invnormalize_mat(T2, iT2);
  MultiplyMat(H, T1, H2, 3, 3, 3);
  MultiplyMat(iT2, H2, H, 3, 3, 3);
}

static void denormalizeAffine(double *H, double *T1, double *T2) {
  double Ha[MAX_PARAMDIM];
  Ha[0] = H[0];
  Ha[1] = H[1];
  Ha[2] = H[4];
  Ha[3] = H[2];
  Ha[4] = H[3];
  Ha[5] = H[5];
  Ha[6] = Ha[7] = 0;
  Ha[8] = 1;
  denormalizeHomography(Ha, T1, T2);
  H[0] = Ha[0];
  H[1] = Ha[1];
  H[2] = Ha[3];
  H[3] = Ha[4];
  H[4] = Ha[2];
  H[5] = Ha[5];
}

int findHomography(const int np, double *pts1, double *pts2, double *mat) {
  // Implemented from Peter Kovesi's normalized implementation
  const int np3 = np * 3;
  double *a = (double *)malloc(sizeof(double) * np3 * 18);
  double *U = a + np3 * 9;
  double S[9], V[9 * 9];
  int i, mini;
  double sx, sy, dx, dy;
  double T1[9], T2[9];

  normalizeHomography(pts1, np, T1);
  normalizeHomography(pts2, np, T2);

  for (i = 0; i < np; ++i) {
    dx = *(pts2++);
    dy = *(pts2++);
    sx = *(pts1++);
    sy = *(pts1++);

    a[i * 3 * 9 + 0] = a[i * 3 * 9 + 1] = a[i * 3 * 9 + 2] = 0;
    a[i * 3 * 9 + 3] = -sx;
    a[i * 3 * 9 + 4] = -sy;
    a[i * 3 * 9 + 5] = -1;
    a[i * 3 * 9 + 6] = dy * sx;
    a[i * 3 * 9 + 7] = dy * sy;
    a[i * 3 * 9 + 8] = dy;

    a[(i * 3 + 1) * 9 + 0] = sx;
    a[(i * 3 + 1) * 9 + 1] = sy;
    a[(i * 3 + 1) * 9 + 2] = 1;
    a[(i * 3 + 1) * 9 + 3] = a[(i * 3 + 1) * 9 + 4] =
        a[(i * 3 + 1) * 9 + 5] = 0;
    a[(i * 3 + 1) * 9 + 6] = -dx * sx;
    a[(i * 3 + 1) * 9 + 7] = -dx * sy;
    a[(i * 3 + 1) * 9 + 8] = -dx;

    a[(i * 3 + 2) * 9 + 0] = -dy * sx;
    a[(i * 3 + 2) * 9 + 1] = -dy * sy;
    a[(i * 3 + 2) * 9 + 2] = -dy;
    a[(i * 3 + 2) * 9 + 3] = dx * sx;
    a[(i * 3 + 2) * 9 + 4] = dx * sy;
    a[(i * 3 + 2) * 9 + 5] = dx;
    a[(i * 3 + 2) * 9 + 6] = a[(i * 3 + 2) * 9 + 7] =
        a[(i * 3 + 2) * 9 + 8] = 0;
  }

  if (SVD(U, S, V, a, np3, 9)) {
    free(a);
    return 1;
  } else {
    double minS = 1e12;
    mini = -1;
    for (i = 0; i < 9; ++i) {
      if (S[i] < minS) {
        minS = S[i];
        mini = i;
      }
    }
  }

  for (i = 0; i < 9; i++)
    mat[i] = V[i * 9 + mini];
  denormalizeHomography(mat, T1, T2);
  free(a);
  if (mat[8] == 0.0) {
    return 1;
  } else {
    double scale = 1.0/mat[8];
    for (i = 0; i < 9; i++)
      mat[i] *= scale;
  }
  return 0;
}

int PseudoInverse(double *inv, double *matx, const int M, const int N) {
  double *U, *W, *V, ans;
  int i, j, k;
  U = (double *)malloc(M * N * sizeof(*matx));
  W = (double *)malloc(N * sizeof(*matx));
  V = (double *)malloc(N * N * sizeof(*matx));

  if (!(U && W && V)) {
    return 1;
  }
  if (SVD(U, W, V, matx, M, N)) {
    return 1;
  }
  for (i = 0; i < N; i++) {
    if (fabs(W[i]) < TINY_NEAR_ZERO) {
      return 1;
    }
  }

  for (i = 0; i < N; i++) {
    for (j = 0; j < M; j++) {
      ans = 0;
      for (k = 0; k < N; k++) {
        ans += V[k + N * i] * U[k + N * j] / W[k];
      }
      inv[j + M * i] = ans;
    }
  }
  free(U);
  free(W);
  free(V);
  return 0;
}

int findAffine(const int np, double *pts1, double *pts2, double *mat) {
  const int np2 = np * 2;
  double *a = (double *)malloc(sizeof(double) * np2 * 13);
  double *b = a + np2 * 6;
  double *temp = b + np2;
  int i;
  double sx, sy, dx, dy;

  double T1[9], T2[9];
  normalizeHomography(pts1, np, T1);
  normalizeHomography(pts2, np, T2);

  for (i = 0; i < np; ++i) {
    dx = *(pts2++);
    dy = *(pts2++);
    sx = *(pts1++);
    sy = *(pts1++);

    a[i * 2 * 6 + 0] = sx;
    a[i * 2 * 6 + 1] = sy;
    a[i * 2 * 6 + 2] = 0;
    a[i * 2 * 6 + 3] = 0;
    a[i * 2 * 6 + 4] = 1;
    a[i * 2 * 6 + 5] = 0;
    a[(i * 2 + 1) * 6 + 0] = 0;
    a[(i * 2 + 1) * 6 + 1] = 0;
    a[(i * 2 + 1) * 6 + 2] = sx;
    a[(i * 2 + 1) * 6 + 3] = sy;
    a[(i * 2 + 1) * 6 + 4] = 0;
    a[(i * 2 + 1) * 6 + 5] = 1;

    b[2 * i] = dx;
    b[2 * i + 1] = dy;
  }
  if (PseudoInverse(temp, a, np2, 6)){
    free(a);
    return 1;
  }
  MultiplyMat(temp, b, mat, 6, np2, 1);
  denormalizeAffine(mat, T1, T2);
  free(a);
  return 0;
}

void projectPointsHomography(double *mat, double *points,
                             double *proj, const int n,
                             const int stride_points, const int stride_proj) {
  int i;
  double x, y, Z;
  for (i = 0; i < n; ++i) {
    x = *(points++), y = *(points++);
    Z = 1. / (mat[6] * x + mat[7] * y + mat[8]);
    *(proj++) = (mat[0] * x + mat[1] * y + mat[2]) * Z;
    *(proj++) = (mat[3] * x + mat[4] * y + mat[5]) * Z;
    points += stride_points - 2;
    proj += stride_proj - 2;
  }
}

void projectPointsAffine(double *mat, double *points,
                         double *proj, const int n,
                         const int stride_points, const int stride_proj) {
  int i;
  for (i = 0; i < n; ++i) {
    const double x = *(points++), y = *(points++);
    *(proj++) = mat[0] * x + mat[1] * y + mat[4];
    *(proj++) = mat[2] * x + mat[3] * y + mat[5];
    points += stride_points - 2;
    proj += stride_proj - 2;
  }
}

#define clip_pixel(v) ((v) < 0 ? 0 : ((v) > 255 ? 255 : (v)))

double getCubicValue(double p[4], double x) {
  return p[1] + 0.5 * x * (p[2] - p[0]
          + x * (2.0 * p[0] - 5.0 * p[1] + 4.0 * p[2] - p[3]
          + x * (3.0 * (p[1] - p[2]) + p[3] - p[0])));
}

void get_subcolumn(unsigned char *ref, double col[4],
                   int stride, int x, int y_start) {
  int i;
  for (i = 0; i < 4; ++i) {
    col[i] = ref[(i + y_start) * stride + x];
  }
}

double bicubic(unsigned char *ref, double x, double y, int stride) {
  double arr[4];
  int k;
  int i = (int) x;
  int j = (int) y;
  for (k = 0; k < 4; ++k) {
    double arr_temp[4];
    get_subcolumn(ref, arr_temp, stride, i + k - 1, j - 1);
    arr[k] = getCubicValue(arr_temp, y - j);
  }
  return getCubicValue(arr, x - i);
}

unsigned char interpolate(unsigned char *ref, double x, double y,
                          int width, int height, int stride) {
  if (x < 0 && y < 0) return ref[0];
  else if (x < 0 && y > height - 1)
    return ref[(height - 1) * stride];
  else if (x > width - 1 && y < 0)
    return ref[width - 1];
  else if (x > width - 1 && y > height - 1)
    return ref[(height - 1) * stride + (width - 1)];
  else if (x < 0) {
    int v;
    int i = (int) y;
    double a = y - i;
    if (y > 1 && y < height - 2) {
      double arr[4];
      get_subcolumn(ref, arr, stride, 0, i - 1);
      return clip_pixel(getCubicValue(arr, a));
    }
    v = (int)(ref[i * stride] * (1 - a) + ref[(i + 1) * stride] * a + 0.5);
    return clip_pixel(v);
  } else if (y < 0) {
    int v;
    int j = (int) x;
    double b = x - j;
    if (x > 1 && x < width - 2) {
      double arr[4] = {ref[j - 1], ref[j], ref[j + 1], ref[j + 2]};
      return clip_pixel(getCubicValue(arr, b));
    }
    v = (int)(ref[j] * (1 - b) + ref[j + 1] * b + 0.5);
    return clip_pixel(v);
  } else if (x > width - 1) {
    int v;
    int i = (int) y;
    double a = y - i;
    if (y > 1 && y < height - 2) {
      double arr[4];
      get_subcolumn(ref, arr, stride, width - 1, i - 1);
      return clip_pixel(getCubicValue(arr, a));
    }
    v = (int)(ref[i * stride + width - 1] * (1 - a) +
                  ref[(i + 1) * stride + width - 1] * a + 0.5);
    return clip_pixel(v);
  } else if (y > height - 1) {
    int v;
    int j = (int) x;
    double b = x - j;
    if (x > 1 && x < width - 2) {
      int row = (height - 1) * stride;
      double arr[4] = {ref[row + j - 1], ref[row + j],
                      ref[row + j + 1], ref[row + j + 2]};
      return clip_pixel(getCubicValue(arr, b));
    }
    v = (int)(ref[(height - 1) * stride + j] * (1 - b) +
                  ref[(height - 1) * stride + j + 1] * b + 0.5);
    return clip_pixel(v);
  } else if (x > 1 && y > 1 && x < width -2 && y < height -2) {
    return clip_pixel(bicubic(ref, x, y, stride));
  } else {
    int i = (int) y;
    int j = (int) x;
    double a = y - i;
    double b = x - j;
    int v = (int)(ref[i * stride + j] * (1 - a) * (1 - b) +
                  ref[i * stride + j + 1] * (1 - a) * b +
                  ref[(i + 1) * stride + j] * a * (1 - b) +
                  ref[(i + 1) * stride + j + 1] * a * b);
    return clip_pixel(v);
  }
}

void warpImage(double *H,
               unsigned char *ref,
               int width, int height, int stride,
               unsigned char *pred,
               int p_col, int p_row,
               int p_width, int p_height, int p_stride,
               int subsampling_col, int subsampling_row,
               int x_scale, int y_scale) {
  int i, j;

  for (i = p_row; i < p_row + p_height; ++i) {
    for (j = p_col; j < p_col + p_width; ++j) {
      double in[2], out[2];
      in[0] = subsampling_col ? 2 * j + 0.5 : j;
      in[1] = subsampling_row ? 2 * i + 0.5 : i;
      projectPointsHomography(H, in, out, 1, 2, 2);
      out[0] = subsampling_col ? (out[0] - 0.5) / 2.0 : out[0];
      out[1] = subsampling_row ? (out[1] - 0.5) / 2.0 : out[1];
      out[0] *= x_scale / 16.0;
      out[1] *= y_scale / 16.0;
      pred[(j - p_col) + (i - p_row) * p_stride] =
        interpolate(ref, out[0], out[1], width, height, stride);
    }
  }
}

void warpImage2(int mi_row, int mi_col,
                double *H,
                unsigned char *ref,
                int width, int height, int stride,
                unsigned char *pred,
                int p_col, int p_row,
                int p_width, int p_height, int p_stride,
                int subsampling_col, int subsampling_row,
                int x_scale, int y_scale) {
  int i, j;

  for (i = p_row; i < p_row + p_height; ++i) {
    for (j = p_col; j < p_col + p_width; ++j) {
      double in[2], out[2];
      in[0] = subsampling_col ? 2 * j + 0.5 : j;
      in[1] = subsampling_row ? 2 * i + 0.5 : i;
      projectPointsAffine(H, in, out, 1, 2, 2);
      out[0] = subsampling_col ? (out[0] - 0.5) / 2.0 : out[0];
      out[1] = subsampling_row ? (out[1] - 0.5) / 2.0 : out[1];
      out[0] *= x_scale / 16.0;
      out[1] *= y_scale / 16.0;
      pred[(j - p_col) + (i - p_row) * p_stride] =
        interpolate(ref,
                    out[0] + mi_col * 8, out[1] + mi_row * 8,
                    width, height, stride);
    }
  }
}
