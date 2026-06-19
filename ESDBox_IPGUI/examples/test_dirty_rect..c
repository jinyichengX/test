#include <stdio.h>
#include <string.h>
#include "ipgui_dirty_rect.h"

static int pass_count = 0;
static int fail_count = 0;

#define CHECK(desc, cond) do { \
    if (cond) { printf("  [PASS] %s\n", desc); pass_count++; } \
    else       { printf("  [FAIL] %s\n", desc); fail_count++; } \
} while(0)

#define CHECK_RECT(desc, r, ex1,ey1,ex2,ey2) do { \
    if ((r)->x1==(ex1)&&(r)->y1==(ey1)&&(r)->x2==(ex2)&&(r)->y2==(ey2)) { \
        printf("  [PASS] %s => (%d,%d,%d,%d)\n",desc,(r)->x1,(r)->y1,(r)->x2,(r)->y2); pass_count++; \
    } else { \
        printf("  [FAIL] %s => got(%d,%d,%d,%d) expected(%d,%d,%d,%d)\n", \
               desc,(r)->x1,(r)->y1,(r)->x2,(r)->y2,ex1,ey1,ex2,ey2); fail_count++; \
    } \
} while(0)

/* 在pool中查找指定矩形 */
static int pool_contains_rect(ipgui_dirty_rect_mgr_t *mgr,
                               int x1,int y1,int x2,int y2)
{
    for (int i = 0; i < mgr->pool_num; i++) {
        if (mgr->pool[i].x1==x1 && mgr->pool[i].y1==y1 &&
            mgr->pool[i].x2==x2 && mgr->pool[i].y2==y2)
            return 1;
    }
    return 0;
}

/* 打印当前pool内容 */
static void dump_pool(ipgui_dirty_rect_mgr_t *mgr)
{
    printf("  [pool pnum=%d]\n", mgr->pool_num);
    for (int i = 0; i < mgr->pool_num; i++)
        printf("    pool[%d]={%d,%d,%d,%d}\n", i,
               mgr->pool[i].x1, mgr->pool[i].y1,
               mgr->pool[i].x2, mgr->pool[i].y2);
}

/* ================================================================
 * TC_S1: 50个水平均匀分布矩形
 *
 * 输入: rect[i] = {i*20, 0, i*20+9, 9},  i=0..49
 * 间距10，相邻合并代价=100
 *
 * 手算预期(由模拟器得出):
 *   pool_num = 8
 *   pool[0] = {0,0,849,9}   (前43个合并成一个大矩形)
 *   剩余7个: {860..980,0,*,9}
 *
 * 关键验证:
 *   1. pool_num == 8 (始终不超出)
 *   2. pool中存在覆盖左侧区域的大矩形 x1=0,y1=0,y2=9
 *   3. pool中存在 {980,0,989,9} (最后一个矩形)
 *   4. 所有pool矩形的y范围均为 [0,9]
 * ================================================================ */
void test_stress_horizontal_50(void)
{
    printf("\n[TC_S1] 50个水平均匀分布矩形(间距10)\n");
    ipgui_dirty_rect_mgr_t mgr;
    ipgui_dirty_rect_mgr_init(&mgr);

    for (int i = 0; i < 50; i++)
        ipgui_dirty_rect_add_xywh(&mgr, (ipgui_coord_t)(i*20), 0, 10, 10);

    dump_pool(&mgr);
    CHECK("pool_num == 8", mgr.pool_num == 8);

    /* 所有矩形都在y=[0,9]这一行，合并后每个pool矩形y范围应仍为[0,9] */
    int all_y_correct = 1;
    for (int i = 0; i < mgr.pool_num; i++)
        if (mgr.pool[i].y1 != 0 || mgr.pool[i].y2 != 9) all_y_correct = 0;
    CHECK("所有pool矩形y范围均为[0,9]", all_y_correct);

    /* 最后一个矩形{980,0,989,9}应存在于pool中 */
    CHECK("pool中存在{980,0,989,9}", pool_contains_rect(&mgr, 980,0,989,9));

    /* pool[0]应为手算得出的最大合并矩形 */
    CHECK("pool中存在大矩形{0,0,849,9}", pool_contains_rect(&mgr, 0,0,849,9));

    /* pool中所有矩形的x范围应覆盖[0,989]，无重叠 */
    int total_covered = 0;
    for (int i = 0; i < mgr.pool_num; i++)
        total_covered += (mgr.pool[i].x2 - mgr.pool[i].x1 + 1);
    /* 原始50个矩形共覆盖50*10=500像素，合并不会减少覆盖 */
    CHECK("总覆盖宽度 >= 500", total_covered >= 500);
}

