/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "vp9/common/vp9_blockd.h"

MB_PREDICTION_MODE vp9_left_block_mode(const MODE_INFO *cur_mi,
                                       const MODE_INFO *left_mi, int b) {
  if (b == 0 || b == 2) {
    if (!left_mi || is_inter_block(&left_mi->mbmi))
      return DC_PRED;

    return left_mi->mbmi.sb_type < BLOCK_8X8 ? left_mi->bmi[b + 1].as_mode
                                             : left_mi->mbmi.mode;
  } else {
    assert(b == 1 || b == 3);
    return cur_mi->bmi[b - 1].as_mode;
  }
}

MB_PREDICTION_MODE vp9_above_block_mode(const MODE_INFO *cur_mi,
                                        const MODE_INFO *above_mi, int b) {
  if (b == 0 || b == 1) {
    if (!above_mi || is_inter_block(&above_mi->mbmi))
      return DC_PRED;

    return above_mi->mbmi.sb_type < BLOCK_8X8 ? above_mi->bmi[b + 2].as_mode
                                              : above_mi->mbmi.mode;
  } else {
    assert(b == 2 || b == 3);
    return cur_mi->bmi[b - 2].as_mode;
  }
}

void vp9_foreach_transformed_block_in_plane(
    const MACROBLOCKD *const xd, BLOCK_SIZE bsize, int plane,
    foreach_transformed_block_visitor visit, void *arg) {
  const struct macroblockd_plane *const pd = &xd->plane[plane];
  const MB_MODE_INFO* mbmi = &xd->mi_8x8[0]->mbmi;
  // block and transform sizes, in number of 4x4 blocks log 2 ("*_b")
  // 4x4=0, 8x8=2, 16x16=4, 32x32=6, 64x64=8
  // transform size varies per plane, look it up in a common way.
  const TX_SIZE tx_size = plane ? get_uv_tx_size(mbmi)
                                : mbmi->tx_size;
  const BLOCK_SIZE plane_bsize = get_plane_block_size(bsize, pd);
  const int num_4x4_w = num_4x4_blocks_wide_lookup[plane_bsize];
  const int num_4x4_h = num_4x4_blocks_high_lookup[plane_bsize];
  const int step = 1 << (tx_size << 1);
  int i;

  // If mb_to_right_edge is < 0 we are in a situation in which
  // the current block size extends into the UMV and we won't
  // visit the sub blocks that are wholly within the UMV.
  if (xd->mb_to_right_edge < 0 || xd->mb_to_bottom_edge < 0) {
    int r, c;

    int max_blocks_wide = num_4x4_w;
    int max_blocks_high = num_4x4_h;

    // xd->mb_to_right_edge is in units of pixels * 8.  This converts
    // it to 4x4 block sizes.
    if (xd->mb_to_right_edge < 0)
      max_blocks_wide += (xd->mb_to_right_edge >> (5 + pd->subsampling_x));

    if (xd->mb_to_bottom_edge < 0)
      max_blocks_high += (xd->mb_to_bottom_edge >> (5 + pd->subsampling_y));

    i = 0;
    // Unlike the normal case - in here we have to keep track of the
    // row and column of the blocks we use so that we know if we are in
    // the unrestricted motion border.
    for (r = 0; r < num_4x4_h; r += (1 << tx_size)) {
      for (c = 0; c < num_4x4_w; c += (1 << tx_size)) {
        if (r < max_blocks_high && c < max_blocks_wide)
          visit(plane, i, plane_bsize, tx_size, arg);
        i += step;
      }
    }
  } else {
    for (i = 0; i < num_4x4_w * num_4x4_h; i += step)
      visit(plane, i, plane_bsize, tx_size, arg);
  }
}

void vp9_foreach_transformed_block(const MACROBLOCKD* const xd,
                                   BLOCK_SIZE bsize,
                                   foreach_transformed_block_visitor visit,
                                   void *arg) {
  int plane;

  for (plane = 0; plane < MAX_MB_PLANE; plane++)
    vp9_foreach_transformed_block_in_plane(xd, bsize, plane, visit, arg);
}

