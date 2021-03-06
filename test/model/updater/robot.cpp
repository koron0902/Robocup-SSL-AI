#define BOOST_TEST_DYN_LINK

#include <boost/math/constants/constants.hpp>
#include <boost/test/unit_test.hpp>

#include "ai/filter/base.hpp"
#include "ai/model/updater/robot.hpp"
#include "ai/util/math/affine.hpp"

using namespace std::chrono_literals;

namespace filter = ai::filter;
namespace model  = ai::model;
namespace util   = ai::util;

// 角度をいい感じに作りたかったので
constexpr double rad(double _deg) {
  using namespace boost::math::double_constants;
  return _deg * pi / 180;
}

BOOST_AUTO_TEST_SUITE(updater_robot)

// チームカラーに対する特殊化がちゃんと機能しているか
BOOST_AUTO_TEST_CASE(color, *boost::unit_test::tolerance(0.0000001)) {
  model::updater::robot<model::teamColor::Blue> rbu;
  model::updater::robot<model::teamColor::Yellow> ryu;

  {
    ssl_protos::vision::DetectionFrame f;
    f.set_camera_id(0);

    auto rb1 = f.add_robots_blue();
    rb1->set_robot_id(1);
    rb1->set_x(10);
    rb1->set_y(20);
    rb1->set_orientation(rad(30));
    rb1->set_confidence(90.0);

    auto ry2 = f.add_robots_yellow();
    ry2->set_robot_id(2);
    ry2->set_x(40);
    ry2->set_y(50);
    ry2->set_orientation(rad(60));
    ry2->set_confidence(90.0);

    rbu.update(f);
    ryu.update(f);
  }

  {
    ai::model::robot r;

    // 青ロボット用updaterから青ロボットの値が取れる
    const auto rb = rbu.value();
    BOOST_TEST(rb.size() == 1);
    BOOST_CHECK_NO_THROW(r = rb.at(1));
    BOOST_TEST(r.id() == 1);
    BOOST_TEST(r.x() == 10);
    BOOST_TEST(r.y() == 20);
    BOOST_TEST(r.theta() == rad(30));

    // 黄ロボット用updaterから黄ロボットの値が取れる
    const auto ry = ryu.value();
    BOOST_TEST(ry.size() == 1);
    BOOST_CHECK_NO_THROW(r = ry.at(2));
    BOOST_TEST(r.id() == 2);
    BOOST_TEST(r.x() == 40);
    BOOST_TEST(r.y() == 50);
    BOOST_TEST(r.theta() == rad(60));
  }
}

