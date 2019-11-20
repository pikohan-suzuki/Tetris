#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <unistd.h>

#define BGM_SIZE 35

void add_mino();        //テトリミノの追加
void rotate();          //テトリミノの回転
void move_to_right_left();
void delete_row();      //行の削除
void change_to_block();

int check_add();        //追加チェック
int check_down();       //下方チェック
int check_left();       //左チェック
int check_right();      //右チェック
int check_rotate();     //回転チェック
void check_row();      //消去できる行があるかのチェック

//ゲームの状態
unsigned char state = 0;
//スコア
unsigned char score = 0;
//現在操作しているミノ
unsigned char now_mino;
unsigned char x,y;
unsigned char dire=0;

unsigned int wait =0;

//BGMデータ配列
unsigned char bgm[BGM_SIZE] =
    {0,141, 189, 178, 158, 178, 189, 212, 178, 141, 158, 178, 189, 178, 158, 141, 178, 212, 158, 133, 105, 118, 133, 141, 178, 141, 158, 178, 189, 178, 158, 141, 178, 212, 0};
unsigned char bgm_len[BGM_SIZE] =
    {2,2, 1, 1, 2, 1, 1, 3, 1, 2, 1, 1, 3, 1, 2, 2, 2, 4, 2, 1, 2, 1, 1, 3, 1, 2, 1, 1, 3, 1, 2, 2, 2, 3, 4};

volatile unsigned char m = 0;
volatile unsigned char note_length = 2;
unsigned char cnt_t0 = 100;
unsigned char fall_cnt_max =500;
unsigned char fall_cnt =0;



volatile unsigned char led[8] = {0x57, 0x50, 0x17, 0x32, 0x06, 0xf4, 0x27, 0xd4};
unsigned char pat[12][8] = {
    {7, 5, 5, 5, 7, 0, 0, 0}, // 0
    {1, 1, 1, 1, 1, 0, 0, 0}, // 1
    {7, 1, 7, 4, 7, 0, 0, 0}, // 2
    {7, 1, 7, 1, 7, 0, 0, 0}, // 3
    {5, 5, 7, 1, 1, 0, 0, 0}, // 4
    {7, 4, 7, 1, 7, 0, 0, 0}, // 5
    {7, 4, 7, 5, 7, 0, 0, 0}, // 6
    {7, 1, 1, 1, 1, 0, 0, 0}, // 7
    {7, 5, 7, 5, 7, 0, 0, 0}, // 8
    {7, 5, 7, 1, 7, 0, 0, 0}, // 9
    {0, 0, 0, 0, 0, 0, 0, 1}, // .
    {0, 0, 0, 0, 0, 0, 7, 0}, // _
};

unsigned char block[8]={0,0,0,0,0,0,0,0};

unsigned char mino[7][8]={
    {7,0,0,0,0,0,0,0},
    {3,3,0,0,0,0,0,0},
    {2,7,0,0,0,0,0,0},
    {4,7,0,0,0,0,0,0},
    {1,7,0,0,0,0,0,0},
    {3,6,0,0,0,0,0,0},
    {6,3,0,0,0,0,0,0}
};

unsigned char mino_x[7]={3,2,3,3,3,3,3};
unsigned char mino_y[7]={1,2,2,2,2,2,2};

unsigned char title[8] = {0x57, 0x50, 0x17, 0x32, 0x06, 0xf4, 0x27, 0xd4};

//led表示
void update_led()
{
    static unsigned char sc = 0xFE;
    static unsigned char scan = 0;
    PORTB = 0;
    sc = (sc << 1) | (sc >> 7);
    PORTD = (PORTD & 0x0F) | (sc & 0xF0);
    PORTC = (PORTC & 0xF0) | (sc & 0x0F);
    scan = (scan + 1) & 7;
    PORTB = led[scan];
}

ISR(PCINT1_vect)
{
    int sw = (~PINC >> 4) & 3;
    if(wait>50){
    switch (state)
    {
    case 0: //タイトル画面
        if (sw == 0x03)
        {
            state = 1;
            score = 0;
            for(int i=0;i<8;i++)
                led[i] = 0x00;
            add_mino();
        }
        break;
    case 1: //ゲーム画面
        switch (sw)
        {
        case 1:
            if(check_left()==1){
                x++;
                move_to_right_left();        
            }
            
            break;
        case 2:
            if(check_right() == 1){
                x--;
                move_to_right_left();
            }
            break;
        case 3:
            /* code */
            break;
        default:
            break;
        }
        break;
    case 2: //スコア画面
        if (sw == 0x03)
        {
            state = 0;
        }
        break;
    default:
        break;
    }
    wait=0;
    }
}

