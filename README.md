Chris Huang

[ How to test ] : run ./test.sh
[ How to run ] : run ./run.sh

[ How to approach the problem ]
For continuous matching, build an OrderBook for each instrument
each OrderBook mantains two OrderSide(Bid/Ask) for passive orders (i.e. not fully consumed upon entry into the book)
for each newly arrive order, match the opposite side until fully consumed, or if only partially consumed then add it to same side

[ How much time spent ]
C++ coding : ~5 hrs
getting familiar with Docker / Catch2 : ~ 2hrs