BOOST_AUTO_TEST_CASE(normal, *boost::unit_test::tolerance(0.0000001)) {
  model::updater::robot<model::teamColor::Blue> ru;

  {
    ssl_protos::vision::DetectionFrame f;
    f.set_camera_id(0);

    auto rb1 = f.add_robots_blue();
    rb1->set_robot_id(1);
    rb1->set_x(10);
    rb1->set_y(11);
    rb1->set_orientation(rad(12));
    rb1->set_confidence(94.0);

    auto rb3 = f.add_robots_blue();
    rb3->set_robot_id(3);
    rb3->set_x(30);
    rb3->set_y(31);
    rb3->set_orientation(rad(32));
    rb3->set_confidence(95.0);

    auto rb5 = f.add_robots_blue();
    rb5->set_robot_id(5);
    rb5->set_x(50);
    rb5->set_y(51);
    rb5->set_orientation(rad(52));
    rb5->set_confidence(96.0);

    ru.update(f);
  }

  {
    ai::model::robot r;
    const auto rb = ru.value();

    // 検出された青ロボは3台
    BOOST_TEST(rb.size() == 3);

    // ID1の青ロボが存在 (ID1の要素を参照してもstd::out_of_range例外が飛ばない)
    BOOST_CHECK_NO_THROW(r = rb.at(1));
    BOOST_TEST(r.id() == 1);
    BOOST_TEST(r.x() == 10);
    BOOST_TEST(r.y() == 11);
    BOOST_TEST(r.theta() == rad(12));

    // ID3の青ロボが存在
    BOOST_CHECK_NO_THROW(r = rb.at(3));
    BOOST_TEST(r.id() == 3);
    BOOST_TEST(r.x() == 30);
    BOOST_TEST(r.y() == 31);
    BOOST_TEST(r.theta() == rad(32));

    // ID5の青ロボが存在
    BOOST_CHECK_NO_THROW(r = rb.at(5));
    BOOST_TEST(r.id() == 5);
    BOOST_TEST(r.x() == 50);
    BOOST_TEST(r.y() == 51);
    BOOST_TEST(r.theta() == rad(52));
  }

  {
    ssl_protos::vision::DetectionFrame f;
    f.set_camera_id(1);

    // 最初よりもconfidenceが高いデータ
    auto rb1 = f.add_robots_blue();
    rb1->set_robot_id(1);
    rb1->set_x(13);
    rb1->set_y(14);
    rb1->set_orientation(rad(15));
    rb1->set_confidence(95.0);

    // 新たに検出されたIDのデータ
    auto rb2 = f.add_robots_blue();
    rb2->set_robot_id(2);
    rb2->set_x(20);
    rb2->set_y(21);
    rb2->set_orientation(rad(22));
    rb2->set_confidence(94.0);

    // 最初よりもconfidenceが低いデータ
    auto rb5 = f.add_robots_blue();
    rb5->set_robot_id(5);
    rb5->set_x(53);
    rb5->set_y(54);
    rb5->set_orientation(rad(55));
    rb5->set_confidence(93.0);

    ru.update(f);
  }

  {
    ai::model::robot r;
    const auto rb = ru.value();

    // 検出された青ロボは4台
    BOOST_TEST(rb.size() == 4);

    // ID1の青ロボが存在
    // cam1で検出されたID1の青ロボのほうがconfidenceが高いので値が更新される
    BOOST_CHECK_NO_THROW(r = rb.at(1));
    BOOST_TEST(r.id() == 1);
    BOOST_TEST(r.x() == 13);
    BOOST_TEST(r.y() == 14);
    BOOST_TEST(r.theta() == rad(15));

    // ID2の青ロボが存在
    BOOST_CHECK_NO_THROW(r = rb.at(2));
    BOOST_TEST(r.id() == 2);
    BOOST_TEST(r.x() == 20);
    BOOST_TEST(r.y() == 21);
    BOOST_TEST(r.theta() == rad(22));

    // ID3の青ロボが存在, 値の変化なし
    BOOST_CHECK_NO_THROW(r = rb.at(3));
    BOOST_TEST(r.id() == 3);
    BOOST_TEST(r.x() == 30);
    BOOST_TEST(r.y() == 31);
    BOOST_TEST(r.theta() == rad(32));

    // ID5の青ロボが存在
    // cam0で検出されたID5の青ロボのほうがconfidenceが高いので値の変化なし
    BOOST_CHECK_NO_THROW(r = rb.at(5));
    BOOST_TEST(r.id() == 5);
    BOOST_TEST(r.x() == 50);
    BOOST_TEST(r.y() == 51);
    BOOST_TEST(r.theta() == rad(52));
  }

  {
    ssl_protos::vision::DetectionFrame f;
    f.set_camera_id(0);

    ru.update(f);
  }

  {
    ai::model::robot r;
    const auto rb = ru.value();

    // 検出された青ロボは3台
    BOOST_TEST(rb.size() == 3);

    // cam1で検出されたID1の青ロボが存在
    BOOST_CHECK_NO_THROW(r = rb.at(1));
    BOOST_TEST(r.id() == 1);
    BOOST_TEST(r.x() == 13);
    BOOST_TEST(r.y() == 14);
    BOOST_TEST(r.theta() == rad(15));

    // cam1で検出されたID2の青ロボが存在
    BOOST_CHECK_NO_THROW(r = rb.at(2));
    BOOST_TEST(r.id() == 2);
    BOOST_TEST(r.x() == 20);
    BOOST_TEST(r.y() == 21);
    BOOST_TEST(r.theta() == rad(22));

    // ID5の青ロボが存在
    // cam0から検出されなくなったが, まだcam1に残っているので
    BOOST_CHECK_NO_THROW(r = rb.at(5));
    BOOST_TEST(r.id() == 5);
    BOOST_TEST(r.x() == 50);
    BOOST_TEST(r.y() == 51);
    BOOST_TEST(r.theta() == rad(52));
  }

  {
    ssl_protos::vision::DetectionFrame f;
    f.set_camera_id(1);

    ru.update(f);
  }

  {
    const auto rb = ru.value();

    // そして誰もいなくなった
    BOOST_TEST(rb.size() == 0);
  }
}

