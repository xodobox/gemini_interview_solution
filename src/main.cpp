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


int main() 
{
  std::cerr << "====== Match Engine =====" << std::endl;
  std::cerr << "Enter 'exit' to quit" << std::endl;

  int64_t seqno = 0;

  MatchingEngine<Order> matching_engine;
  std::string line;

  auto dump_all_resting_orders = [&matching_engine]()
  {
      /// printing all resting orders is not performance critical, happens only when input queue is idle
      std::vector<Order> asks, bids;
      for(auto iter = matching_engine.order_books.begin(); iter != matching_engine.order_books.end(); ++iter)
      {
          asks.insert(asks.end(), iter->second.asks.begin(), iter->second.asks.end());
          bids.insert(bids.end(), iter->second.bids.begin(), iter->second.bids.end());
      }
      auto compare = [](const auto & lhs, const auto & rhs)
      {
          return lhs.seqno < rhs.seqno;
      };
      std::sort(asks.begin(), asks.end(), compare);
      std::sort(bids.begin(), bids.end(), compare);
      std::cout << std::endl;
      for(auto & x : asks) std::cout << x << std::endl;
      for(auto & x : bids) std::cout << x << std::endl;
      std::cout << std::endl;
  };

#if 1
  bool dump_orders_pending = false;
  while(getline(std::cin, line) && line != "exit")
  {
      std::istringstream stream{line};
      Order order;
      stream >> order.order_id >> order.side >> order.instrument >> order.quantity >> order.price;

      if(order.quantity > 0)
      {
          matching_engine.new_order(order);
          dump_orders_pending = true; /// order book changed
      }
      if(dump_orders_pending && !std::cin.rdbuf()->in_avail())
      {
          dump_all_resting_orders();
          dump_orders_pending = false;
      }
  }

#else

  while (getline(std::cin, line) && line != "exit")
  {
      std::istringstream stream{line};
      Order order;
      stream >> order.order_id >> order.side >> order.instrument >> order.quantity >> order.price;

      if(order.quantity > 0)
      {
        std::cout << order << std::endl;
          matching_engine.new_order(order);
      }
  }
#endif
  return 0;
}