void vp9_foreach_transformed_block_uv(const MACROBLOCKD* const xd,
                                      BLOCK_SIZE bsize,
                                      foreach_transformed_block_visitor visit,
                                      void *arg) {
  int plane;

  for (plane = 1; plane < MAX_MB_PLANE; plane++)
    vp9_foreach_transformed_block_in_plane(xd, bsize, plane, visit, arg);
}

void vp9_set_contexts(const MACROBLOCKD *xd, struct macroblockd_plane *pd,
                      BLOCK_SIZE plane_bsize, TX_SIZE tx_size, int has_eob,
                      int aoff, int loff) {
  ENTROPY_CONTEXT *const a = pd->above_context + aoff;
  ENTROPY_CONTEXT *const l = pd->left_context + loff;
  const int tx_size_in_blocks = 1 << tx_size;

  // above
  if (has_eob && xd->mb_to_right_edge < 0) {
    int i;
    const int blocks_wide = num_4x4_blocks_wide_lookup[plane_bsize] +
                            (xd->mb_to_right_edge >> (5 + pd->subsampling_x));
    int above_contexts = tx_size_in_blocks;
    if (above_contexts + aoff > blocks_wide)
      above_contexts = blocks_wide - aoff;

    for (i = 0; i < above_contexts; ++i)
      a[i] = has_eob;
    for (i = above_contexts; i < tx_size_in_blocks; ++i)
      a[i] = 0;
  } else {
    vpx_memset(a, has_eob, sizeof(ENTROPY_CONTEXT) * tx_size_in_blocks);
  }

  // left
  if (has_eob && xd->mb_to_bottom_edge < 0) {
    int i;
    const int blocks_high = num_4x4_blocks_high_lookup[plane_bsize] +
                            (xd->mb_to_bottom_edge >> (5 + pd->subsampling_y));
    int left_contexts = tx_size_in_blocks;
    if (left_contexts + loff > blocks_high)
      left_contexts = blocks_high - loff;

    for (i = 0; i < left_contexts; ++i)
      l[i] = has_eob;
    for (i = left_contexts; i < tx_size_in_blocks; ++i)
      l[i] = 0;
  } else {
    vpx_memset(l, has_eob, sizeof(ENTROPY_CONTEXT) * tx_size_in_blocks);
  }
}

void vp9_setup_block_planes(MACROBLOCKD *xd, int ss_x, int ss_y) {
  int i;

  for (i = 0; i < MAX_MB_PLANE; i++) {
    xd->plane[i].plane_type = i ? PLANE_TYPE_UV : PLANE_TYPE_Y;
    xd->plane[i].subsampling_x = i ? ss_x : 0;
    xd->plane[i].subsampling_y = i ? ss_y : 0;
  }
#if CONFIG_ALPHA
  // TODO(jkoleszar): Using the Y w/h for now
  xd->plane[3].plane_type = PLANE_TYPE_Y;
  xd->plane[3].subsampling_x = 0;
  xd->plane[3].subsampling_y = 0;
#endif
}

#if CONFIG_GBT

