#define BOOST_TEST_DYN_LINK

#include <stdexcept>
#include <boost/math/constants/constants.hpp>
#include <boost/test/unit_test.hpp>

#include "ai/model/updater/world.hpp"
#include "ssl-protos/vision/wrapper.pb.h"

BOOST_AUTO_TEST_SUITE(updater_world)

BOOST_AUTO_TEST_CASE(detection, *boost::unit_test::tolerance(0.0000001)) {
  namespace bmc = boost::math::double_constants;

  ai::model::updater::world wu{};

  {
    ssl_protos::vision::WrapperPacket p;

    auto md = p.mutable_detection();
    md->set_camera_id(0);

    auto b = md->add_balls();
    b->set_x(1);
    b->set_y(2);
    b->set_z(3);
    b->set_confidence(93.0);

    auto rb1 = md->add_robots_blue();
    rb1->set_robot_id(1);
    rb1->set_x(10);
    rb1->set_y(11);
    rb1->set_orientation(bmc::sixth_pi);
    rb1->set_confidence(94.0);

    auto rb3 = md->add_robots_blue();
    rb3->set_robot_id(3);
    rb3->set_x(30);
    rb3->set_y(31);
    rb3->set_orientation(bmc::third_pi);
    rb3->set_confidence(95.0);

    auto ry5 = md->add_robots_yellow();
    ry5->set_robot_id(5);
    ry5->set_x(500);
    ry5->set_y(501);
    ry5->set_orientation(bmc::half_pi);
    ry5->set_confidence(96.0);

    auto ry7 = md->add_robots_yellow();
    ry7->set_robot_id(7);
    ry7->set_x(700);
    ry7->set_y(701);
    ry7->set_orientation(bmc::two_thirds_pi);
    ry7->set_confidence(97.0);

    wu.update(p);
  }

  {
    const auto w = wu.value();

    // cam0で検出されたボール(1, 2, 3)
    const auto b = w.ball();
    BOOST_TEST(b.x() == 1);
    BOOST_TEST(b.y() == 2);

    ai::model::robot r;

    // 検出された青ロボは2台
    BOOST_TEST(w.robotsBlue().size() == 2);

    // ID1の青ロボが存在 (ID1の要素を参照してもstd::out_of_range例外が飛ばない)
    BOOST_CHECK_NO_THROW(r = w.robotsBlue().at(1));
    BOOST_TEST(r.id() == 1);
    BOOST_TEST(r.x() == 10);
    BOOST_TEST(r.y() == 11);
    BOOST_TEST(r.theta() == bmc::sixth_pi);

    // ID3の青ロボが存在
    BOOST_CHECK_NO_THROW(r = w.robotsBlue().at(3));
    BOOST_TEST(r.id() == 3);
    BOOST_TEST(r.x() == 30);
    BOOST_TEST(r.y() == 31);
    BOOST_TEST(r.theta() == bmc::third_pi);

    // 検出された黃ロボは2台
    BOOST_TEST(w.robotsYellow().size() == 2);

    // ID5の黃ロボが存在
    BOOST_CHECK_NO_THROW(r = w.robotsYellow().at(5));
    BOOST_TEST(r.id() == 5);
    BOOST_TEST(r.x() == 500);
    BOOST_TEST(r.y() == 501);
    BOOST_TEST(r.theta() == bmc::half_pi);

    // ID7の黃ロボが存在
    BOOST_CHECK_NO_THROW(r = w.robotsYellow().at(7));
    BOOST_TEST(r.id() == 7);
    BOOST_TEST(r.x() == 700);
    BOOST_TEST(r.y() == 701);
    BOOST_TEST(r.theta() == bmc::two_thirds_pi);

    // fieldは変更されていない (初期値のまま)
    const auto f1 = w.field();
    const auto f2 = ai::model::field{};
    BOOST_TEST(f1.length() == f2.length());
    BOOST_TEST(f1.width() == f2.width());
    BOOST_TEST(f1.centerRadius() == f2.centerRadius());
    BOOST_TEST(f1.goalWidth() == f2.goalWidth());
    BOOST_TEST(f1.penaltyLength() == f2.penaltyLength());
    BOOST_TEST(f1.penaltyWidth() == f2.penaltyWidth());
  }
}

BOOST_AUTO_TEST_CASE(transformation, *boost::unit_test::tolerance(0.0000001)) {
  namespace bmc = boost::math::double_constants;

  ai::model::updater::world wu{};
  // 90度回転, x軸方向に10, y軸方向に20平行移動
  wu.transformationMatrix(10, 20, bmc::half_pi);

  {
    ssl_protos::vision::WrapperPacket p;

    auto md = p.mutable_detection();
    md->set_camera_id(0);

    auto b = md->add_balls();
    b->set_x(1);
    b->set_y(2);
    b->set_z(3);
    b->set_confidence(93.0);

    auto rb1 = md->add_robots_blue();
    rb1->set_robot_id(1);
    rb1->set_x(10);
    rb1->set_y(11);
    rb1->set_orientation(bmc::sixth_pi);
    rb1->set_confidence(94.0);

    auto ry5 = md->add_robots_yellow();
    ry5->set_robot_id(5);
    ry5->set_x(500);
    ry5->set_y(501);
    ry5->set_orientation(bmc::pi);
    ry5->set_confidence(96.0);

    wu.update(p);
  }

  {
    const auto w = wu.value();

    // cam0で検出されたボール(1, 2, 3)
    const auto b = w.ball();
    BOOST_TEST(b.x() == 8.0);
    BOOST_TEST(b.y() == 21.0);

    ai::model::robot r;

    // 検出された青ロボは1台
    BOOST_TEST(w.robotsBlue().size() == 1);

    // ID1の青ロボが存在 (ID1の要素を参照してもstd::out_of_range例外が飛ばない)
    BOOST_CHECK_NO_THROW(r = w.robotsBlue().at(1));
    BOOST_TEST(r.id() == 1);
    BOOST_TEST(r.x() == -1.0);
    BOOST_TEST(r.y() == 30.0);
    BOOST_TEST(r.theta() == bmc::pi + bmc::two_thirds_pi);

    // 検出された黃ロボは1台
    BOOST_TEST(w.robotsYellow().size() == 1);

    // ID5の黃ロボが存在
    BOOST_CHECK_NO_THROW(r = w.robotsYellow().at(5));
    BOOST_TEST(r.id() == 5);
    BOOST_TEST(r.x() == -491.0);
    BOOST_TEST(r.y() == 520.0);
    BOOST_TEST(r.theta() == bmc::half_pi);
  }
}

