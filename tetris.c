#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#define BGM_SIZE 35

//ゲームの状態
unsigned char state = 0;
//スコア
unsigned char score = 0;

//BGMデータ配列
unsigned char bgm[BGM_SIZE] =
    {0,141, 189, 178, 158, 178, 189, 212, 178, 141, 158, 178, 189, 178, 158, 141, 178, 212, 158, 133, 105, 118, 133, 141, 178, 141, 158, 178, 189, 178, 158, 141, 178, 212, 0};
unsigned char bgm_len[BGM_SIZE] =
    {2,2, 1, 1, 2, 1, 1, 3, 1, 2, 1, 1, 3, 1, 2, 2, 2, 4, 2, 1, 2, 1, 1, 3, 1, 2, 1, 1, 3, 1, 2, 2, 2, 3, 4};

volatile unsigned char m = 0;
volatile unsigned char note_length = 2;
unsigned char cnt_t0 = 100;

volatile unsigned char led[8] = {0x57, 0x50, 0x17, 0x32, 0x06, 0xf4, 0x27, 0xd4};
;
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
    switch (state)
    {
    case 0: //タイトル画面
        if (sw == 0x03)
        {
            state = 1;
            score = 0;
        }
        break;
    case 1: //ゲーム画面
        switch (sw)
        {
        case 1:
            /* code */
            break;
        case 2:
            /* code */
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
}

//BGM再生
ISR(TIMER0_COMPA_vect)
{
    if (state == 1)
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
}

ISR(TIMER1_COMPA_vect)
{
    update_led();
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