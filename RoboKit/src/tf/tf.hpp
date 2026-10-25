#include <cmath>

class Transform2 {//笛卡尔坐标系
  public:
    typedef struct {
      double x, y;
    }vec2_t;

    Transform2() : pos_{0,0}, angle_(0.0) {}
    ~Transform2() = default;

    void SetPos(const vec2_t& pos) {pos_ = pos;}
    void SetAngle(double angle) {angle_ = angle;}
    vec2_t GetPos(void) const {return pos_;}
    double GetAngle(void) const {return angle_;}

    //知道子坐标系和父坐标系的位姿关系，且知道了子系下的一个点xy，就可以计算出这个点在父坐标系下的位置
    vec2_t PosTransform(const vec2_t& pos_in_child) const
    {
      vec2_t pos;
      vec2_t pos_temp;

      //转成弧度制
      double rad = angle_ * M_PI / 180.0;
      pos_temp.x = pos_in_child.x * std::cos(rad) - pos_in_child.y * std::sin(rad);
      pos_temp.y = pos_in_child.x * std::sin(rad) + pos_in_child.y * std::cos(rad);

      pos.x = pos_.x + pos_temp.x;
      pos.y = pos_.y + pos_temp.y;

      return pos;
    }

    //将角度转换到(-360, 360)开区间
    void AngleTransform(void)
    {
      angle_ = std::fmod(angle_, 360);//std::fmod计算两个浮点数相除的余数
    }

  //该坐标系相对于父坐标系的位置和角度
  private:
    vec2_t pos_;//rel to parent position
    double angle_;//rel to parent angle, 角度制例如35°
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

    Transform3() : pos_{0,0,0}, roll_(0.0), pitch_(0.0), yaw_(0.0) {}
    ~Transform3() = default;

    vec3_t PosTransform(const vec3_t &pos_in_child) const
    {
      // 转弧度
      double rad_r = roll_  * M_PI / 180.0;
      double rad_p = pitch_ * M_PI / 180.0;
      double rad_y = yaw_   * M_PI / 180.0;

      double cr = std::cos(rad_r), sr = std::sin(rad_r);
      double cp = std::cos(rad_p), sp = std::sin(rad_p);
      double cy = std::cos(rad_y), sy = std::sin(rad_y);

      // 外旋 X→Y→Z（固定帧）：R = Rz · Ry · Rx
      double r00 = cy * cp;
      double r01 = cy * sp * sr - sy * cr;
      double r02 = cy * sp * cr + sy * sr;
      double r10 = sy * cp;
      double r11 = sy * sp * sr + cy * cr;
      double r12 = sy * sp * cr - cy * sr;
      double r20 = -sp;
      double r21 = cp * sr;
      double r22 = cp * cr;

      // p_parent = R · p_child + pos_
      vec3_t pos;
      pos.x = pos_.x + r00 * pos_in_child.x + r01 * pos_in_child.y + r02 * pos_in_child.z;
      pos.y = pos_.y + r10 * pos_in_child.x + r11 * pos_in_child.y + r12 * pos_in_child.z;
      pos.z = pos_.z + r20 * pos_in_child.x + r21 * pos_in_child.y + r22 * pos_in_child.z;

      return pos;
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
    float roll_, pitch_, yaw_;//rel to parent angle
};