#include <cmath>
#include <cstddef>

class Transform2 {//笛卡尔坐标系
  public:
    typedef struct {
      double x, y;
    }vec2_t;

    Transform2() : pos_{0,0}, angle_(0.0), r00_(1.0), r01_(0.0), r10_(0.0), r11_(1.0) {}
    ~Transform2() = default;

    void SetPos(const vec2_t& pos) {pos_ = pos;}
    void SetAngle(double angle) {
      angle_ = angle;
      double rad = angle_ * M_PI / 180.0;
      r00_ = std::cos(rad);
      r01_ = -std::sin(rad);
      r10_ = std::sin(rad);
      r11_ = std::cos(rad);
    }
    vec2_t GetPos(void) const {return pos_;}
    double GetAngle(void) const {return angle_;}

    // 单点变换
    //知道子坐标系和父坐标系的位姿关系，且知道了子系下的一个点xy，就可以计算出这个点在父坐标系下的位置
    inline vec2_t PosTransform(const vec2_t& pos_in_child) const
    {
      vec2_t pos;
      pos.x = pos_.x + r00_ * pos_in_child.x + r01_ * pos_in_child.y;
      pos.y = pos_.y + r10_ * pos_in_child.x + r11_ * pos_in_child.y;
      return pos;
    }

    //批量变换：n 个点连续存放于 pts，原地写回
    inline void PosTransformBatch(vec2_t * pts, size_t n) const
    {
      const double r00 = r00_, r01 = r01_;
      const double r10 = r10_, r11 = r11_;
      const double px = pos_.x, py = pos_.y;

      for (size_t i = 0; i < n; ++i) {
        double x = pts[i].x;
        double y = pts[i].y;
        pts[i].x = px + r00 * x + r01 * y;
        pts[i].y = py + r10 * x + r11 * y;
      }
    }

    //将角度转换到(-360, 360)开区间
    double AngleTransform(void)
    {
      return std::fmod(angle_, 360);//std::fmod计算两个浮点数相除的余数
    }

  //该坐标系相对于父坐标系的位置和角度
  private:
    vec2_t pos_;//rel to parent position
    double angle_;//rel to parent angle, 角度制例如35°

    //rotation matrix cache(from angle_)
    double r00_, r01_, r10_, r11_;
};

//该变换遵循ROS2的右手系，且是子坐标系相对于父坐标系的变换
//ROS2采用的方案称为固定（或静态）帧RPY。 这意味着三个单独的旋转都应用于原始的、静止的坐标轴上（外旋X--->Y--->Z）
class Transform3 {
  public:
    typedef struct {
      float x, y, z;
    }vec3_t;

    typedef struct{
      float w, x, y, z;
    }quat_t;

    Transform3() : pos_{0,0,0}, roll_(0.0f), pitch_(0.0f), yaw_(0.0f),
                   r00_(1.0), r01_(0.0), r02_(0.0),
                   r10_(0.0), r11_(1.0), r12_(0.0),
                   r20_(0.0), r21_(0.0), r22_(1.0) {}
    ~Transform3() = default;

    void SetPos(const vec3_t& pos) { pos_ = pos; }
    //RPY 一起改，没有只改一个的情况
    void SetRPY(float roll, float pitch, float yaw) {
      roll_ = roll; pitch_ = pitch; yaw_ = yaw;
      UpdateMatrix();
    }

    vec3_t GetPos(void) const { return pos_; }
    float  GetRoll (void) const { return roll_; }
    float  GetPitch(void) const { return pitch_; }
    float  GetYaw  (void) const { return yaw_; }

    // 单点变换
    // R 已缓存，单点接口适合少量点（位姿/标注框角点）
    inline vec3_t PosTransform(const vec3_t &pos_in_child) const
    {
      vec3_t pos;
      pos.x = pos_.x + r00_ * pos_in_child.x + r01_ * pos_in_child.y + r02_ * pos_in_child.z;
      pos.y = pos_.y + r10_ * pos_in_child.x + r11_ * pos_in_child.y + r12_ * pos_in_child.z;
      pos.z = pos_.z + r20_ * pos_in_child.x + r21_ * pos_in_child.y + r22_ * pos_in_child.z;
      return pos;
    }

    //批量变换：n 个点连续存放于 pts，原地写回
    inline void PosTransformBatch(vec3_t * pts, size_t n) const
    {
      const double r00 = r00_, r01 = r01_, r02 = r02_;
      const double r10 = r10_, r11 = r11_, r12 = r12_;
      const double r20 = r20_, r21 = r21_, r22 = r22_;
      const float px = pos_.x, py = pos_.y, pz = pos_.z;

      for (size_t i = 0; i < n; ++i) {
        double x = pts[i].x;
        double y = pts[i].y;
        double z = pts[i].z;
        pts[i].x = px + r00 * x + r01 * y + r02 * z;
        pts[i].y = py + r10 * x + r11 * y + r12 * z;
        pts[i].z = pz + r20 * x + r21 * y + r22 * z;
      }
    }

    //子坐标系下的一个姿态在父坐标系下的姿态
    inline quat_t OrientationTransform(const vec3_t& rpy_in_child) const
    {

    }
    
    //子坐标系下的一个姿态在父坐标系下的姿态
    inline quat_t OrientationTransform(const quat_t& q_in_child) const
    {

    }

    //先作用q2，再作用q1
    quat_t QuatMul(const quat_t & q1, const quat_t & q2)
    {
      return {
        q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z,
        q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
        q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x,
        q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w,
      };
    }

  private:
    vec3_t pos_;//rel to parent position
    float  roll_, pitch_, yaw_;//子坐标系相对于父坐标系的RPY角度, 角度制

    //rotation matrix cache(from RPY), row-major
    double r00_, r01_, r02_;
    double r10_, r11_, r12_;
    double r20_, r21_, r22_;

    void UpdateMatrix(void)
    {
      //转弧度
      double rad_r = roll_  * M_PI / 180.0;
      double rad_p = pitch_ * M_PI / 180.0;
      double rad_y = yaw_   * M_PI / 180.0;

      double cr = std::cos(rad_r), sr = std::sin(rad_r);
      double cp = std::cos(rad_p), sp = std::sin(rad_p);
      double cy = std::cos(rad_y), sy = std::sin(rad_y);

      //外旋 X→Y→Z（固定帧）：R = Rz · Ry · Rx
      r00_ = cy * cp;
      r01_ = cy * sp * sr - sy * cr;
      r02_ = cy * sp * cr + sy * sr;
      r10_ = sy * cp;
      r11_ = sy * sp * sr + cy * cr;
      r12_ = sy * sp * cr - cy * sr;
      r20_ = -sp;
      r21_ = cp * sr;
      r22_ = cp * cr;
    }
};