BOOST_AUTO_TEST_CASE(transformation, *boost::unit_test::tolerance(0.0000001)) {
  using namespace boost::math::double_constants;

  model::updater::robot<model::teamColor::Blue> ru;
  // 90度回転, x軸方向に10, y軸方向に20平行移動
  ru.transformationMatrix(util::math::makeTransformationMatrix(10.0, 20.0, half_pi));

  {
    ssl_protos::vision::DetectionFrame f;
    f.set_camera_id(0);
    f.set_t_capture(2.0);

    auto rb1 = f.add_robots_blue();
    rb1->set_robot_id(0);
    rb1->set_x(100);
    rb1->set_y(200);
    rb1->set_orientation(0);
    rb1->set_confidence(90.0);

    ru.update(f);
  }

  {
    const auto rb = ru.value();
    ai::model::robot r;
    BOOST_TEST(rb.size() == 1);
    BOOST_CHECK_NO_THROW(r = rb.at(0));
    BOOST_TEST(r.x() == -190.0);
    BOOST_TEST(r.y() == 120.0);
    BOOST_TEST(r.theta() == 3 * half_pi);
  }
}

struct mockFilter1 : public filter::base<model::robot, filter::timing::OnUpdated> {
  // コンストラクタの引数に渡された値
  int arg1;
  int arg2;

  // 最後にupdateの引数に与えられた値
  model::robot v;
  util::TimePointType t;

  mockFilter1(int _a1, int _a2) : arg1(_a1), arg2(_a2) {}

  model::robot update(const model::robot& _value, util::TimePointType _time) override {
    v = _value;
    t = _time;

    // vx, ayに適当な値をセットして返す
    model::robot r{};
    r.vx(_value.x() * 2);
    r.ay(_value.y() * 3);
    return r;
  }
};

BOOST_AUTO_TEST_CASE(on_updated_filter, *boost::unit_test::tolerance(0.0000001)) {
  model::updater::robot<model::teamColor::Blue> ru;

  // ID0にmock_filter1を設定
  const auto fp = ru.setFilter<mockFilter1>(0, 123, 456).lock();

  // tをhigh_resolution_clockの型に変換する関数
  auto durationCast = [](auto t) { return std::chrono::duration_cast<util::DurationType>(t); };

  // set_filterの引数に与えた値がFilterのコンストラクタに正しく渡されているか
  BOOST_TEST(fp->arg1 == 123);
  BOOST_TEST(fp->arg2 == 456);

  {
    ssl_protos::vision::DetectionFrame f;
    f.set_camera_id(0);
    f.set_t_capture(2.0);

    auto rb1 = f.add_robots_blue();
    rb1->set_robot_id(0);
    rb1->set_x(1);
    rb1->set_y(2);
    rb1->set_orientation(rad(3));
    rb1->set_confidence(90.0);

    ru.update(f);
  }

  {
    // updateの引数に値が正しく渡されているか
    BOOST_TEST(fp->v.x() == 1);
    BOOST_TEST(fp->v.y() == 2);
    BOOST_TEST(fp->v.theta() == rad(3));
    BOOST_TEST(fp->t.time_since_epoch().count() == durationCast(2s).count());

    // vx, ayが選択された値の2倍, 3倍になっている
    const auto rb = ru.value();
    ai::model::robot r;
    BOOST_TEST(rb.size() == 1);
    BOOST_CHECK_NO_THROW(r = rb.at(0));
    BOOST_TEST(r.vx() == 2);
    BOOST_TEST(r.ay() == 6);
  }

  {
    ssl_protos::vision::DetectionFrame f;
    f.set_camera_id(1);
    f.set_t_capture(4.0);

    auto rb1 = f.add_robots_blue();
    rb1->set_robot_id(0);
    rb1->set_x(10);
    rb1->set_y(20);
    rb1->set_orientation(rad(30));
    rb1->set_confidence(92.0);

    ru.update(f);
  }

  {
    // updateの引数に値が正しく渡されているか
    BOOST_TEST(fp->v.x() == 10);
    BOOST_TEST(fp->v.y() == 20);
    BOOST_TEST(fp->v.theta() == rad(30));
    BOOST_TEST(fp->t.time_since_epoch().count() == durationCast(4s).count());

    // vx, ayが選択された値の2倍, 3倍になっている
    const auto rb = ru.value();
    ai::model::robot r;
    BOOST_TEST(rb.size() == 1);
    BOOST_CHECK_NO_THROW(r = rb.at(0));
    BOOST_TEST(r.vx() == 20);
    BOOST_TEST(r.ay() == 60);
  }

  {
    ssl_protos::vision::DetectionFrame f;
    f.set_camera_id(0);
    f.set_t_capture(8.0);

    auto rb1 = f.add_robots_blue();
    rb1->set_x(100);
    rb1->set_y(200);
    rb1->set_orientation(rad(300));
    rb1->set_confidence(90.0);

    ru.update(f);
  }

  {
    // 最新でない値が選択された場合は変化しない
    const auto rb = ru.value();
    ai::model::robot r;
    BOOST_TEST(rb.size() == 1);
    BOOST_CHECK_NO_THROW(r = rb.at(0));
    BOOST_TEST(r.vx() == 20);
    BOOST_TEST(r.ay() == 60);
  }
}

