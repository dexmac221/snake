/**
 * @file main.c
 * @author dexmac (dexmac@libero.it)
 * @brief 
 * @version 0.1
 * @date 2022-11-19
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <peekpoke.h>
#include <joystick.h>
#include <conio.h>
#include "charmap.h"

#ifdef __C64__
#define JOY_DRIVER  (c64_stdjoy_joy)
#endif

#ifdef __C128__
#include <c128.h>
#define JOY_DRIVER  (c128_stdjoy_joy)
#endif

#ifdef __PLUS4__
#define JOY_DRIVER  (plus4_stdjoy_joy)
#endif

#ifdef __C16__
#define JOY_DRIVER  (c16_stdjoy_joy)
#endif

#ifdef __VIC20__
#define JOY_DRIVER  (vic20_stdjoy_joy)
#endif

#define xsize 40
#define ysize 25

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define HW "6502"

#ifdef __C64__
#undef HW
#define HW "commodore 64"
#define CREG 53272
#endif // __C64__

#ifdef __C128__
#undef HW
#define HW "commodore 128"
#define CREG 2604
#define CHARDEFS_PTR 0x1000/1024
#endif // __C64__

#ifdef __PLUS4__
#undef HW
#define HW "commodore plus4"
#define CREG 65299
#endif // __PLUS4__

#ifdef __C16__
#undef HW
#define HW "commodore c16"
#define CREG 65299
#endif // __PLUS4__

#ifdef __VIC20__
#undef HW
#define HW "commodore vic20"
#define CREG 2604
#endif // __VIC20__

#ifdef __CBM510__
#undef HW
#define HW "commodore CBM510"
#define CREG 2604
#endif // __CBM510__


#define UP 0
#define DOWN 2
#define LEFT 1
#define RIGHT 3
#define MAX_SNAKE 16
#define MAX_BUSH 16

#if defined(__C64__) || defined(__C128__)
#define COLOR_ADDRESS 55296
#define CHAR_ADDRESS 1024
#define LUMINANCE 0
#endif

#if defined(__PLUS4__) || defined(__C16__)
#define COLOR_ADDRESS 2048
#define CHAR_ADDRESS 3072
#define LUMINANCE 5*16
#endif

#ifdef __VIC20__
#define COLOR_ADDRESS 2048
#define CHAR_ADDRESS 3072
#define LUMINANCE 5*16
#endif

#define uchar unsigned char

uchar bushx[MAX_BUSH];
uchar bushy[MAX_BUSH];

uchar snakex[MAX_SNAKE];
uchar snakey[MAX_SNAKE];
uchar snaked[MAX_SNAKE];
 
int snake_size=0;
int xlast=1;
int ylast=1;
int last_direction=0;
int ax=0;
int ay=0;
int direct=1;
int points=0;
int lives=5;
int ingame=0;
int pause=0;
int record=0;

/**
 * @brief Write text onto the screen on x,y
 * 
 * @param string content
 * @param x 
 * @param y 
 * @param color 
 */
void write_string(char *string,int x,int y,int color){
	int len=strlen(string);
	int i=0;
	int counter=0;
	for(i=x;i<(len+x);i++){
		int cv=(int)string[counter];
		
		if(cv>=32 && cv<=63)
			POKE(CHAR_ADDRESS+(y*xsize+i),cv);
		else
			POKE(CHAR_ADDRESS+(y*xsize+i),(cv-64));
		
		POKE(COLOR_ADDRESS+(y*xsize+i),color+LUMINANCE);
		
		counter++;
	}
}

/**
 * @brief Write infos on screen
 * 
 * @param go 
 */
void write_infos(int go){
	char string[40];
	string[0]='\0';
	
	sprintf(string,"points:%d     snakes:%d     winner :%d",points,lives,record);
	
	write_string(string,1,0,2);
	
	if (go==1)
		sprintf(string,"dexmac snake!!! press p to pause game!");
	else
		sprintf(string,"dexmac snake!!! press s to play!");
	
	write_string(string,5,24,2);
}

/**
 * @brief Fill forest with bushes
 * 
 */
void create_forest(){
	int i=0;
	for(i=0;i<MAX_BUSH;i++){
		bushx[i]=rand()%(xsize-2)+1;
		bushy[i]=rand()%(ysize-2)+1;
	}
}

/**
 * @brief Draw forest
 * 
 */
void draw_forest(){
	int i=0;
	for(i=0;i<MAX_BUSH;i++){
		int x=bushx[i];
		int y=bushy[i];
		POKE(CHAR_ADDRESS+(y*xsize+x),35);
		POKE(COLOR_ADDRESS+(y*xsize+x),5+LUMINANCE);
	}
}

