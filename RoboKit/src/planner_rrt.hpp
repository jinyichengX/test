#include <vector>
#include <random>
#include <iostream>
#include <cmath>

namespace test {

    struct Vec2 {
        double x, y;
        Vec2(double x_, double y_): x(x_), y(y_) {}
    };

    class PointNode {
        public:
            Vec2 point;
            int parent_idx = -1;  // 父节点索引，-1 表示无父（根节点）
            PointNode(Vec2 p): point(p) {}
    };

    class RRTPlanner {
        private:
            std::random_device rd_;
            std::mt19937 gen_;

            //地图边界
            Vec2 map_min_, map_max_;

            double step_size_;// 单次路径步长
            int iter_max;//最大迭代次数

            //最终生成的路径点序列
            std::vector<Vec2> path;
        public:
            //定义一个容器，存储节点
            std::vector<PointNode> node_array_;
            
            RRTPlanner(const Vec2& map_min, const Vec2& map_max, double step_size = 5.0, int iter_max = 1000)   // 默认值 5.0
                : rd_(), gen_(rd_()), map_min_(map_min), map_max_(map_max), step_size_(step_size), iter_max(iter_max) {}
            ~RRTPlanner() = default;

            //获取最终生成的回溯路径
            std::vector<Vec2> GetPath() {
                return path;
            }

            //从笛卡尔坐标系左下min到右上max的范围内均匀生成随机点
            Vec2 GenRandomPoint() {
                std::uniform_real_distribution<double> dis_x(map_min_.x, map_max_.x);
                std::uniform_real_distribution<double> dis_y(map_min_.y, map_max_.y);
                return Vec2(dis_x(gen_), dis_y(gen_));
            }

            //返回 node_array 中距离 p 最近的节点索引；数组为空返回 -1
            int SearchNearestNode(const std::vector<PointNode>& node_array, const Vec2& p) {
                if (node_array.empty()) {
                    return -1;
                }

                int nearest_idx = 0;
                double dx0 = node_array[0].point.x - p.x;
                double dy0 = node_array[0].point.y - p.y;
                double min_dist_pow = dx0 * dx0 + dy0 * dy0;

                for (size_t i = 1; i < node_array.size(); i++) {
                    double dx = node_array[i].point.x - p.x;
                    double dy = node_array[i].point.y - p.y;
                    double dist_pow = dx * dx + dy * dy;
                    if (dist_pow < min_dist_pow) {
                        min_dist_pow = dist_pow;
                        nearest_idx = (int)i;
                    }
                }
                return nearest_idx;
            }

            Vec2 Normalize(const Vec2& v) {
                double mod = std::sqrt(v.x * v.x + v.y * v.y);
                if (mod < 1e-9) {
                    return Vec2(0.0, 0.0);
                }
                return Vec2(v.x / mod, v.y / mod);
            }

            void AddNewPoint(const Vec2& p, int par_idx) {
                PointNode new_pn(p);
                new_pn.parent_idx = par_idx;   // 存索引，不存指针（vector 扩容会使指针失效）
                node_array_.push_back(new_pn);
            }

            void DoRRTPlan(const Vec2& start, const Vec2& end) {
                //push起点
                PointNode node_start(start);
                node_array_.push_back(node_start);

                int iter = 0;
                while (iter ++ < iter_max) {
                    //生成随机点
                    Vec2 random = GenRandomPoint();
                    //找到最近的节点并计算出向量
                    int nearest_idx = SearchNearestNode(node_array_, random);
                    if (nearest_idx == -1) {
                        std::cerr << "Error: No nearest node found." << std::endl;
                        return;
                    }

                    //标准化向量并乘以步长
                    Vec2 dir(random.x - node_array_[nearest_idx].point.x, random.y - node_array_[nearest_idx].point.y);
                    dir = Normalize(dir);
                    if (dir.x == 0.0 && dir.y == 0.0) {
                        continue;
                    }
                    dir.x *= step_size_;
                    dir.y *= step_size_;

                    Vec2 new_point(dir.x + node_array_[nearest_idx].point.x, dir.y + node_array_[nearest_idx].point.y);
                    AddNewPoint(new_point, nearest_idx);
                    
                    //新节点距离终点是否小于等于步长
                    if (std::sqrt((new_point.x - end.x) * (new_point.x - end.x) +
                        (new_point.y - end.y) * (new_point.y - end.y)) <= step_size_) {
                        //添加终点
                        AddNewPoint(end, node_array_.size() - 1);

                        //回溯路径（沿 parent_idx 链回溯到根）
                        path.push_back(end);
                        int cur_idx = (int)node_array_.size() - 2;   // 新节点索引
                        while (cur_idx != -1) {
                            path.push_back(node_array_[cur_idx].point);
                            cur_idx = node_array_[cur_idx].parent_idx;
                        }

                        return;
                    }
                }
                std::cerr << "Error: Maximum iterations reached." << std::endl;
                return;
            }
    };

}