struct mockFilter2 : public filter::base<model::robot, filter::timing::Manual> {
  using own_type = filter::base<model::robot, filter::timing::Manual>;

  // コンストラクタの引数に渡された値
  int arg1;
  int arg2;

  mockFilter2(own_type::LastValueFuncType _lf, own_type::WriterFuncType _wf, int _a1, int _a2)
      : base(_lf, _wf), arg1(_a1), arg2(_a2) {}

  auto lv() {
    return lastValue();
  }

  void wv(std::optional<model::robot> _v) {
    write(_v);
  }
};

BOOST_AUTO_TEST_CASE(manualFilter, *boost::unit_test::tolerance(0.0000001)) {
  model::updater::robot<model::teamColor::Blue> ru;

  // ID0にmock_filter2を設定
  const auto fp = ru.setFilter<mockFilter2>(0, 123, 456).lock();

  // set_filterの引数に与えた値がFilterのコンストラクタに正しく渡されているか
  BOOST_TEST(fp->arg1 == 123);
  BOOST_TEST(fp->arg2 == 456);

  {
    ssl_protos::vision::DetectionFrame f;
    f.set_camera_id(0);

    auto rb1 = f.add_robots_blue();
    rb1->set_robot_id(0);
    rb1->set_x(10);
    rb1->set_y(20);
    rb1->set_orientation(rad(30));
    rb1->set_confidence(90.0);

    ru.update(f);
  }

  {
    // updateが呼ばれただけでは値は更新されない
    const auto rb = ru.value();
    BOOST_TEST(rb.size() == 0);

    // last_valueから選択された値が取れる
    const auto lv = fp->lv();
    BOOST_TEST(static_cast<bool>(lv));
    BOOST_TEST(lv->x() == 10);
    BOOST_TEST(lv->y() == 20);
    BOOST_TEST(lv->theta() == rad(30));
  }

  {
    // writeで値が更新される
    const auto r1 = model::robot{40, 50, 60};
    fp->wv(r1);

    const auto rb = ru.value();
    ai::model::robot r2;
    BOOST_TEST(rb.size() == 1);
    BOOST_CHECK_NO_THROW(r2 = rb.at(0));
    BOOST_TEST(r2.x() == r1.x());
    BOOST_TEST(r2.y() == r1.y());
    BOOST_TEST(r2.theta() == r1.theta());
  }

  {
    // nulloptで更新すると消える
    fp->wv(std::nullopt);

    const auto rb = ru.value();
    BOOST_TEST(rb.size() == 0);
  }

  {
    // 候補リストを空にするとlast_valueがnulloptを返す
    ssl_protos::vision::DetectionFrame f;
    f.set_camera_id(0);
    ru.update(f);
    BOOST_TEST(!fp->lv());
  }
}

