class Scene {
  Camera camera;
  PointCloud point_cloud;
  MotionField motion_field;
  ArrayList<PVector> last_positions;
  ArrayList<PVector> current_positions;
  int[] render_list;
  boolean[] real;

  Scene(Camera camera, PointCloud point_cloud, MotionField motion_field) {
    this.camera = camera;
    this.point_cloud = point_cloud;
    this.motion_field = motion_field;
    last_positions = project2Camera();
    current_positions = project2Camera();
    render_list = new int[width * height];
    real = new boolean[width * height];
    updateRenderList();
  }

  ArrayList<PVector> project2Camera() {
    ArrayList<PVector> projs = new ArrayList<PVector>();
    for (int i = 0; i < point_cloud.size(); i++) {
      PVector proj = camera.project(point_cloud.getPosition(i));
      projs.add(proj);
    }
    return projs;
  }
  // check if (i,j) is a valid position and if it has pixel value
  boolean check(int i, int j, ArrayList<PVector> queue, boolean[] visited) {
    if (i >= 0 && i < height && j >= 0 && j < width &&
        !visited[i * width + j]) {
      visited[i * width + j] = true;
      if (real[i * width + j] && render_list[i * width + j] != -1) {
        return true;
      }
      queue.add(new PVector(i, j));
    }
    return false;
  }
  // interpolate a position by using BFS
  void interpolate(int row, int col) {
    if (render_list[row * width + col] != -1) return;
    boolean[] visited = new boolean[width * height];
    for (int i = 0; i < width * height; i++) visited[i] = false;
    visited[row * width + col] = true;
    ArrayList<PVector> queue = new ArrayList<PVector>();
    queue.add(new PVector(row, col));
    while (queue.size() > 0) {
      PVector pix = queue.get(0);
      queue.remove(0);
      int i = int(pix.x);
      int j = int(pix.y);
      // up
      if (check(i - 1, j, queue, visited)) {
        render_list[row * width + col] = render_list[(i - 1) * width + j];
        return;
      }
      // down
      if (check(i + 1, j, queue, visited)) {
        render_list[row * width + col] = render_list[(i + 1) * width + j];
        return;
      }
      // left
      if (check(i, j - 1, queue, visited)) {
        render_list[row * width + col] = render_list[i * width + j - 1];
        return;
      }
      // right
      if (check(i, j + 1, queue, visited)) {
        render_list[row * width + col] = render_list[i * width + j + 1];
        return;
      }
    }
  }
  // update render list by using depth test
  void updateRenderList() {
    // clear render list
    for (int i = 0; i < width * height; i++) render_list[i] = -1;
    for (int i = 0; i < width * height; i++) real[i] = false;
    // depth test and get render list
    float[] depth = new float[width * height];
    for (int i = 0; i < width * height; i++) depth[i] = Float.POSITIVE_INFINITY;
    for (int i = 0; i < current_positions.size(); i++) {
      PVector pos = current_positions.get(i);
      int row = int(pos.y + height / 2);
      int col = int(pos.x + width / 2);
      int idx = row * width + col;
      if (row >= 0 && row < height && col >= 0 && col < width) {
        if (render_list[idx] == -1 || pos.z < depth[idx]) {
          depth[idx] = pos.z;
          render_list[idx] = i;
          real[idx] = true;
        }
      }
    }
  }

  void run(boolean inter) {
    camera.run();
    last_positions = current_positions;
    current_positions = project2Camera();
    updateRenderList();
    // interpolation
    if (inter) {
      for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++) {
          interpolate(i, j);
        }
    }
    motion_field.update(last_positions, current_positions, render_list);
  }

  void render(boolean show_motion_field) {
    for (int i = 0; i < height; i++)
      for (int j = 0; j < width; j++) {
        if (render_list[i * width + j] == -1) continue;
        stroke(point_cloud.getColor(render_list[i * width + j]));
        point(j, i);
      }
    if (show_motion_field) {
      motion_field.show();
    }
  }
}