//BGM再生
ISR(TIMER0_COMPA_vect)
{
    //if (state == 1)
    if(0)
    {
        if (cnt_t0 == 0)
        {
            note_length--;
            if (note_length == 0)
            {
                if (m + 1 == BGM_SIZE)
                {
                    m = 0;
                }
                else
                {
                    m++;
                }
                OCR2A = bgm[m];
                note_length = bgm_len[m];
            }
            cnt_t0 = 100;
        }
        else
        {
            cnt_t0--;
        }
    }else{
        OCR2A=0;
    }
    wait++;
}


ISR(TIMER1_COMPA_vect)
{
    update_led();
    if(state==1){
        if(fall_cnt==fall_cnt_max){
            //1つ下へ
            y++;
            if(check_down()==1){
                
                for(int i =0;i<8;i++){
                    if(i>=y && i<y+mino_y[now_mino]){
                        led[i] = block[i] | (mino[now_mino][i-y] << x);
                    }else{
                        led[i] = block[i];
                        
                    }
                }
            }else{
                change_to_block();
                check_row();
                add_mino();
            }
            fall_cnt=0;
        }else{
            fall_cnt++;
        }
    }
}

void add_mino(){
    int rnd = rand()%7;
    now_mino = rnd;
    x=0;
    y=0;
    for(int i=0;i<8;i++){
        unsigned char result[8];
        led[i] = block[i] | mino[now_mino][i];
    }
    
}

void move_to_right_left(){
    for(int i =0;i<8;i++){
        if(i>=y && i<y+mino_y[now_mino]){
            // led[i] = block[i] | (mino[now_mino][i-y] << x);
            led[i] = mino[now_mino][i-y] << x;
        }else{
            led[i] = block[i];
                        
        }
    }
}

void change_to_block(){
        for(int i=y;i<y+mino_y[now_mino];i++){
            block[i] = block[i] | mino[now_mino][i-y] << x;
        }
}

void delete_row(int row){
    block[row] = 0x00;
    for(int i = row;i>=1;i--){
        block[i]=block[i-1];
    }
    block[0] =0x00;
}

int check_down(){
    int flg =1;
    if(y==7)
        return 0;
    for(int i =0;i<3;i++){
        if(y+mino_y[now_mino] > 7){
            flg=0;
            break;
        // }else if(led[y+mino_y[now_mino]][x+i]==1){
        }else if((block[y+mino_y[now_mino]] >> (x+1)) & 0x01 == 1){
            flg = 0;
            break;
        }
    }
    return flg;
}

int check_right(){
    int flg=1;
    if(x==0)
        return 0;
    for(int i=y;i<y+mino_y[now_mino];i++){
        if((block[i]>>(x-1)) & 0x01 == 1){
            flg=0;
            break;
        }
    }
    return flg;
}

int check_left(){
    int flg=1;
    if(x+mino_x[now_mino] == 8)
        return 0;
    for(int i=y;i<y+mino_y[now_mino];i++){
        if((block[i]>>(x+mino_x[now_mino]-1)) & 0x01 == 1){
            flg=0;
            break;
        }
    }
    return flg;
}

void check_row(){
    for(int i=7;i>=0;i--){
        if(block[i]==0xff){
            delete_row(i);
        }
    }
}

int main(void)
{
    DDRD = 0xFE;
    DDRC = 0x0F;
    DDRB = 0xFF;

    PORTD = 0xf0;
    PORTC = 0x30;
    PORTB = 0x00;

    //ピン変化割り込み設定
    PCMSK1 = 0x30;
    PCICR = _BV(PCIE1);

    //ブザーコンペアマッチ出力用タイマー
    TCCR2B = 0x04;
    TCCR2A = 0x12;
    OCR2A = 141;
    //BGM再生用タイマー
    TCCR0B = 0x03;
    TCCR0A = 0x02;
    TIMSK0 = 0x02;
    OCR0A = 249;

    TCCR1A = 0x02;
    TCCR1B = 0x03;
    OCR1A = 249;
    TIMSK1 |= _BV(OCIE0A);

    sei();
    for (;;)
    {
        wdt_reset();
    }
    return 0;
}