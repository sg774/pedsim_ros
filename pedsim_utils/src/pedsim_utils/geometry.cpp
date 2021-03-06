
#include <pedsim_utils/geometry.h>
#include <ros/ros.h>
namespace pedsim {

geometry_msgs::Quaternion angleToQuaternion(const double theta) {
  Eigen::Matrix3f rotation_matrix(Eigen::Matrix3f::Identity());
  rotation_matrix(0, 0) = std::cos(theta);
  rotation_matrix(0, 1) = -std::sin(theta);
  rotation_matrix(0, 2) = 0;
  rotation_matrix(1, 0) = std::sin(theta);
  rotation_matrix(1, 1) = std::cos(theta);
  rotation_matrix(1, 2) = 0;
  rotation_matrix(2, 0) = 0;
  rotation_matrix(2, 1) = 0;
  rotation_matrix(2, 2) = 1;

  Eigen::Quaternionf quaternion(rotation_matrix);
  return toQuaternionMsg(quaternion.normalized());
}

geometry_msgs::Quaternion rpyToQuaternion(const double roll, const double pitch,
                                          const double yaw) {
  Eigen::Quaternionf r_m = Eigen::AngleAxisf(roll, Eigen::Vector3f::UnitX()) *
                           Eigen::AngleAxisf(pitch, Eigen::Vector3f::UnitY()) *
                           Eigen::AngleAxisf(yaw, Eigen::Vector3f::UnitZ());

  return toQuaternionMsg(r_m.normalized());
}

geometry_msgs::Quaternion toQuaternionMsg(
    const Eigen::Quaternionf& quaternion) {
  geometry_msgs::Quaternion gq;
  gq.x = quaternion.x();
  gq.y = quaternion.y();
  gq.z = quaternion.z();
  gq.w = quaternion.w();
  return std::move(gq);
}

geometry_msgs::Quaternion poseFrom2DVelocity(const double vx, const double vy) {
  const double theta = std::atan2(vy, vx);
  return rpyToQuaternion(M_PI / 2.0, theta + (M_PI / 2.0), 0.0);
}

std::vector<std::pair<float, float>> LineObstacleToCells(const float x1,
                                                         const float y1,
                                                         const float x2,
                                                         const float y2) {
  int i;             // loop counter
  int ystep, xstep;  // the step on y and x axis
  int error;         // the error accumulated during the increment
  int errorprev;     // *vision the previous value of the error variable
  // int y = y1 - 0.5, x = x1 - 0.5;  // the line points
  float y = y1, x = x1;  // the line points
  float ddy, ddx;        // compulsory variables: the double values of dy and dx
  float dx = x2 - x1 - 0.5;// reduce by the obstacle half size
  float dy = y2 - y1 - 0.5;
  double unit_x, unit_y;
  unit_x = 0.5;
  unit_y = 0.5;
    x +=0.5; // enlarge by the obstacle half size
    y +=0.5; // enlarge by the obstacle half size
  ddy = 2 * dy;  // work with double values for full precision
  ddx = 2 * dx;

  std::vector<std::pair<float, float>> obstacle_cells;  // TODO - reserve.
  obstacle_cells.emplace_back(std::make_pair(x, y));

  if (ddx >= ddy) {
    // first octant (0 <= slope <= 1)
    // compulsory initialization (even for errorprev, needed when dx==dy)
    errorprev = error = dx;  // start in the middle of the square
    for (i = 0; i < dx/unit_x-1; i++) {
      // do not use the first point (already done)
      x += unit_x;
        //ROS_INFO_STREAM("x: " << x);
      obstacle_cells.emplace_back(std::make_pair(x, y));

    }
  } else {
    // the same as above
    errorprev = error = dy;
    for (i = 0; i < dy; i++) {
      y += ystep;
      error += ddx;
      if (error > ddy) {
        x += xstep;
        error -= ddy;
        if (error + errorprev < ddy) {
          obstacle_cells.emplace_back(std::make_pair(x - xstep, y));
        } else if (error + errorprev > ddy) {
          obstacle_cells.emplace_back(std::make_pair(x, y - ystep));
        } else {
          obstacle_cells.emplace_back(std::make_pair(x, y - ystep));
          obstacle_cells.emplace_back(std::make_pair(x - xstep, y));
        }
      }
      obstacle_cells.emplace_back(std::make_pair(x, y));
      errorprev = error;
    }
  }
  return obstacle_cells;
}

}  // namespace pedsim
