import sys
import matplotlib.pyplot as plt
from matplotlib.collections import LineCollection
from matplotlib import colors as mcolors
from sklearn.cluster import KMeans
import numpy as np
import math

def read_frame_stats(fp, mi_rows, mi_cols, block_size):
  line = fp.readline()
  rows = int(math.ceil(mi_rows * 1. /block_size))
  cols = int(math.ceil(mi_cols * 1. /block_size))
  print "=", mi_rows, mi_cols, rows, cols
  log_stats = np.zeros((rows, cols))
  label_arr = np.zeros((rows, cols))
  stats = np.zeros((rows, cols))
  cnt = 0

  # read variance
  while line:
    word_ls = line.split()
    if word_ls[0] == "=":
      break
    row = int(word_ls[1]) / block_size
    col = int(word_ls[3]) / block_size
    v = int(word_ls[5])
    log_stats[row][col] = math.log(1+v)
    stats[row][col] = v
    cnt += 1
    line = fp.readline()

  # read center list
  line = fp.readline()
  line = fp.readline()
  word_ls = line.split()
  ctr_ls = np.array([float(item) for item in word_ls])

  # read group_idx
  line = fp.readline()
  word_ls = line.split()
  kmeans_data_size = int(word_ls[1])
  for i in range(kmeans_data_size):
    line = fp.readline()
    word_ls = line.split()
    row = int(word_ls[1]) / block_size
    col = int(word_ls[3]) / block_size
    label = int(word_ls[7])
    label_arr[row][col] = label

  img = yuv_to_rgb(read_frame(fp))
  print cnt, kmeans_data_size, rows * cols
  return stats, log_stats, ctr_ls, label_arr , img

def read_frame(fp, no_swap=0):
  plane = [None, None, None]
  for i in range(3):
    line = fp.readline()
    word_ls = line.split()
    word_ls = [int(item) for item in word_ls]
    rows = word_ls[0]
    cols = word_ls[1]

    line = fp.readline()
    word_ls = line.split()
    word_ls = [int(item) for item in word_ls]

    plane[i] = np.array(word_ls).reshape(rows, cols)
    if i > 0:
      plane[i] = plane[i].repeat(2, axis=0).repeat(2, axis=1)
  plane = np.array(plane)
  if no_swap == 0:
    plane = np.swapaxes(np.swapaxes(plane, 0, 1), 1, 2)
  return plane

def draw_blocks(axis, w, h, bs):
  colors = np.array([(0.7, 0., 0., 0.7)])
  segs = []
  for r in range(0, h, bs):
    x_ls = [0, w]
    y_ls = [r, r]
    segs.append(np.column_stack([x_ls, y_ls]))

  for c in range(0, w, bs):
    x_ls = [c, c]
    y_ls = [0, h]
    segs.append(np.column_stack([x_ls, y_ls]))

  line_setments = LineCollection(segs, linewidths=(.2,), colors=colors, linestyle='solid')
  axis.add_collection(line_setments)

def get_labels_from_kmeans(stats):
  stats_arr = stats.flatten()
  stats_arr = stats_arr.reshape((len(stats_arr), 1))
  kmeans = KMeans(n_clusters=3, random_state=0).fit(stats_arr)
  labels = kmeans.predict(stats_arr)
  labels = labels.reshape(stats.shape)
  return labels, kmeans.cluster_centers_

def yuv_to_rgb(yuv):
  #mat = np.array([
  #    [1.164,   0   , 1.596  ],
  #    [1.164, -0.391, -0.813],
  #    [1.164, 2.018 , 0     ] ]
  #               )
  #c = np.array([[ -16 , -16 , -16  ],
  #              [ 0   , -128, -128 ],
  #              [ -128, -128,   0  ]])

  mat = np.array([[1, 0, 1.4075], [1, -0.3445, -0.7169], [1, 1.7790, 0]])
  c = np.array([[0, 0, 0], [0, -128, -128], [-128, -128, 0]])
  mat_c = np.dot(mat, c)
  v = np.array([mat_c[0, 0], mat_c[1, 1], mat_c[2, 2]])
  mat = mat.transpose()
  rgb = np.dot(yuv, mat) + v
  rgb = rgb.astype(int)
  rgb = rgb.clip(0, 255)
  return rgb / 255.

def inv_log(v):
  return math.exp(v) - 1

if __name__ == '__main__':
  filename = sys.argv[1]
  fp = open(filename)
  line = fp.readline()
  while line:
    word_ls = line.split()
    mi_rows = int(word_ls[2])
    mi_cols = int(word_ls[4])
    block_size = int(word_ls[6])
    stats, log_stats, ctr_ls, label_arr, img = read_frame_stats(fp, mi_rows, mi_cols, block_size)
    fig, axes = plt.subplots(2, 2)
    axes[0][0].imshow(img)
    draw_blocks(axes[0][0], img.shape[1], img.shape[0], 64)
    axes[0][1].imshow(log_stats)

    log_stats_arr = log_stats.flatten()
    log_stats_max = log_stats.max()
    log_stats_min = log_stats.min()
    step = (log_stats_max - log_stats_min) / 20.
    log_stats_bins = np.arange(log_stats_min, log_stats_max, step)
    axes[1][0].hist(log_stats_arr, bins=log_stats_bins)

    print ctr_ls
    axes[1][1].imshow(label_arr)

    #labels, centers = get_labels_from_kmeans(log_stats)
    #print centers
    #axes[1][1].imshow(labels)

    plt.show()
    line = fp.readline()