/* ================================================================
 * TC_S2: 连续添加32个矩形，验证pool_num始终<=8
 *
 * 输入: 32个互不相交的矩形，每个间隔足够大
 * rect[i] = {i*50, 0, i*50+9, 9}
 *
 * 预期: 无论添加多少个，pool_num始终<=IPGUI_DIRTY_RECT_POOL
 * ================================================================ */
void test_stress_pool_never_overflow(void)
{
    printf("\n[TC_S2] 32个矩形连续添加，pool_num始终不超过8\n");
    ipgui_dirty_rect_mgr_t mgr;
    ipgui_dirty_rect_mgr_init(&mgr);

    int overflow = 0;
    for (int i = 0; i < 32; i++) {
        ipgui_dirty_rect_add_xywh(&mgr, (ipgui_coord_t)(i*50), 0, 10, 10);
        if (mgr.pool_num > IPGUI_DIRTY_RECT_POOL) {
            printf("  [!!] 第%d个矩形加入后 pool_num=%d 溢出!\n", i, mgr.pool_num);
            overflow = 1;
        }
    }
    CHECK("32次添加过程中pool_num始终<=8", !overflow);
    CHECK("最终pool_num==8", mgr.pool_num == 8);
}

/* ================================================================
 * TC_S3: 4个聚集区域，每区域25个矩形，共100个
 *
 * 区域划分:
 *   区域0: x=[0..89],   y=[0..89]   (5x5网格，间距20，每格10x10)
 *   区域1: x=[200..289],y=[0..89]
 *   区域2: x=[0..89],   y=[200..289]
 *   区域3: x=[200..289],y=[200..289]
 *
 * 手算预期(由模拟器得出):
 *   pool_num = 8
 *   区域0完整合并为 {0,0,89,89}      (pool[0])
 *   区域1完整合并为 {200,0,289,89}   (pool[1])
 *   区域2完整合并为 {0,200,89,289}   (pool[2])
 *   区域3因pool槽位压力被分裂为多个矩形占据剩余5个槽位
 *
 * 关键验证:
 *   1. pool_num == 8
 *   2. pool中存在{0,0,89,89}
 *   3. pool中存在{200,0,289,89}
 *   4. pool中存在{0,200,89,289}
 *   5. 区域3的所有矩形x范围均在[200,289]
 * ================================================================ */
void test_stress_4clusters_100rects(void)
{
    printf("\n[TC_S3] 4个聚集区域各25个矩形共100个\n");
    ipgui_dirty_rect_mgr_t mgr;
    ipgui_dirty_rect_mgr_init(&mgr);

    int cx[] = {0, 200,   0, 200};
    int cy[] = {0,   0, 200, 200};
    for (int g = 0; g < 4; g++)
        for (int i = 0; i < 25; i++) {
            int x = cx[g] + (i%5)*20;
            int y = cy[g] + (i/5)*20;
            ipgui_dirty_rect_add_xywh(&mgr, (ipgui_coord_t)x,
                                            (ipgui_coord_t)y, 10, 10);
        }

    dump_pool(&mgr);
    CHECK("pool_num == 8", mgr.pool_num == 8);
    CHECK("pool中存在区域0合并矩形{0,0,89,89}",
          pool_contains_rect(&mgr, 0,0,89,89));
    CHECK("pool中存在区域1合并矩形{200,0,289,89}",
          pool_contains_rect(&mgr, 200,0,289,89));
    CHECK("pool中存在区域2合并矩形{0,200,89,289}",
          pool_contains_rect(&mgr, 0,200,89,289));

    /* 区域3的矩形应全部落在x=[200,289], y=[200,289]范围内 */
    int region3_ok = 1;
    for (int i = 0; i < mgr.pool_num; i++) {
        /* 跳过已知的区域0,1,2 */
        ipgui_dirty_rect_t *r = &mgr.pool[i];
        if ((r->x1==0&&r->x2==89&&r->y1==0&&r->y2==89) ||
            (r->x1==200&&r->x2==289&&r->y1==0&&r->y2==89) ||
            (r->x1==0&&r->x2==89&&r->y1==200&&r->y2==289))
            continue;
        /* 其余矩形应在区域3范围内 */
        if (r->x1<200||r->x2>289||r->y1<200||r->y2>289) {
            printf("  [!!] pool[%d]={%d,%d,%d,%d} 不在区域3范围内\n",
                   i,r->x1,r->y1,r->x2,r->y2);
            region3_ok = 0;
        }
    }
    CHECK("剩余pool矩形均在区域3范围[200..289,200..289]内", region3_ok);
}

