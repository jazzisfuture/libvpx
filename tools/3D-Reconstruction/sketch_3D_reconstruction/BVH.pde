/*
 *AABB bounding box
 *Bouding Volume Hierarchy
 */
class BoundingBox {
  float min_x, min_y, min_z, max_x, max_y, max_z;
  PVector center;
  BoundingBox() {
    min_x = Float.POSITIVE_INFINITY;
    min_y = Float.POSITIVE_INFINITY;
    min_z = Float.POSITIVE_INFINITY;
    max_x = Float.NEGATIVE_INFINITY;
    max_y = Float.NEGATIVE_INFINITY;
    max_z = Float.NEGATIVE_INFINITY;
    center = new PVector();
  }
  // build a bounding box for a triangle
  void create(Triangle t) {
    min_x = min(t.p1.x, min(t.p2.x, t.p3.x));
    max_x = max(t.p1.x, max(t.p2.x, t.p3.x));

    min_y = min(t.p1.y, min(t.p2.y, t.p3.y));
    max_y = max(t.p1.y, max(t.p2.y, t.p3.y));

    min_z = min(t.p1.z, min(t.p2.z, t.p3.z));
    max_z = max(t.p1.z, max(t.p2.z, t.p3.z));
    center.x = (max_x + min_x) / 2;
    center.y = (max_y + min_y) / 2;
    center.z = (max_z + min_z) / 2;
  }
  // merge two bounding boxes
  void add(BoundingBox bbx) {
    min_x = min(min_x, bbx.min_x);
    min_y = min(min_y, bbx.min_y);
    min_z = min(min_z, bbx.min_z);

    max_x = max(max_x, bbx.max_x);
    max_y = max(max_y, bbx.max_y);
    max_z = max(max_z, bbx.max_z);
    center.x = (max_x + min_x) / 2;
    center.y = (max_y + min_y) / 2;
    center.z = (max_z + min_z) / 2;
  }
  // compare two bounding boxes
  float compare(BoundingBox bbx, int idx) {
    if (idx == 1)
      return center.x - bbx.center.x;
    else if (idx == 2)
      return center.y - bbx.center.y;
    return center.z - bbx.center.z;
  }
  // check if a ray is intersected with the bounding box
  boolean intersect(Ray r) {
    float tmin, tmax;
    if (r.dir.x >= 0) {
      tmin = (min_x - r.ori.x) * (1.0f / r.dir.x);
      tmax = (max_x - r.ori.x) * (1.0f / r.dir.x);
    } else {
      tmin = (max_x - r.ori.x) * (1.0f / r.dir.x);
      tmax = (min_x - r.ori.x) * (1.0f / r.dir.x);
    }

    float tymin, tymax;
    if (r.dir.y >= 0) {
      tymin = (min_y - r.ori.y) * (1.0f / r.dir.y);
      tymax = (max_y - r.ori.y) * (1.0f / r.dir.y);
    } else {
      tymin = (max_y - r.ori.y) * (1.0f / r.dir.y);
      tymax = (min_y - r.ori.y) * (1.0f / r.dir.y);
    }

    if (tmax < tymin || tymax < tmin) return false;

    tmin = tmin < tymin ? tymin : tmin;
    tmax = tmax > tymax ? tymax : tmax;

    float tzmin, tzmax;
    if (r.dir.z >= 0) {
      tzmin = (min_z - r.ori.z) * (1.0f / r.dir.z);
      tzmax = (max_z - r.ori.z) * (1.0f / r.dir.z);
    } else {
      tzmin = (max_z - r.ori.z) * (1.0f / r.dir.z);
      tzmax = (min_z - r.ori.z) * (1.0f / r.dir.z);
    }
    if (tmax < tzmin || tmin > tzmax) return false;
    return true;
  }
}
// Bounding Volume Hierarchy
class BVH {
  // Binary Tree
  BVH left, right;
  BoundingBox bbx;
  ArrayList<Triangle> mesh;
  BVH(ArrayList<Triangle> mesh) {
    this.mesh = mesh;
    if (this.mesh.size() <= 1) return;
    bbx = new BoundingBox();
    // build bounding box
    for (int i = 0; i < this.mesh.size(); i++) {
      Triangle t = this.mesh.get(i);
      bbx.add(t.bbx);
    }
    // random select an axis and split the mesh into two parts based on the mean
    // value of selected axis
    int i = 0, j = this.mesh.size() - 1;
    int axis = int(random(100)) % 3 + 1;
    while (i < j) {
      Triangle ti = this.mesh.get(i);
      Triangle tj = this.mesh.get(j);
      while (i + 1 < this.mesh.size() && ti.bbx.compare(bbx, axis) < 0) {
        i++;
        ti = this.mesh.get(i);
      }
      while (j - 1 >= 0 && tj.bbx.compare(bbx, axis) >= 0) {
        j--;
        tj = this.mesh.get(j);
      }
      if (i < j) {
        this.mesh.set(i, tj);
        this.mesh.set(j, ti);
      }
    }
    // Build left node and right node
    int pivot;
    if (j == 0)
      pivot = 1;
    else if (i == this.mesh.size() - 1)
      pivot = this.mesh.size() - 1;
    else
      pivot = i + 1;
    ArrayList<Triangle> left_mesh = new ArrayList<Triangle>();
    ArrayList<Triangle> right_mesh = new ArrayList<Triangle>();
    for (int k = 0; k < mesh.size(); k++) {
      if (k < pivot)
        left_mesh.add(this.mesh.get(k));
      else
        right_mesh.add(this.mesh.get(k));
    }
    left = new BVH(left_mesh);
    right = new BVH(right_mesh);
  }
  // check if a ray intersect with current volume
  boolean intersect(Ray r, float[] param) {
    if (mesh.size() == 0) return false;
    if (mesh.size() == 1) {
      Triangle t = mesh.get(0);
      return t.intersect(r, param);
    }
    if (!bbx.intersect(r)) return false;
    boolean left_res = left.intersect(r, param);
    boolean right_res = right.intersect(r, param);
    return left_res || right_res;
  }
}
