void dmbsy();

int flag[2] = {0,0};
int turn = 0;
int var = 0;

void thread0(){
      begin:
            dmbsy();
            flag[0] = 1;
            if ( turn == 0) goto endloop;
            ide_loop:
                  dmbsy();
                  if( flag[1] != 0) goto ide_loop;
            turn = 0;
            endloop:
                  dmbsy();
                  flag[0] = 2;
                  if(flag[1] == 2) goto begin;
      //CS
      dmbsy();
      var = var + 1;
      //unlock
      dmbsy();
      flag[0] = 0;

}
void thread1(){
      begin:
            dmbsy();
            flag[1] = 1;
            if ( turn != 0) goto endloop;
            ide_loop:
                  dmbsy();
                  if( flag[0] != 0) goto ide_loop;
            turn = 1;
            endloop:
                  dmbsy();
                  flag[1] = 2;
                  if(flag[0] == 2) goto begin;
      //CS
      dmbsy();
      var = var + 1;
      //unlock
      dmbsy();
      flag[1] = 0;

}