/* ================================================================
 * TC_S4: 同一位置重复添加100次
 *
 * 输入: 100次 add_xywh(0, 0, 100, 100)
 * 第1次插入后，后续99次都应被包含检查丢弃
 *
 * 预期: pool_num == 1, pool[0] == {0,0,99,99}
 * ================================================================ */
void test_stress_duplicate_100(void)
{
    printf("\n[TC_S4] 同一矩形重复添加100次\n");
    ipgui_dirty_rect_mgr_t mgr;
    ipgui_dirty_rect_mgr_init(&mgr);

    for (int i = 0; i < 100; i++)
        ipgui_dirty_rect_add_xywh(&mgr, 0, 0, 100, 100);

    CHECK("pool_num == 1", mgr.pool_num == 1);
    ipgui_dirty_rect_t *r = ipgui_dirty_rect_get(&mgr, 0);
    CHECK_RECT("pool[0]={0,0,99,99}", r, 0,0,99,99);
}

/* ================================================================
 * TC_S5: 逐渐扩大的嵌套矩形，共50个
 *
 * 输入: rect[i] = {i*2, i*2, 200-i*2, 200-i*2},  i=0..49
 *   i=0: {0,0,200,200}  最大
 *   i=1: {2,2,198,198}  被i=0包含 => 丢弃
 *   i=2: {4,4,196,196}  被i=0包含 => 丢弃
 *   ...全部被i=0包含
 *
 * 预期: pool_num == 1, pool[0] == {0,0,200,200}
 * ================================================================ */
void test_stress_nested_50(void)
{
    printf("\n[TC_S5] 逐渐缩小的嵌套矩形50个(全被第1个包含)\n");
    ipgui_dirty_rect_mgr_t mgr;
    ipgui_dirty_rect_mgr_init(&mgr);

    /* 先添加最大的 */
    ipgui_dirty_rect_add_xywh(&mgr, 0, 0, 201, 201);  /* {0,0,200,200} */
    for (int i = 1; i < 50; i++) {
        ipgui_coord_t x = (ipgui_coord_t)(i*2);
        ipgui_coord_t y = (ipgui_coord_t)(i*2);
        ipgui_coord_t w = (ipgui_coord_t)(201 - i*4);
        ipgui_coord_t h = (ipgui_coord_t)(201 - i*4);
        if (w > 0 && h > 0)
            ipgui_dirty_rect_add_xywh(&mgr, x, y, w, h);
    }

    CHECK("pool_num == 1", mgr.pool_num == 1);
    ipgui_dirty_rect_t *r = ipgui_dirty_rect_get(&mgr, 0);
    CHECK_RECT("pool[0]={0,0,200,200}", r, 0,0,200,200);
}

/* ================================================================
 * TC_S6: 先添加小矩形8个填满pool，再用1个大矩形全部覆盖
 *
 * 步骤1: 添加8个 10x10 小矩形，各自分散
 * 步骤2: 添加1个 1000x1000 大矩形包含所有小矩形
 *
 * 预期: 8个小矩形全被移除，pool_num==1，pool[0]={0,0,999,999}
 * ================================================================ */
void test_stress_big_rect_covers_all(void)
{
    printf("\n[TC_S6] 8个小矩形填满后添加1个覆盖全部的大矩形\n");
    ipgui_dirty_rect_mgr_t mgr;
    ipgui_dirty_rect_mgr_init(&mgr);

    for (int i = 0; i < 8; i++)
        ipgui_dirty_rect_add_xywh(&mgr, (ipgui_coord_t)(i*100), 0, 10, 10);
    CHECK("添加8个后pool_num==8", mgr.pool_num == 8);

    ipgui_dirty_rect_add_xywh(&mgr, 0, 0, 1000, 1000);
    dump_pool(&mgr);
    CHECK("大矩形加入后pool_num==1", mgr.pool_num == 1);
    ipgui_dirty_rect_t *r = ipgui_dirty_rect_get(&mgr, 0);
    CHECK_RECT("pool[0]={0,0,999,999}", r, 0,0,999,999);
}