/**
 * @brief Create a snake object of size block number
 * 
 * @param size 
 */
void create_snake(int size){
	int i=0;
	
	snake_size=size;
	if(size>8)
		return;
	
	snakex[0]=2;
	snakey[0]=10;
	snaked[0]=UP;
	
	for(i=1;i<snake_size;i++){
		snakex[i]=2;
		snakey[i]=10+i;
		snaked[i]=UP;
	}
}

/**
 * @brief Move snake on the screen
 * 
 */
void move_snake(){
	int i=0;
	
	for(i=0;i<snake_size;i++){
		switch(snaked[i]){
			case UP:
				if(snakey[i]==1)
					snakey[i]=ysize-2;
				else
					snakey[i]--;
			break;
			
			case DOWN:
				if(snakey[i]==(ysize-2))
					snakey[i]=1;
				else
					snakey[i]++;
			break;
				
			case LEFT:
				if(snakex[i]==1)
					snakex[i]=xsize-1;
				else
					snakex[i]--;
			break;
			
			case RIGHT:
				if(snakex[i]==(xsize-1))
					snakex[i]=1;
				else
					snakex[i]++;
			break;
		}
		//sn++;
	}
}

/**
 * @brief Change snake direction
 * 
 * @param dir 
 */
void cd_snake(uchar dir){
	int i=1;
	int tmp_direction=0;
	int old_direction=snaked[0];
	
	uchar direction;
	
	switch(dir){
		case 1:
			direction=UP;
		break;
		
		case 2:
			direction=DOWN;
		break;
		
		case 4:
			direction=LEFT;
		break;
		
		case 8:
			direction=RIGHT;
		break;
		
		default:
			direction=last_direction;
		break;
	}
	
	snaked[0]=direction;
	
	while(i<snake_size){
		tmp_direction=snaked[i];
		snaked[i]=old_direction;
		old_direction=tmp_direction;
		i++;
	}
	
	last_direction=direction;
}

/**
 * @brief Draw snake on the screen
 * 
 */
void draw_snake(){
	int i=0;

	int x=snakex[i];
	int y=snakey[i];
	
	POKE(CHAR_ADDRESS+(y*xsize+x),102);
	POKE(COLOR_ADDRESS+(y*xsize+x),2+LUMINANCE);
	for(i=1;i<snake_size;i++){
		x=snakex[i];
		y=snakey[i];
		
		POKE(CHAR_ADDRESS+(y*xsize+x),81);
		POKE(COLOR_ADDRESS+(y*xsize+x),7+LUMINANCE);
	}
	
	POKE(COLOR_ADDRESS+(y*xsize+x),0+LUMINANCE);
}

/**
 * @brief Draw apple on the screen
 * 
 */
void draw_apple(){
	POKE(CHAR_ADDRESS+(ay*xsize+ax),90);
	POKE(COLOR_ADDRESS+(ay*xsize+ax),2+LUMINANCE);
}

/**
 * @brief Create a apple object
 * 
 */
void create_apple(){
	uchar i=0;
	while(1){
		uchar override=0;
		ax=rand()%(xsize-2)+1;
		ay=rand()%(ysize-2)+1;
		for(i=0;i<MAX_BUSH;i++){
			if(bushx[i]==ax && bushy[i]==ay)
				override=1;
		}
		if(override==1)
			continue;
		
		break;
	}
}

/**
 * @brief Eat apple event
 * 
 * @return int 
 */
int eat_apple(){
	if(snakex[0]==ax && snakey[0]==ay){
		create_apple();
	
		if(snake_size<MAX_SNAKE){
			switch(snaked[snake_size-1]){
				case UP:
					snakex[snake_size]=snakex[snake_size-1];
					snakey[snake_size]=snakey[snake_size-1]+1;
				break;
				
				case DOWN:
					snakex[snake_size]=snakex[snake_size-1];
					snakey[snake_size]=snakey[snake_size-1]-1;
				break;
				
				case LEFT:
					snakex[snake_size]=snakex[snake_size-1]+1;
					snakey[snake_size]=snakey[snake_size-1];
				break;
				
				case RIGHT:
					snakex[snake_size]=snakex[snake_size-1]-1;
					snakey[snake_size]=snakey[snake_size-1];
				break;
					
				default:
					
				break;
			}
			snaked[snake_size]=snaked[snake_size-1];
			snake_size++;
		}
		points++;
		write_infos(ingame);
	}
	//points++;
}

/**
 * @brief Move snake on the screen, following apples
 * 
 * @return int 
 */
