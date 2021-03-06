#define BOOST_TEST_DYN_LINK

#include <vector>
#include <boost/test/unit_test.hpp>

#include "ai/model/ball.hpp"
#include "ai/filter/observer/ball.hpp"

using namespace std::chrono_literals;

namespace filter = ai::filter;
namespace model  = ai::model;

BOOST_AUTO_TEST_SUITE(state_observer_ball)

BOOST_AUTO_TEST_CASE(rest, *boost::unit_test::tolerance(10.0)) {
  // テストするボールの位置
  const auto ball_positions = std::vector<model::ball>{// コーナー
                                                       {4500, 3000},
                                                       {4500, -3000},
                                                       {-4500, -3000},
                                                       {-4500, 3000},
                                                       // 中心
                                                       {0, 0}};

  // 初期時刻
  auto t = ai::util::TimePointType{};

  filter::observer::ball obs{{}, t};

  for (auto&& ball : ball_positions) {
    model::ball b{};

    // 状態オブザーバを16ms * 500回更新する
    for (auto i = 0u; i < 1000; ++i) {
      t += 16ms;
      b = obs.update(ball, t);
    }

    // それっぽい値が返ってくるか
    BOOST_TEST(b.x() == ball.x());
    BOOST_TEST(b.y() == ball.y());
  }
}

BOOST_AUTO_TEST_SUITE_END()