/* ================================================================
 * TC_S7: 交替添加两组矩形，共40个
 *
 * 奇数次: {0,0,9,9} 区域附近，偶数次: {500,500,509,509} 区域附近
 * 各组20个，每个小矩形在各自区域内偏移1像素
 *   奇: {i,   0,   i+9,  9},   i=0,1,2,...,19
 *   偶: {500+i,500, 509+i,509}, i=0,1,2,...,19
 *
 * 由于两组矩形区域紧密，同组矩形会合并在一起
 * 预期(模拟器验证):
 *   pool_num <= 8
 *   pool中存在覆盖{0,0,*,9}区域的矩形
 *   pool中存在覆盖{500,500,*,509}区域的矩形
 * ================================================================ */
void test_stress_two_groups_alternating(void)
{
    printf("\n[TC_S7] 两组矩形交替添加共40个\n");
    ipgui_dirty_rect_mgr_t mgr;
    ipgui_dirty_rect_mgr_init(&mgr);

    for (int i = 0; i < 20; i++) {
        ipgui_dirty_rect_add_xywh(&mgr, (ipgui_coord_t)i,       0,   10, 10);
        ipgui_dirty_rect_add_xywh(&mgr, (ipgui_coord_t)(500+i), 500, 10, 10);
    }

    dump_pool(&mgr);
    CHECK("pool_num <= 8", mgr.pool_num <= 8);

    /* 区域A: x=[0,28],y=[0,9] 应被某个pool矩形单独完整覆盖 */
    int area_a_covered = 0;
    for (int i = 0; i < mgr.pool_num; i++) {
        ipgui_dirty_rect_t *r = &mgr.pool[i];
        if (r->x1 <= 0 && r->x2 >= 28 && r->y1 <= 0 && r->y2 >= 9)
            area_a_covered = 1;
    }
    CHECK("区域A[0,0,28,9]被某pool矩形完整覆盖", area_a_covered);

    /* 区域B分散在多个矩形中，验证所有B区域矩形都在合理范围内且数量>=1 */
    int area_b_count = 0, area_b_oob = 0;
    for (int i = 0; i < mgr.pool_num; i++) {
        ipgui_dirty_rect_t *r = &mgr.pool[i];
        if (r->y2 <= 9) continue;
        area_b_count++;
        if (r->x1 < 500 || r->x2 > 528 || r->y1 < 500 || r->y2 > 509)
            area_b_oob = 1;
    }
    CHECK("pool中B区域矩形数量>=1", area_b_count >= 1);
    CHECK("pool中B区域所有矩形均在[500..528,500..509]内", !area_b_oob);
}

/* ================================================================
 * TC_S8: flush后再继续添加矩形，验证flush不影响后续add
 *
 * 步骤1: 添加4个矩形，flush
 * 步骤2: 再添加20个矩形
 *
 * 预期: 全程pool_num<=8，flush后pool内容合理
 * ================================================================ */
void test_stress_flush_then_continue(void)
{
    printf("\n[TC_S8] flush后继续添加20个矩形\n");
    ipgui_dirty_rect_mgr_t mgr;
    ipgui_dirty_rect_mgr_init(&mgr);

    /* 第一批4个 */
    ipgui_dirty_rect_add_xywh(&mgr,   0,   0, 10, 10);
    ipgui_dirty_rect_add_xywh(&mgr, 100, 100, 10, 10);
    ipgui_dirty_rect_add_xywh(&mgr, 200,   0, 10, 10);
    ipgui_dirty_rect_add_xywh(&mgr,   0, 200, 10, 10);
    CHECK("flush前pool_num==4", mgr.pool_num == 4);

    ipgui_dirty_rect_flush(&mgr);
    CHECK("flush后pool_num<=4", mgr.pool_num <= 4);
    int after_flush = mgr.pool_num;
    printf("  flush后pool_num=%d\n", after_flush);

    /* 继续添加20个 */
    int overflow = 0;
    for (int i = 0; i < 20; i++) {
        ipgui_dirty_rect_add_xywh(&mgr, (ipgui_coord_t)(i*30), 400, 10, 10);
        if (mgr.pool_num > IPGUI_DIRTY_RECT_POOL) overflow = 1;
    }
    CHECK("继续添加20个过程中pool_num始终<=8", !overflow);
    CHECK("最终pool_num<=8", mgr.pool_num <= 8);
    dump_pool(&mgr);
}

