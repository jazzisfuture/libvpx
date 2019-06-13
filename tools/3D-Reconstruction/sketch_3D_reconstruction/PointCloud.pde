class PointCloud {
  ArrayList<PVector> points;  // array to save points
  IntList point_colors;       // array to save points color
  PVector cloud_mass;
  PointCloud() {
    // initialize
    points = new ArrayList<PVector>();
    point_colors = new IntList();
    cloud_mass = new PVector(0, 0, 0);
  }

  void generate(PImage rgb, PImage depth, Transform trans) {
    for (int v = 0; v < depth.height; v++)
      for (int u = 0; u < depth.width; u++) {
        // get depth value (red channel)
        color depth_px = depth.get(u, v);
        int d = depth_px & 0x0000FFFF;
        // only transform the pixel whose depth is not 0
        if (d > 0) {
          // add transformed pixel as well as pixel color to the list
          PVector pos = trans.transform(u, v, d);
          points.add(pos);
          point_colors.append(rgb.get(u, v));
          // accumulate z value
          cloud_mass = PVector.add(cloud_mass, pos);
        }
      }
  }
  
  int size()
  {
    return points.size();
  }
  
  PVector getPosition(int i)
  {
    if(i>=points.size())
    {
      println("point position: index "+str(i)+" exceeds");
      exit();
    }
    return points.get(i);
  }
  
  color getColor(int i)
  {
    if(i>=point_colors.size())
    {
      println("point color: index "+str(i)+" exceeds");
      exit();
    }
    return point_colors.get(i);
  }
  
  PVector getCloudCenter() {
    if (points.size() > 0) return PVector.div(cloud_mass, points.size());
    return new PVector(0, 0, 0);
  }
  
  void merge(PointCloud point_cloud)
  {
    for(int i=0;i<point_cloud.size();i++)
    {
      points.add(point_cloud.getPosition(i));
      point_colors.append(point_cloud.getColor(i));
    }
    cloud_mass = PVector.add(cloud_mass,point_cloud.cloud_mass);
  }
}