BOOST_AUTO_TEST_CASE(geometry) {
  ai::model::updater::world wu{};

  {
    ssl_protos::vision::WrapperPacket p;

    auto mf = p.mutable_geometry()->mutable_field();
    mf->set_field_length(9000);
    mf->set_field_width(6000);
    mf->set_goal_width(1000);

    auto cca = mf->add_field_arcs();
    cca->set_name("CenterCircle");
    cca->set_radius(200);

    wu.update(p);
  }

  {
    const auto w = wu.value();

    const auto f = w.field();
    BOOST_TEST(f.length() == 9000);
    BOOST_TEST(f.width() == 6000);
    BOOST_TEST(f.goalWidth() == 1000);
    BOOST_TEST(f.centerRadius() == 200);

    // ボールやロボットは検出されていない
    const auto b = w.ball();
    BOOST_TEST(b.x() == 0);
    BOOST_TEST(b.y() == 0);
    BOOST_TEST(w.robotsBlue().size() == 0);
    BOOST_TEST(w.robotsYellow().size() == 0);
  }
}

BOOST_AUTO_TEST_CASE(disabling_camera) {
  namespace bmc = boost::math::double_constants;

  ai::model::updater::world wu{};

  BOOST_TEST(wu.isCameraEnabled(0));
  BOOST_TEST(wu.isCameraEnabled(1));
  wu.disableCamera(0);
  wu.disableCamera(1);
  BOOST_TEST(!wu.isCameraEnabled(0));
  BOOST_TEST(!wu.isCameraEnabled(1));

  wu.enableCamera(1);
  BOOST_TEST(wu.isCameraEnabled(1));

  {
    ssl_protos::vision::WrapperPacket p;

    auto md = p.mutable_detection();
    md->set_camera_id(0);

    auto b = md->add_balls();
    b->set_x(1);
    b->set_y(2);
    b->set_confidence(93.0);

    auto rb1 = md->add_robots_blue();
    rb1->set_robot_id(1);
    rb1->set_x(10);
    rb1->set_y(11);
    rb1->set_orientation(bmc::sixth_pi);
    rb1->set_confidence(94.0);

    auto ry5 = md->add_robots_yellow();
    ry5->set_robot_id(5);
    ry5->set_x(500);
    ry5->set_y(501);
    ry5->set_orientation(bmc::half_pi);
    ry5->set_confidence(96.0);

    auto mf = p.mutable_geometry()->mutable_field();
    mf->set_field_length(90000);
    mf->set_field_width(60000);
    mf->set_goal_width(10000);

    wu.update(p);
  }

  {
    const auto w = wu.value();

    // 無効化されたカメラからは更新されない
    const auto b = w.ball();
    BOOST_TEST(b.x() == decltype(b){}.x());
    BOOST_TEST(b.y() == decltype(b){}.y());

    BOOST_TEST(w.robotsBlue().size() == 0);
    BOOST_TEST(w.robotsYellow().size() == 0);

    const auto f = w.field();
    BOOST_TEST(f.length() == decltype(f){}.length());
    BOOST_TEST(f.width() == decltype(f){}.width());
    BOOST_TEST(f.goalWidth() == decltype(f){}.goalWidth());
  }

  {
    ssl_protos::vision::WrapperPacket p;

    auto md = p.mutable_detection();
    md->set_camera_id(1);

    auto b = md->add_balls();
    b->set_x(1);
    b->set_y(2);
    b->set_confidence(93.0);

    auto rb1 = md->add_robots_blue();
    rb1->set_robot_id(1);
    rb1->set_x(10);
    rb1->set_y(11);
    rb1->set_orientation(bmc::sixth_pi);
    rb1->set_confidence(94.0);

    auto ry5 = md->add_robots_yellow();
    ry5->set_robot_id(5);
    ry5->set_x(500);
    ry5->set_y(501);
    ry5->set_orientation(bmc::half_pi);
    ry5->set_confidence(96.0);

    auto mf = p.mutable_geometry()->mutable_field();
    mf->set_field_length(90000);
    mf->set_field_width(60000);
    mf->set_goal_width(10000);

    wu.update(p);
  }

  {
    const auto w = wu.value();

    const auto b = w.ball();
    BOOST_TEST(b.x() == 1);
    BOOST_TEST(b.y() == 2);

    BOOST_TEST(w.robotsBlue().size() == 1);
    BOOST_TEST(w.robotsBlue().count(1) == 1);
    BOOST_TEST(w.robotsYellow().size() == 1);
    BOOST_TEST(w.robotsYellow().count(5) == 1);

    const auto f = w.field();
    BOOST_TEST(f.length() == 90000);
    BOOST_TEST(f.width() == 60000);
    BOOST_TEST(f.goalWidth() == 10000);
  }
}

BOOST_AUTO_TEST_SUITE_END()