int demo(){
	uchar dx = abs(snakex[0]-ax);
	uchar dy = abs(snakey[0]-ay);
	
	if(dx>=dy){
		if(snakex[0]<=ax)
			if(snaked[0]!=LEFT)
				return 8;
		
		if(snakex[0]>ax)
			if(snaked[0]!=RIGHT)
				return 4;
	}else{
		if(snakey[0]>=ay)
			if(snaked[1]!=DOWN)
				return 1;
		
		if(snakey[0]<ay)
			if(snaked[1]!=UP)
				return 2;
	}
	
	return -1;
}

/**
 * @brief Check if snake eats apple
 * 
 * @return int 
 */
int self_eat(){
	int i=0;
	int hx = snakex[i];
	int hy = snakey[i];

	for(i=1;i<snake_size;i++){
		int x=snakex[i];
		int y=snakey[i];
		
		if(x==hx && y==hy)
			return 1;
		
	}
	return -1;
}

/**
 * @brief Create new
 * 
 */
void reset_game(){
	if(points>record)
		record=points;
	
	lives=5;
	points=0;
	direct=1;
	clrscr();  
	create_forest();
	create_apple();
	draw_forest();
	create_snake(5);
	write_infos(ingame);
}

/**
 * @brief 
 * 
 */
void draw_cs(){
	int x=0;
	int y=0;
	int nd=0;
	for(y=0;y<ysize;y++){
		for(x=0;x<xsize;x++){
			POKE(CHAR_ADDRESS+(y*xsize+x),nd%255);
			POKE(COLOR_ADDRESS+(y*xsize+x),5+LUMINANCE);
			nd++;
		}
	}
}

int main(){

	int counter=0;
	int x=0,y=0;
	char key=0;
	int xo=0,yo=0;
	int i=0,j=0;
	int delay=0;
	
	bordercolor (0); 
	bgcolor (0);      
	clrscr();       
	
	if (joy_install(JOY_DRIVER) != JOY_ERR_OK) {
		return 1;
	}
	//int i;

// Uppercase mode
#ifdef __C128__
	POKE (53272, (PEEK ( 53272 ) & 241) | 2);
#endif // __C128__

#ifdef __C64__
	POKE(53272,21);
#endif // __C64__
	
#if defined(__PLUS4__) || defined(__C16__)
	POKE(1351,128);
#endif // __C64__

// Copy custom character to ram and use it instead of rom one
#ifdef __C64__
	POKE(53281,0);
	POKE(CREG,(PEEK(CREG)&240)+12);

	for(i=0;i<192*8;i++){
		POKE(12288+i,charmap[i]);
	}
#endif

#ifdef __C128__
	fast();
	POKE(247, PEEK(247) | 128);
 	POKE(0xd9, PEEK(0xd9) | 4);
	POKE(CREG, PEEK(CREG) & 240 | CHARDEFS_PTR);
	for(i=0;i<192*8;i++){
		POKE(0x1000+i,charmap[i]);
	}
#endif

#if defined(__PLUS4__) || defined(__C16__)
	POKE(CREG,60);
	POKE(CREG-1,192);

	for(i=0;i<192*8;i++){
		POKE(15360+i,charmap[i]);
	}
#endif
	
	// Create new map
	srand(time(NULL));
	reset_game();
	
	for(;;){  
		if(kbhit()){
			// Wait key to start or pause
			key = cgetc() ;
			switch(key){
				case 's':
					// Start game
					if(ingame==0){
						ingame=1;
						counter=0;
						reset_game();
					}
				break;
				case 'p':
				    pause= pause==0 ? 1:0;
				break;
			}
		}
		
		// Snake collision
		if(self_eat()==1){
			clrscr();
			create_apple();
			create_snake(5);
			write_infos(ingame);
			counter=0;
			
			if(lives==0){
				lives=5;
				ingame=0;
				reset_game();
			}
			
			lives--;
		}
		
		if(pause==0)
		{
			// Game routine
			draw_apple();
			eat_apple();
			move_snake();
			
			if(ingame==0){
				// Demo mode
				if(counter%(rand()%2)==0){
					direct=demo();
				}
			}
			else{
				direct=joy_read(1);
			}	
			
			
			if(counter%2==0){
				draw_forest();
			}
			
			cd_snake(direct);

			draw_snake();
			
			if (points<32){
				for(delay=0;delay<500;delay++);
			}else if(points>32 && points<64) {
				for(delay=0;delay<400;delay++);
			}else if(points>64 && points<128){
				for(delay=0;delay<200;delay++);
			}else if(points>128 && points<192){
				for(delay=0;delay<100;delay++);
			} else if(points>192 && points<256){
				for(delay=0;delay<50;delay++);
			}
			
			counter++;
		}
	}
	
	return 0;
}

