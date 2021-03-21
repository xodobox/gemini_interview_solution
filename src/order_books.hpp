namespace gemini_test {

    enum Side : int { BUY = 0, SELL = 1 };

    struct Order
    {
        std::string order_id;
        std::string side;
        std::string instrument;
        size_t      quantity = 0;
        double      price = 0;
        int64_t     seqno;
        friend std::ostream & operator << ( std::ostream & s, const Order & o)
        {
            s << o.order_id << ' ' << o.side << ' ' << o.instrument << ' ' << o.quantity << ' ' << o.price << ' ' << o.seqno;
            return s;
        }
    };

    struct Trade
    {
        std::string instrument;
        std::string order_id_ask;
        std::string order_id_bid;
        size_t      quantity;
        double      price;
        friend std::ostream & operator << ( std::ostream & s, const Trade & o)
        {
            s << "TRADE " << o.instrument << ' ' << o.order_id_ask << ' ' << o.order_id_bid << ' ' << o.quantity << ' ' << o.price;
            return s;
        }
    };

    /// OrderSide defines one side of the order book, orders are sorted by time/price priority, 
    /// as it's cheaper to push/pop at the back() side of vector, and most order book updates happen around the top of the book
    /// high priority orders are stored at the tail of the vector.
    template<typename T, Side side> struct OrderSide
    {

        bool empty() const { return orders_.empty(); }

        auto begin() { return orders_.rbegin(); }
        auto end()   { return orders_.rend(); }
        auto cbegin() const { return orders_.crbegin(); }
        auto cend()   const { return orders_.crend(); }

        /// when matching aggressive orders, remove fully-matched top-of-book order
        auto remove_tob()
        {
            orders_.pop_back();
        }

        static bool compare_order(const T & lhs, const T & rhs)
        {
            if constexpr(side == Side::BUY)
            {
                return rhs.price < lhs.price;
            }
            else
            {
                return lhs.price < rhs.price;
            }
        }
        /// given an order, rank() find the position where the order should be inserted
        size_t rank_order(const T & order) const 
        {
            if(empty())
            {
                return 0;
            }

            auto iter = std::upper_bound(cbegin(), cend(), order, compare_order);
            return iter - cbegin();
        }
        auto rest_order(size_t rank, const T & order)
        {
            orders_.insert((begin() + rank).base(), order);
        }
        auto rest_order(const T & order)
        {
            rest_order(rank_order(order), order);
        }


        std::vector<T> orders_;
        static constexpr Side side_ = side;
    };

    /// OrderBook holds BID/ASK for the matching of one instrument, 
    template<typename T> struct OrderBook
    {
        OrderSide<T, Side::BUY> bids;
        OrderSide<T, Side::SELL> asks;

        template<Side side> auto & opposite_side()
        {
            if constexpr(side == Side::SELL)
            {
                return bids;
            }
            else
            {
                return asks;
            }
        }
        template<Side side> auto & resting_side()
        {
            if constexpr(side == Side::BUY)
            {
                return bids;
            }
            else
            {
                return asks;
            }
        }

        /// new_order returns sequence of trades, if any
        template<Side side> auto new_order(T & ord)
        {
            std::vector<Trade> trades;
            auto & resting  = resting_side<side>();
            auto & opposite = opposite_side<side>();

            /// as there's no auction, the book is never crossed
            /// first check whether the new order crosses the opposite side
            while(ord.quantity > 0 && !opposite.empty())
            {
                auto iter = opposite.begin();
                if(opposite.compare_order(ord, *iter))
                {
                    break;
                }
                if(iter->quantity <= ord.quantity)
                {
                    if constexpr(side == Side::BUY)
                    {
                        trades.push_back({ ord.instrument, iter->order_id, ord.order_id, iter->quantity, iter->price }); 
                    }
                    else
                    {
                        trades.push_back({ ord.instrument, ord.order_id, iter->order_id, iter->quantity, iter->price }); 
                    }
                    ord.quantity -= iter->quantity;
                    opposite.remove_tob();
                }
                else
                {
                    if constexpr(side == Side::BUY)
                    {
                        trades.push_back({ ord.instrument, iter->order_id, ord.order_id, ord.quantity, iter->price }); 
                    }
                    else
                    {
                        trades.push_back({ ord.instrument, ord.order_id, iter->order_id, ord.quantity, iter->price }); 
                    }
                    iter->quantity -= ord.quantity;
                    ord.quantity = 0;
                }
            }
            if(ord.quantity)
            {
                resting.rest_order(ord);
            }
            return trades;
        }

        auto add_order(T ord)
        {
            if(ord.side == "BUY")
            {
                return new_order<Side::BUY>(ord);
            }
            else
            {
                return new_order<Side::SELL>(ord);
            }
        }
    };

    template<typename T> struct MatchingEngine
    {
        std::map<std::string, OrderBook<T>> order_books;
        uint64_t seqno = 0;

        auto new_order(T & ord)
        {
            ord.seqno = seqno++;
            auto iter = order_books.find(ord.instrument);
            if(iter == order_books.end())
            {
                auto[it,_] = order_books.insert({ord.instrument, OrderBook<T>{}});
                iter = it;
            }
            auto trades = iter->second.add_order(ord);
            for(auto & x : trades)
            {
                std::cout << x << std::endl;
            }
        }
    };

} /// namespace gemini_test