void jacobi(double **a, int n, double d[], double **v, int *nrot)
// input a, output v
{
  int j,iq,ip,i;
  double tresh,theta,tau,t,sm,s,h,g,c,*b,*z;

  b=(double*)malloc(sizeof(double)*(n+1));
  z=(double*)malloc(sizeof(double)*(n+1));
  for (ip=1;ip<=n;ip++) {
    for (iq=1;iq<=n;iq++) v[ip][iq]=0.0;
    v[ip][ip]=1.0;
  }
  for (ip=1;ip<=n;ip++) {
    b[ip]=d[ip]=a[ip][ip];
    z[ip]=0.0;
  }
  *nrot=0;
  for (i=1;i<=50;i++) {
    sm=0.0;
    for (ip=1;ip<=n-1;ip++) {
      for (iq=ip+1;iq<=n;iq++)
        sm += fabs(a[ip][iq]);
    }
    if (sm == 0.0) {
      free(z);
      free(b);
      return;
    }
    if (i < 4)
      tresh=0.2*sm/(n*n);
    else
      tresh=0.0;
    for (ip=1;ip<=n-1;ip++) {
      for (iq=ip+1;iq<=n;iq++) {
        g=100.0*fabs(a[ip][iq]);
        if (i > 4 && (double)(fabs(d[ip])+g) == (double)fabs(d[ip])
          && (double)(fabs(d[iq])+g) == (double)fabs(d[iq]))
          a[ip][iq]=0.0;
        else if (fabs(a[ip][iq]) > tresh) {
          h=d[iq]-d[ip];
          if ((double)(fabs(h)+g) == (double)fabs(h))
            t=(a[ip][iq])/h;
          else {
            theta=0.5*h/(a[ip][iq]);
            t=1.0/(fabs(theta)+sqrt(1.0+theta*theta));
            if (theta < 0.0) t = -t;
          }
          c=1.0/sqrt(1+t*t);
          s=t*c;
          tau=s/(1.0+c);
          h=t*a[ip][iq];
          z[ip] -= h;
          z[iq] += h;
          d[ip] -= h;
          d[iq] += h;
          a[ip][iq]=0.0;
          for (j=1;j<=ip-1;j++) {
            ROTATE(a,j,ip,j,iq)
          }
          for (j=ip+1;j<=iq-1;j++) {
            ROTATE(a,ip,j,j,iq)
          }
          for (j=iq+1;j<=n;j++) {
            ROTATE(a,ip,j,iq,j)
          }
          for (j=1;j<=n;j++) {
            ROTATE(v,j,ip,j,iq)
          }
          ++(*nrot);
        }
      }
    }
    for (ip=1;ip<=n;ip++) {
      b[ip] += z[ip];
      d[ip]=b[ip];
      z[ip]=0.0;
    }
  }
  printf("Too many iterations in routine jacobi");
}

void eigsrt( double d[], double **v, int n )
{
  int k,j,i;
  double p;

  for (i=1;i<n;i++) {
    p=d[k=i];
    for (j=i+1;j<=n;j++)
      if (d[j] <= p) p=d[k=j];
    if (k != i) {
      d[k]=d[i];
      d[i]=p;
      for (j=1;j<=n;j++) {
        p=v[j][i];
        v[j][i]=v[j][k];
        v[j][k]=p;
      }
    }
  }
}

void eig(double *aa, double *vv, int n) // nxn matrix
{
  // malloc memory
  int i, j, nrot;
  double **a;
  double **v;
  double d[65*65];
  a = malloc( sizeof(double*) * (n+1) );
  v = malloc( sizeof(double*) * (n+1) );
  for (i = 0; i <= n; i++)
  {
    a[i] = malloc( sizeof(double) * (n+1) );
    v[i] = malloc( sizeof(double) * (n+1) );
  }

  // set indexing starting from 1
  for (i = 1; i <= n; i++)
    for (j = 1; j <=n; j++)
      a[i][j] = aa[(i-1) * n + j-1];

  // calculation using jacobi
  jacobi( a, n, d, v, &nrot );

//  for (i = 1; i <= n; i++)
//    printf("%f ", d[i]);
//  printf("\n");


  // sort the eval and evec
  eigsrt( d, v, n );

//  for (i = 1; i <= n; i++)
//    printf("%f ", d[i]);
//  printf("\n");

  // set indexing starting from 0
  for (i = 0; i < n; i++)
      for (j = 0; j < n; j++)
        vv[i * n + j] = v[i+1][j+1];

//  for (i = 0; i < n; i++)
//    printf("%f\n", vv[i*n+2]);


  // free memory
  for (i = 0; i <= n; i++)
  {
    free(a[i]);
    free(v[i]);
  }
  free(a);
  free(v);
}

#endif