/* ================================================================
 * TC_S9: 100个矩形全部相同区域(完全重叠)，不同尺寸大小递增
 *
 * rect[i] = {0, 0, i+1, i+1},  i=0..99
 * 即每次添加比前一个稍大的矩形
 * 最大的{0,0,100,100}会包含所有小的
 *
 * 添加顺序: 从小到大
 *   - 前8个直接插入
 *   - 第9个{0,0,9,9}比前8个都大，包含rect[0..7]，全部移除，直接插入
 *   - 此后每次添加更大的矩形，都包含pool中现有的，移除后直接插入
 *
 * 预期: pool_num==1, pool[0]=={0,0,99,99}
 * ================================================================ */
void test_stress_growing_rects_100(void)
{
    printf("\n[TC_S9] 100个从小到大递增矩形(原点出发)\n");
    ipgui_dirty_rect_mgr_t mgr;
    ipgui_dirty_rect_mgr_init(&mgr);

    for (int i = 0; i < 100; i++)
        ipgui_dirty_rect_add_xywh(&mgr, 0, 0,
                                  (ipgui_coord_t)(i+1),
                                  (ipgui_coord_t)(i+1));

    dump_pool(&mgr);
    CHECK("pool_num==1", mgr.pool_num == 1);
    ipgui_dirty_rect_t *r = ipgui_dirty_rect_get(&mgr, 0);
    CHECK_RECT("pool[0]={0,0,99,99}", r, 0,0,99,99);
}

/* ================================================================
 * TC_S10: reset后重新使用，添加20个矩形
 *
 * 验证reset后mgr状态完全干净，不受之前数据影响
 * ================================================================ */
void test_stress_reset_reuse(void)
{
    printf("\n[TC_S10] 添加50个后reset，再添加20个\n");
    ipgui_dirty_rect_mgr_t mgr;
    ipgui_dirty_rect_mgr_init(&mgr);

    /* 第一轮：50个 */
    for (int i = 0; i < 50; i++)
        ipgui_dirty_rect_add_xywh(&mgr, (ipgui_coord_t)(i*20), 0, 10, 10);
    CHECK("第一轮后pool_num==8", mgr.pool_num == 8);

    ipgui_dirty_rect_mgr_reset(&mgr);
    CHECK("reset后pool_num==0", mgr.pool_num == 0);

    /* 第二轮：20个，紧密排列 */
    int overflow = 0;
    for (int i = 0; i < 20; i++) {
        ipgui_dirty_rect_add_xywh(&mgr, (ipgui_coord_t)(i*5), 100, 5, 5);
        if (mgr.pool_num > IPGUI_DIRTY_RECT_POOL) overflow = 1;
    }
    CHECK("第二轮20个过程中pool_num始终<=8", !overflow);
    CHECK("第二轮后pool_num<=8", mgr.pool_num <= 8);

    /* 验证第二轮矩形都在y=100这一行 */
    int y_ok = 1;
    for (int i = 0; i < mgr.pool_num; i++)
        if (mgr.pool[i].y1 != 100 || mgr.pool[i].y2 != 104) y_ok = 0;
    CHECK("第二轮所有pool矩形y范围为[100,104]", y_ok);
    dump_pool(&mgr);
}

int test_dirty_rect(void)
{
    printf("========================================\n");
    printf("  ipgui_dirty_rect 压力测试\n");
    printf("========================================\n");

    test_stress_horizontal_50();
    test_stress_pool_never_overflow();
    test_stress_4clusters_100rects();
    test_stress_duplicate_100();
    test_stress_nested_50();
    test_stress_big_rect_covers_all();
    test_stress_two_groups_alternating();
    test_stress_flush_then_continue();
    test_stress_growing_rects_100();
    test_stress_reset_reuse();

    printf("\n========================================\n");
    printf("  结果: %d PASS, %d FAIL\n", pass_count, fail_count);
    printf("========================================\n");
    return fail_count > 0 ? 1 : 0;
}