BOOST_AUTO_TEST_CASE(default_filter, *boost::unit_test::tolerance(0.0000001)) {
  model::updater::robot<model::teamColor::Blue> ru;

  // ID1にmock_filter2を設定
  const auto fp1 = ru.setFilter<mockFilter2>(1, 123, 456);
  // デフォルトのFilterとしてmock_filter1を設定
  ru.setDefaultFilter<mockFilter1>(123, 456);

  {
    ssl_protos::vision::DetectionFrame f;
    f.set_camera_id(0);

    auto rb1 = f.add_robots_blue();
    rb1->set_robot_id(1);
    rb1->set_x(10);
    rb1->set_y(11);
    rb1->set_orientation(rad(12));
    rb1->set_confidence(94.0);

    auto rb3 = f.add_robots_blue();
    rb3->set_robot_id(3);
    rb3->set_x(30);
    rb3->set_y(31);
    rb3->set_orientation(rad(32));
    rb3->set_confidence(95.0);

    auto rb5 = f.add_robots_blue();
    rb5->set_robot_id(5);
    rb5->set_x(50);
    rb5->set_y(51);
    rb5->set_orientation(rad(52));
    rb5->set_confidence(96.0);

    ru.update(f);
  }

  {
    ai::model::robot r;
    const auto rb = ru.value();

    // 検出された青ロボは2台
    BOOST_TEST(rb.size() == 2);

    // ID1の青ロボが存在しない
    // (検出はされているが, manualなFilterが登録されているので)
    BOOST_TEST(!rb.count(1));

    // ID3の青ロボが存在
    // mock_filter1が適用された値になっている
    BOOST_CHECK_NO_THROW(r = rb.at(3));
    BOOST_TEST(r.vx() == 60);
    BOOST_TEST(r.ay() == 93);

    // ID5の青ロボが存在
    // mock_filter1が適用された値になっている
    BOOST_CHECK_NO_THROW(r = rb.at(5));
    BOOST_TEST(r.vx() == 100);
    BOOST_TEST(r.ay() == 153);
  }

  // default_filterを解除
  ru.clearDefaultFilter();

  {
    ssl_protos::vision::DetectionFrame f;
    f.set_camera_id(1);

    auto rb7 = f.add_robots_blue();
    rb7->set_robot_id(7);
    rb7->set_x(70);
    rb7->set_y(71);
    rb7->set_orientation(rad(72));
    rb7->set_confidence(94.0);

    ru.update(f);
  }

  {
    ai::model::robot r;
    const auto rb = ru.value();

    // 検出された青ロボは3台
    BOOST_TEST(rb.size() == 3);

    // ID1の青ロボが存在しない
    BOOST_TEST(!rb.count(1));

    // ID3の青ロボが存在
    BOOST_CHECK_NO_THROW(r = rb.at(3));
    BOOST_TEST(r.vx() == 60);
    BOOST_TEST(r.ay() == 93);

    // ID5の青ロボが存在
    BOOST_CHECK_NO_THROW(r = rb.at(5));
    BOOST_TEST(r.vx() == 100);
    BOOST_TEST(r.ay() == 153);

    // ID7の青ロボが存在
    // filter_initializer_が初期化されたので, mock_filter1は適用されていない
    BOOST_CHECK_NO_THROW(r = rb.at(7));
    BOOST_TEST(r.id() == 7);
    BOOST_TEST(r.x() == 70);
    BOOST_TEST(r.y() == 71);
    BOOST_TEST(r.theta() == rad(72));
  }
}

BOOST_AUTO_TEST_CASE(clear_filter) {
  model::updater::robot<model::teamColor::Blue> ru;

  // clear_filterを呼ぶと登録したFilterが死んでいる
  const auto fp1 = ru.setFilter<mockFilter1>(0, 123, 456);
  BOOST_TEST(!fp1.expired());
  ru.clearFilter(0);
  BOOST_TEST(fp1.expired());

  // manualなFilterも同様
  const auto fp2 = ru.setFilter<mockFilter2>(0, 123, 456);
  BOOST_TEST(!fp2.expired());
  ru.clearFilter(0);
  BOOST_TEST(fp2.expired());

  // on_updatedなFilterが登録されている状態でmanualなFilterを登録するとon_updatedなFilterは死ぬ
  const auto fp3 = ru.setFilter<mockFilter1>(0, 123, 456);
  const auto fp4 = ru.setFilter<mockFilter2>(0, 123, 456);
  BOOST_TEST(fp3.expired());
  BOOST_TEST(!fp4.expired());

  // 逆も同様
  const auto fp5 = ru.setFilter<mockFilter1>(0, 123, 456);
  BOOST_TEST(fp4.expired());
  BOOST_TEST(!fp5.expired());
  ru.clearFilter(0);

  // いくつかのFilterを設定
  const auto fp6 = ru.setFilter<mockFilter1>(0, 123, 456);
  const auto fp7 = ru.setFilter<mockFilter1>(1, 123, 456);
  const auto fp8 = ru.setFilter<mockFilter2>(2, 123, 456);
  const auto fp9 = ru.setFilter<mockFilter2>(3, 123, 456);
  BOOST_TEST(!fp6.expired());
  BOOST_TEST(!fp7.expired());
  BOOST_TEST(!fp8.expired());
  BOOST_TEST(!fp9.expired());
  // clear_all_filters()で全部死ぬ
  ru.clearAllFilters();
  BOOST_TEST(fp6.expired());
  BOOST_TEST(fp7.expired());
  BOOST_TEST(fp8.expired());
  BOOST_TEST(fp9.expired());
}

BOOST_AUTO_TEST_SUITE_END()
