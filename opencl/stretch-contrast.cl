/* This file is an image processing operation for GEGL
 *
 * GEGL is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * GEGL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GEGL; if not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright 2013 Carlos Zubieta <czubieta.dev@gmail.com>
 */


__kernel void init_stretch (__global float *out_min,
                            __global float *out_max)
{
  int gid = get_global_id (0);

  out_min[gid] =  FLT_MAX;
  out_max[gid] = -FLT_MAX;
}

__kernel void two_stages_local_min_max_reduce (__global const float4 *in,
                                               __global       float  *out_min,
                                               __global       float  *out_max,
                                               __local        float  *aux_min,
                                               __local        float  *aux_max,
                                                              int    n_pixels)
{
  int    gid   = get_global_id(0);
  int    gsize = get_global_size(0);
  int    lid   = get_local_id(0);
  int    lsize = get_local_size(0);
  float4 min_v = (float4)( FLT_MAX);
  float4 max_v = (float4)(-FLT_MAX);
  float4 in_v;
  float  aux0, aux1;
  int    it;

  /* Loop sequentially over chunks of input vector */
  for (it = gid; it < n_pixels; it += gsize)
    {
      in_v  =  in[it];
      min_v =  fmin (min_v, in_v);
      max_v =  fmax (max_v, in_v);
    }

  /* Perform parallel reduction */
  aux_min[lid] = min (min (min_v.x, min_v.y), min_v.z);
  aux_max[lid] = max (max (max_v.x, max_v.y), max_v.z);

  barrier (CLK_LOCAL_MEM_FENCE);

  for(it = lsize / 2; it > 0; it >>= 1)
    {
      if (lid < it)
        {
          aux0         = aux_min[lid + it];
          aux1         = aux_min[lid];
          aux_min[lid] = fmin (aux0, aux1);

          aux0         = aux_max[lid + it];
          aux1         = aux_max[lid];
          aux_max[lid] = fmax (aux0, aux1);
        }
      barrier (CLK_LOCAL_MEM_FENCE);
  }
  if (lid == 0)
    {
      out_min[get_group_id(0)] = aux_min[0];
      out_max[get_group_id(0)] = aux_max[0];
    }

  /* the work-group size is the size of the buffer.
   * Make sure it's fully initialized */
  if (gid == 0)
    {
      /* No special case handling, gsize is a multiple of lsize */
      int nb_wg = gsize / lsize;
      for (it = nb_wg; it < lsize; it++)
        {
          out_min[it] =  FLT_MAX;
          out_max[it] = -FLT_MAX;
        }
    }
}

__kernel void global_min_max_reduce (__global float *in_min,
                                     __global float *in_max,
                                     __global float *out_min_max)
{
  int   gid   = get_global_id(0);
  int   lid   = get_local_id(0);
  int   lsize = get_local_size(0);
  float aux0, aux1;
  int   it;

  /* Perform parallel reduction */
  for (it = lsize / 2; it > 0; it >>= 1)
    {
      if (lid < it)
        {
          aux0        = in_min[lid + it];
          aux1        = in_min[lid];
          in_min[gid] = fmin (aux0, aux1);

          aux0        = in_max[lid + it];
          aux1        = in_max[lid];
          in_max[gid] = fmax (aux0, aux1);
        }
      barrier (CLK_GLOBAL_MEM_FENCE);
  }
  if (lid == 0)
    {
      out_min_max[0] = in_min[gid];
      out_min_max[1] = in_max[gid];
    }
}

__kernel void cl_stretch_contrast (__global const float4 *in,
                                   __global       float4 *out,
                                                  float   min,
                                                  float   diff)
{
  int    gid  = get_global_id(0);
  float4 in_v = in[gid];

  in_v.xyz = (in_v.xyz - min) / diff;
  out[gid] = in_v;
}
