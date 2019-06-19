class PointCloud {
  ArrayList<PVector> points;  // array to save points
  IntList point_colors;       // array to save points color
  PVector cloud_mass;
  PVector[] depth;
  boolean[] real;
  PointCloud() {
    // initialize
    points = new ArrayList<PVector>();
    point_colors = new IntList();
    cloud_mass = new PVector(0, 0, 0);
    depth = new PVector[width * height];
    real = new boolean[width * height];
  }

  void generate(PImage rgb_img, PImage depth_img, Transform trans) {
    if (depth_img.width != width || depth_img.height != height ||
        rgb_img.width != width || rgb_img.height != height) {
      println("rgb and depth file dimension should be same with window size");
      exit();
    }
    // clear depth and real
    for (int i = 0; i < width * height; i++) {
      depth[i] = new PVector(0, Float.POSITIVE_INFINITY);
      real[i] = false;
    }
    for (int v = 0; v < height; v++)
      for (int u = 0; u < width; u++) {
        // get depth value (red channel)
        color depth_px = depth_img.get(u, v);
        depth[v * width + u].x = depth_px & 0x0000FFFF;
        if (int(depth[v * width + u].x) != 0) real[v * width + u] = true;
        point_colors.append(rgb_img.get(u, v));
      }
    for (int v = 0; v < height; v++)
      for (int u = 0; u < width; u++) {
        if (int(depth[v * width + u].x) == 0) {
          interpolate(v, u);
        }
        // add transformed pixel as well as pixel color to the list
        PVector pos = trans.transform(u, v, int(depth[v * width + u].x));
        points.add(pos);
        // accumulate z value
        cloud_mass = PVector.add(cloud_mass, pos);
      }
  }
  // check if a coordinate is a validate neighbor
  boolean check(int i, int j, ArrayList<PVector> queue, boolean[] visited,
                int back_idx) {
    if (i >= 0 && i < height && j >= 0 && j < width) {
      if (real[i * width + j]) return true;
      if (!visited[i * width + j]) {
        visited[i * width + j] = true;
        queue.add(new PVector(i, j, back_idx));
      }
    }
    return false;
  }
  // update the value on the path
  void updateBackChain(int idx, ArrayList<PVector> back) {
    int back_idx = back.size() - 1;
    color ref_color = point_colors.get(idx);
    int ref_depth = int(depth[idx].x);
    while (back_idx != -1) {
      PVector pos = back.get(back_idx);
      int i = int(pos.x);
      int j = int(pos.y);
      back_idx = int(pos.z);
      color dst_color = point_colors.get(i * width + j);
      float ori_diff = int(depth[i * width + j].y);
      float dst_diff = abs(dst_color - ref_color);
      // if(dst_diff<ori_diff)
      if (int(depth[i * width + j].x) == 0) {
        depth[i * width + j].x = ref_depth;
        depth[i * width + j].y = dst_diff;
      }
    }
  }
  // interpolate
  void interpolate(int row, int col) {
    if (row < 0 || row >= height || col < 0 || col >= width ||
        int(depth[row * width + col].x) != 0)
      return;
    ArrayList<PVector> queue = new ArrayList<PVector>();
    queue.add(new PVector(row, col, -1));
    ArrayList<PVector> back = new ArrayList<PVector>();
    boolean[] visited = new boolean[width * height];
    for (int i = 0; i < width * height; i++) visited[i] = false;
    visited[row * width + col] = true;
    while (queue.size() > 0) {
      // pop
      PVector pos = queue.get(0);
      queue.remove(0);
      int i = int(pos.x);
      int j = int(pos.y);
      int back_idx = int(pos.z);
      back.add(new PVector(i, j, back_idx));
      if (check(i - 1, j, queue, visited, back.size() - 1)) {
        updateBackChain((i - 1) * width + j, back);
      }
      if (check(i + 1, j, queue, visited, back.size() - 1)) {
        updateBackChain((i + 1) * width + j, back);
      }
      if (check(i, j - 1, queue, visited, back.size() - 1)) {
        updateBackChain(i * width + j - 1, back);
      }
      if (check(i, j + 1, queue, visited, back.size() - 1)) {
        updateBackChain(i * width + j + 1, back);
      }
    }
  }
  // get point cloud size
  int size() { return points.size(); }
  // get ith position
  PVector getPosition(int i) {
    if (i >= points.size()) {
      println("point position: index " + str(i) + " exceeds");
      exit();
    }
    return points.get(i);
  }
  // get ith color
  color getColor(int i) {
    if (i >= point_colors.size()) {
      println("point color: index " + str(i) + " exceeds");
      exit();
    }
    return point_colors.get(i);
  }
  // get cloud center
  PVector getCloudCenter() {
    if (points.size() > 0) return PVector.div(cloud_mass, points.size());
    return new PVector(0, 0, 0);
  }
  // merge two clouds
  void merge(PointCloud point_cloud) {
    for (int i = 0; i < point_cloud.size(); i++) {
      points.add(point_cloud.getPosition(i));
      point_colors.append(point_cloud.getColor(i));
    }
    cloud_mass = PVector.add(cloud_mass, point_cloud.cloud_mass);
  }
}
