這份README主要紀錄過程中遇到哪些bug以及向誰請教。

bug:
1.  忘記把conn_fd加到fd_set裡面，所以read_server一測就當掉了
2.  沒有把conn_fd指定為for迴圈判斷完FD_ISSET之後的i
3.  select裡面放的set放錯
4.  read_server用完忘記清掉conn_fd
5.  write_server用完忘記清掉conn_fd	
6.  write_server進第二次輸入指令時，id跑掉
7.  file_lock沒鎖好
8.  file_lock沒unlock好
9.  file_lock鎖好了，但是對於連到同一個write_server沒辦法鎖
10. read_server的unlock寫爛掉了，導致用read_server戳lock完後壞掉 

Reference:
徐維謙(b07902001)、黃永雯(b07902127)、林楷恩
http://beej-zhtw.netdpi.net/07-advanced-technology/7-2-select
https://blog.csdn.net/u012349696/article/details/50791364

這次作業花了大約二十小時，很多地方從似懂非懂，到現在已經可以知道流程運作，收穫很多！
