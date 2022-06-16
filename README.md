# 1091528_proj_3
Tomasulo實作
請先下載input.txt，再進行程式。

一開始先將程式讀進一個queue中存取

開始進行程式時，
先判斷程式是加法還是乘法

1.從queue中issue一個instruction
檢查對的RS的空間是否還有
判斷RF中的值版本正不正確，若不正確則從RAT拿

2.進行execute
拿到值後，就開始計算instruction的結果
3.write back

計算完後，將值寫入RS、RAT、RF中
BUFFER會顯示目前哪一個instruction寫入

程式結束條件:
queue沒有任何instruction，需要寫回的程式也都寫回了，沒有任何還需要執行的程式。
