#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <map>
#include <vector>
#include <array>
#include <algorithm>
#include "order_books.hpp"
using namespace gemini_test;

#define CATCH_CONFIG_MAIN
#include "catch_amalgamated.hpp"
/*
int main() 
{
  std::cerr << "====== Match Engine =====" << std::endl;
  std::cerr << "Enter 'exit' to quit" << std::endl;

  int64_t seqno = 0;

  MatchingEngine<Order> matching_engine;

  std::string line;
  while (getline(std::cin, line) && line != "exit")
  {
      std::istringstream stream{line};
      Order order;
      stream >> order.order_id >> order.side >> order.instrument >> order.quantity >> order.price;
      std::cout << order << std::endl;
      matching_engine.new_order(order);
  }
  return 0;
}
*/

TEST_CASE("OrderSide sorted by price priority", "[OrderSide]")
{
    OrderSide< Order, Side::BUY > bids;
    OrderSide< Order, Side::SELL> asks;

    Order order{ "xxx", "", "BTCUSD", 1, 0., 0 };
    int64_t seqno = 0;

    constexpr int base_price = 10000;
    constexpr int total_orders = 100;
    constexpr int price_slots  = 19;

    for(int i = 0; i < total_orders; ++i)
    {
        order.price = base_price + (i % price_slots); 
        order.side = "BUY";
        order.seqno = seqno++;
        bids.rest_order(bids.rank_order(order), order);
        order.side = "SELL";
        asks.rest_order(asks.rank_order(order), order);
        order.seqno = seqno++;
    }

    /// verify that all levels are ordered by price
    auto check_side_prices = [](const auto & side)
    {
        if(side.empty()) return true;
        for(auto iter = side.cbegin() + 1; iter != side.cend(); ++iter)
        {
            if(side.compare_order(*iter, *(iter-1)))
                return false;
        }
        return true;
    };

    /// verify that for orders of same price, they are ordered by seqno (timestamp)
    auto check_side_seqno = [](const auto & side)
    {
        if(side.empty()) return true;

        for(auto iter = side.cbegin(); iter != side.cend();)
        {
            auto rank = side.rank_order(*iter);
            auto base_rank = size_t(iter - side.cbegin());

            auto s = iter;
            auto e = iter + (rank - base_rank);

            for(auto i = s; i != e-1; ++i)
            {
                if(i->seqno >= (i+1)->seqno) return false;
            }
            iter = e;
        }
        return true;
    };

    REQUIRE(check_side_prices(bids));
    REQUIRE(check_side_prices(asks));

    REQUIRE(check_side_seqno(bids));
    REQUIRE(check_side_seqno(asks));
}

TEST_CASE("OrderBook: Aggressive order; partial / full match","[OrderBook]")
{
    auto check_order_qty = [](const auto & side, auto & order_id, auto qty)
    {
        for(auto iter = side.cbegin(); iter != side.cend(); ++iter)
        {
            if(iter->order_id == order_id && iter->quantity == qty)
            {
                return true;
            }
        }
        return qty == 0;
    };
    auto check_trade = [](auto & trades, auto bid_id, auto ask_id, auto prx, auto qty)
    {
        for(auto iter = trades.begin(); iter != trades.end(); ++iter)
        {
            if(bid_id == iter->order_id_bid && ask_id == iter->order_id_ask && prx == iter->price && qty == iter->quantity)
            {
                return true;
            }
        }
        return false;
    };
    SECTION( "Aggressive order fully matched" )
    {
        OrderBook<Order> book;
        int64_t seqno = 0;

        book.add_order({"1", "BUY", "BTCUSD", 5, 10000, seqno++});
        book.add_order({"2", "BUY", "BTCUSD", 1, 10000, seqno++});
        book.add_order({"3", "BUY", "BTCUSD", 1, 10001, seqno++});
        book.add_order({"4", "BUY", "BTCUSD", 1, 10002, seqno++});

        auto trades = book.add_order({"5", "SELL","BTCUSD", 4, 9999, seqno++});

        REQUIRE(trades.size() == 3);
        REQUIRE(check_trade(trades, "1", "5", 10000, 2));
        REQUIRE(check_trade(trades, "3", "5", 10001, 1));
        REQUIRE(check_trade(trades, "4", "5", 10002, 1));

        REQUIRE(check_order_qty(book.bids, "1", 3));
        REQUIRE(check_order_qty(book.bids, "2", 1));
        REQUIRE(check_order_qty(book.bids, "3", 0));
        REQUIRE(check_order_qty(book.bids, "4", 0));
        REQUIRE(check_order_qty(book.asks, "5", 0));
    }

    SECTION( "Aggressive order partially matched" )
    {
        OrderBook<Order> book;
        int64_t seqno = 0;

        book.add_order({"1", "BUY", "BTCUSD", 5,  9000, seqno++});
        book.add_order({"2", "BUY", "BTCUSD", 1, 10000, seqno++});
        book.add_order({"3", "BUY", "BTCUSD", 1, 10001, seqno++});
        book.add_order({"4", "BUY", "BTCUSD", 1, 10002, seqno++});

        auto trades = book.add_order({"5", "SELL","BTCUSD", 4, 9999, seqno++});

        REQUIRE(trades.size() == 3);
        REQUIRE(check_trade(trades, "2", "5", 10000, 1));
        REQUIRE(check_trade(trades, "3", "5", 10001, 1));
        REQUIRE(check_trade(trades, "4", "5", 10002, 1));

        REQUIRE(check_order_qty(book.bids, "1", 5));
        REQUIRE(check_order_qty(book.bids, "2", 0));
        REQUIRE(check_order_qty(book.bids, "3", 0));
        REQUIRE(check_order_qty(book.bids, "4", 0));
        REQUIRE(check_order_qty(book.asks, "5", 1));
    }
}
